// Copyright 2024 Accenture.

#include "eeprom/EepromDriver.h"

#include "nvs.h"
#include "nvs_flash.h"

#include <cstring>

namespace
{
static char const* NVS_NAMESPACE = "openbsw_eep";
static char const* NVS_KEY       = "eeprom_data";
} // namespace

namespace eeprom
{

EepromDriver::EepromDriver() : _initialized(false) { (void)::memset(_cache, 0xFF, EEPROM_SIZE); }

::bsp::BspReturnCode EepromDriver::init()
{
    if (_initialized)
    {
        return ::bsp::BSP_OK;
    }

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    if (err != ESP_OK)
    {
        return ::bsp::BSP_ERROR;
    }

    // Load existing data from NVS into cache
    nvs_handle_t handle;
    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err == ESP_OK)
    {
        size_t length = EEPROM_SIZE;
        err           = nvs_get_blob(handle, NVS_KEY, _cache, &length);
        nvs_close(handle);
    }

    _initialized = true;
    return ::bsp::BSP_OK;
}

::bsp::BspReturnCode
EepromDriver::read(uint32_t address, uint8_t* buffer, uint32_t length)
{
    if (!_initialized || (buffer == nullptr) || (length == 0U)
        || ((address + length) > EEPROM_SIZE))
    {
        return ::bsp::BSP_ERROR;
    }

    (void)::memcpy(buffer, &_cache[address], length);
    return ::bsp::BSP_OK;
}

::bsp::BspReturnCode
EepromDriver::write(uint32_t address, uint8_t const* buffer, uint32_t length)
{
    if (!_initialized || (buffer == nullptr) || (length == 0U)
        || ((address + length) > EEPROM_SIZE))
    {
        return ::bsp::BSP_ERROR;
    }

    (void)::memcpy(&_cache[address], buffer, length);

    // Persist to NVS
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK)
    {
        return ::bsp::BSP_ERROR;
    }

    err = nvs_set_blob(handle, NVS_KEY, _cache, EEPROM_SIZE);
    if (err == ESP_OK)
    {
        err = nvs_commit(handle);
    }
    nvs_close(handle);

    return (err == ESP_OK) ? ::bsp::BSP_OK : ::bsp::BSP_ERROR;
}

} // namespace eeprom
