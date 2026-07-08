// Copyright 2024 Accenture.

#include "can/Mcp2515CanTransceiver.h"

#include "can/canframes/ICANFrameSentListener.h"

#include "driver/spi_common.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_rom_sys.h"

namespace can
{

namespace
{
// MCP2515 SPI instruction set
constexpr uint8_t CMD_RESET       = 0xC0U;
constexpr uint8_t CMD_READ        = 0x03U;
constexpr uint8_t CMD_WRITE       = 0x02U;
constexpr uint8_t CMD_READ_STATUS = 0xA0U;
constexpr uint8_t CMD_BIT_MODIFY  = 0x05U;
constexpr uint8_t CMD_LOAD_TX0    = 0x40U;
constexpr uint8_t CMD_RTS_TX0     = 0x81U;
constexpr uint8_t CMD_READ_RX0    = 0x90U;
constexpr uint8_t CMD_READ_RX1    = 0x94U;

// MCP2515 registers
constexpr uint8_t REG_CANCTRL  = 0x0FU;
constexpr uint8_t REG_CANSTAT  = 0x0EU;
constexpr uint8_t REG_CNF3     = 0x28U;
constexpr uint8_t REG_CNF2     = 0x29U;
constexpr uint8_t REG_CNF1     = 0x2AU;
constexpr uint8_t REG_CANINTF  = 0x2CU;
constexpr uint8_t REG_TXB0CTRL = 0x30U;
constexpr uint8_t REG_RXB0CTRL = 0x60U;
constexpr uint8_t REG_RXB1CTRL = 0x70U;

// CANCTRL/CANSTAT operation-mode field (bits 7:5)
constexpr uint8_t OPMODE_MASK   = 0xE0U;
constexpr uint8_t OPMODE_NORMAL = 0x00U;
constexpr uint8_t OPMODE_CONFIG = 0x80U;

// Bit timing for an 8 MHz crystal at 500 kbit/s
constexpr uint8_t CNF1_8MHZ_500K = 0x00U;
constexpr uint8_t CNF2_8MHZ_500K = 0x90U;
constexpr uint8_t CNF3_8MHZ_500K = 0x02U;

// RXBnCTRL RXM = 0b11 -> accept every frame (masks/filters disabled)
constexpr uint8_t RXM_ACCEPT_ALL = 0x60U;

// READ_STATUS response bits
constexpr uint8_t STATUS_RX0IF = 0x01U; // RXB0 full
constexpr uint8_t STATUS_RX1IF = 0x02U; // RXB1 full

// TXB0CTRL.TXREQ -> a message is queued/pending in TXB0
constexpr uint8_t TXB_TXREQ = 0x08U;

constexpr uint8_t CAN_MAX_DLC = 8U;

esp_err_t spiTxRx(
    spi_device_handle_t const spi, uint8_t const* const tx, uint8_t* const rx, size_t const len)
{
    spi_transaction_t txn = {};
    txn.length            = len * 8U; // total length is expressed in bits
    txn.tx_buffer         = tx;
    txn.rx_buffer         = rx;
    return spi_device_transmit(spi, &txn);
}

uint8_t mcpReadRegister(spi_device_handle_t const spi, uint8_t const addr)
{
    uint8_t const tx[3] = {CMD_READ, addr, 0x00U};
    uint8_t rx[3]       = {};
    (void)spiTxRx(spi, tx, rx, sizeof(tx));
    return rx[2];
}

void mcpWriteRegister(spi_device_handle_t const spi, uint8_t const addr, uint8_t const value)
{
    uint8_t const tx[3] = {CMD_WRITE, addr, value};
    uint8_t rx[3]       = {};
    (void)spiTxRx(spi, tx, rx, sizeof(tx));
}

void mcpBitModify(
    spi_device_handle_t const spi, uint8_t const addr, uint8_t const mask, uint8_t const value)
{
    uint8_t const tx[4] = {CMD_BIT_MODIFY, addr, mask, value};
    uint8_t rx[4]       = {};
    (void)spiTxRx(spi, tx, rx, sizeof(tx));
}

uint8_t mcpReadStatus(spi_device_handle_t const spi)
{
    uint8_t const tx[2] = {CMD_READ_STATUS, 0x00U};
    uint8_t rx[2]       = {};
    (void)spiTxRx(spi, tx, rx, sizeof(tx));
    return rx[1];
}

void mcpReset(spi_device_handle_t const spi)
{
    uint8_t const tx[1] = {CMD_RESET};
    uint8_t rx[1]       = {};
    (void)spiTxRx(spi, tx, rx, sizeof(tx));
}

// Polls CANSTAT until the controller reports the requested operating mode.
bool mcpWaitForMode(spi_device_handle_t const spi, uint8_t const opmode)
{
    for (uint32_t attempt = 0U; attempt < 100U; ++attempt)
    {
        if ((mcpReadRegister(spi, REG_CANSTAT) & OPMODE_MASK) == opmode)
        {
            return true;
        }
        esp_rom_delay_us(100U);
    }
    return false;
}
} // namespace

ICanTransceiver::ErrorCode Mcp2515CanTransceiver::init()
{
    if (_initialized)
    {
        return ErrorCode::CAN_ERR_OK;
    }

    spi_host_device_t const host = static_cast<spi_host_device_t>(_config.spiHost);

    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num      = _config.mosiPin;
    buscfg.miso_io_num      = _config.misoPin;
    buscfg.sclk_io_num      = _config.sclkPin;
    buscfg.quadwp_io_num    = -1;
    buscfg.quadhd_io_num    = -1;
    buscfg.max_transfer_sz  = 0;

    esp_err_t const busErr = spi_bus_initialize(host, &buscfg, SPI_DMA_DISABLED);
    // A sibling MCP2515 sharing this host may already have initialized the bus;
    // that is not an error for this transceiver.
    if ((busErr != ESP_OK) && (busErr != ESP_ERR_INVALID_STATE))
    {
        return ErrorCode::CAN_ERR_INIT_FAILED;
    }

    spi_device_interface_config_t devcfg = {};
    devcfg.mode                          = 0;
    devcfg.clock_speed_hz                = static_cast<int>(_config.spiClockHz);
    devcfg.spics_io_num                  = _config.csPin;
    devcfg.queue_size                    = 4;

    if (spi_bus_add_device(host, &devcfg, &_spi) != ESP_OK)
    {
        return ErrorCode::CAN_ERR_INIT_FAILED;
    }

    // Reset the controller and let its oscillator/logic settle.
    mcpReset(_spi);
    esp_rom_delay_us(10000U);

    // Request configuration mode (touching only the REQOP field) and wait for
    // the controller to acknowledge it.
    mcpBitModify(_spi, REG_CANCTRL, OPMODE_MASK, OPMODE_CONFIG);
    if (!mcpWaitForMode(_spi, OPMODE_CONFIG))
    {
        (void)spi_bus_remove_device(_spi);
        _spi = nullptr;
        return ErrorCode::CAN_ERR_INIT_FAILED;
    }

    // Program bit timing (8 MHz crystal, 500 kbit/s).
    mcpWriteRegister(_spi, REG_CNF1, CNF1_8MHZ_500K);
    mcpWriteRegister(_spi, REG_CNF2, CNF2_8MHZ_500K);
    mcpWriteRegister(_spi, REG_CNF3, CNF3_8MHZ_500K);

    // Accept every frame in both receive buffers (RXM = 0b11).
    mcpWriteRegister(_spi, REG_RXB0CTRL, RXM_ACCEPT_ALL);
    mcpWriteRegister(_spi, REG_RXB1CTRL, RXM_ACCEPT_ALL);

    // Clear all interrupt flags.
    mcpWriteRegister(_spi, REG_CANINTF, 0x00U);

    // Enter normal operating mode and confirm the transition.
    mcpBitModify(_spi, REG_CANCTRL, OPMODE_MASK, OPMODE_NORMAL);
    if (!mcpWaitForMode(_spi, OPMODE_NORMAL))
    {
        (void)spi_bus_remove_device(_spi);
        _spi = nullptr;
        return ErrorCode::CAN_ERR_INIT_FAILED;
    }

    _initialized = true;
    setState(State::INITIALIZED);
    return ErrorCode::CAN_ERR_OK;
}

void Mcp2515CanTransceiver::shutdown()
{
    if (_initialized)
    {
        if (_spi != nullptr)
        {
            // Return the controller to configuration mode so it stops driving
            // the bus, then release the SPI device. The bus itself is left
            // initialized because it may be shared with a sibling MCP2515.
            mcpBitModify(_spi, REG_CANCTRL, OPMODE_MASK, OPMODE_CONFIG);
            (void)spi_bus_remove_device(_spi);
            _spi = nullptr;
        }
        _initialized = false;
        setState(State::CLOSED);
    }
}

ICanTransceiver::ErrorCode Mcp2515CanTransceiver::open(CANFrame const& frame)
{
    if (!isInState(State::INITIALIZED))
    {
        return ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }
    setState(State::OPEN);
    (void)write(frame);
    return ErrorCode::CAN_ERR_OK;
}

ICanTransceiver::ErrorCode Mcp2515CanTransceiver::open()
{
    if (!isInState(State::INITIALIZED))
    {
        return ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }
    setState(State::OPEN);
    return ErrorCode::CAN_ERR_OK;
}

ICanTransceiver::ErrorCode Mcp2515CanTransceiver::close()
{
    shutdown();
    return ErrorCode::CAN_ERR_OK;
}

ICanTransceiver::ErrorCode Mcp2515CanTransceiver::mute()
{
    if (!isInState(State::OPEN))
    {
        return ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }
    setState(State::MUTED);
    return ErrorCode::CAN_ERR_OK;
}

ICanTransceiver::ErrorCode Mcp2515CanTransceiver::unmute()
{
    if (!isInState(State::MUTED))
    {
        return ErrorCode::CAN_ERR_ILLEGAL_STATE;
    }
    setState(State::OPEN);
    return ErrorCode::CAN_ERR_OK;
}

uint32_t Mcp2515CanTransceiver::getBaudrate() const { return _config.canBaud; }

uint16_t Mcp2515CanTransceiver::getHwQueueTimeout() const
{
    // Approximate: one CAN frame at 500kbps takes ~230us, queue depth ~5
    return static_cast<uint16_t>(
        (_config.canBaud > 0U) ? (5U * 8U * 130U * 1000U / _config.canBaud) : 10U);
}

ICanTransceiver::ErrorCode Mcp2515CanTransceiver::write(CANFrame const& frame)
{
    if (!_initialized)
    {
        return ErrorCode::CAN_ERR_TX_OFFLINE;
    }

    // If TXB0 is still transmitting the previous frame the queue is full.
    if ((mcpReadRegister(_spi, REG_TXB0CTRL) & TXB_TXREQ) != 0U)
    {
        return ErrorCode::CAN_ERR_TX_HW_QUEUE_FULL;
    }

    uint32_t const id = frame.getId();
    uint8_t dlc       = frame.getPayloadLength();
    if (dlc > CAN_MAX_DLC)
    {
        dlc = CAN_MAX_DLC;
    }

    // LOAD_TX0 writes TXB0 starting at SIDH: SIDH, SIDL, EID8, EID0, DLC, D0..D7.
    uint8_t tx[14] = {};
    tx[0]          = CMD_LOAD_TX0;
    tx[1]          = static_cast<uint8_t>((id >> 3) & 0xFFU); // SIDH: SID[10:3]
    tx[2]          = static_cast<uint8_t>((id << 5) & 0xE0U); // SIDL: SID[2:0], standard frame
    tx[3]          = 0x00U;                                   // EID8 (unused for 11-bit id)
    tx[4]          = 0x00U;                                   // EID0 (unused for 11-bit id)
    tx[5]          = dlc;                                     // DLC (standard data frame)
    for (uint8_t i = 0U; i < dlc; ++i)
    {
        tx[6U + i] = frame.getPayload()[i];
    }

    uint8_t rx[14] = {};
    (void)spiTxRx(_spi, tx, rx, static_cast<size_t>(6U) + dlc);

    // Request transmission of TXB0.
    uint8_t const rtsTx[1] = {CMD_RTS_TX0};
    uint8_t rtsRx[1]       = {};
    (void)spiTxRx(_spi, rtsTx, rtsRx, sizeof(rtsTx));

    return ErrorCode::CAN_ERR_OK;
}

ICanTransceiver::ErrorCode
Mcp2515CanTransceiver::write(CANFrame const& frame, ICANFrameSentListener& listener)
{
    ErrorCode const result = write(frame);
    if (result == ErrorCode::CAN_ERR_OK)
    {
        // Notify the per-write listener (drives DoCAN tx-confirmation) as well as
        // globally-registered sent-listeners, matching TwaiCanTransceiver semantics.
        listener.canFrameSent(frame);
        notifySentListeners(frame);
    }
    return result;
}

void Mcp2515CanTransceiver::readRxBuffer(uint8_t const readCmd)
{
    // READ RX BUFFER clocks out SIDH, SIDL, EID8, EID0, DLC, D0..D7 (13 bytes)
    // and auto-clears the associated RXnIF flag when CS is deasserted.
    uint8_t tx[14] = {};
    tx[0]          = readCmd;
    uint8_t rx[14] = {};
    (void)spiTxRx(_spi, tx, rx, sizeof(tx));

    uint8_t const sidh = rx[1];
    uint8_t const sidl = rx[2];
    uint8_t dlc        = rx[5] & 0x0FU;
    if (dlc > CAN_MAX_DLC)
    {
        dlc = CAN_MAX_DLC;
    }

    uint32_t const id
        = (static_cast<uint32_t>(sidh) << 3) | (static_cast<uint32_t>(sidl) >> 5);

    CANFrame frame;
    frame.setId(id);
    frame.setPayloadLength(dlc);
    for (uint8_t i = 0U; i < dlc; ++i)
    {
        frame.getPayload()[i] = rx[6U + i];
    }
    notifyListeners(frame);
}

void Mcp2515CanTransceiver::run()
{
    if (!_initialized)
    {
        return;
    }

    uint8_t const status = mcpReadStatus(_spi);
    if ((status & STATUS_RX0IF) != 0U)
    {
        readRxBuffer(CMD_READ_RX0);
    }
    if ((status & STATUS_RX1IF) != 0U)
    {
        readRxBuffer(CMD_READ_RX1);
    }
}

} // namespace can
