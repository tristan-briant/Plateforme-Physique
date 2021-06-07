#include "bmm150.h"
#include "bmm150_defs.h"
#include <M5Stack.h>
#include <Preferences.h>

Preferences prefs;
struct bmm150_dev dev;
bmm150_mag_data mag_offset;
bmm150_mag_data mag_max;
bmm150_mag_data mag_min;

int8_t i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *read_data, uint16_t len)
{
    if (M5.I2C.readBytes(dev_id, reg_addr, len, read_data))
    {
        return BMM150_OK;
    }
    else
    {
        return BMM150_E_DEV_NOT_FOUND;
    }
}

int8_t i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *read_data, uint16_t len)
{
    if (M5.I2C.writeBytes(dev_id, reg_addr, read_data, len))
    {
        return BMM150_OK;
    }
    else
    {
        return BMM150_E_DEV_NOT_FOUND;
    }
}

int8_t bmm150_initialization()
{
    int8_t rslt = BMM150_OK;

    /* Sensor interface over SPI with native chip select line */
    dev.dev_id = 0x10;
    dev.intf = BMM150_I2C_INTF;
    dev.read = i2c_read;
    dev.write = i2c_write;
    dev.delay_ms = delay;

    /* make sure max < mag data first  */
    mag_max.x = -2000;
    mag_max.y = -2000;
    mag_max.z = -2000;

    /* make sure min > mag data first  */
    mag_min.x = 2000;
    mag_min.y = 2000;
    mag_min.z = 2000;

    rslt = bmm150_init(&dev);
    dev.settings.pwr_mode = BMM150_NORMAL_MODE;
    rslt |= bmm150_set_op_mode(&dev);
    dev.settings.preset_mode = BMM150_PRESETMODE_ENHANCED;
    rslt |= bmm150_set_presetmode(&dev);
    return rslt;
}

void bmm150_offset_save(float *offset)
{
    prefs.begin("bmm150", false);

    mag_offset.x = offset[1]; // index start at 1 c.f. MagBias
    mag_offset.y = offset[2];
    mag_offset.z = offset[3];

    prefs.putBytes("offset", (uint8_t *)&mag_offset, sizeof(bmm150_mag_data));
    prefs.end();
    Serial.printf("%f   %f   %f", mag_offset.x, mag_offset.y, mag_offset.z);
}

void bmm150_offset_load(float *offset)
{
    if (prefs.begin("bmm150", true))
    {
        prefs.getBytes("offset", (uint8_t *)&mag_offset, sizeof(bmm150_mag_data));
        prefs.end();
        Serial.printf("%f   %f   %f", mag_offset.x, mag_offset.y, mag_offset.z);
        offset[1] = mag_offset.x;
        offset[2] = mag_offset.y;
        offset[3] = mag_offset.z;
        Serial.printf("bmm150 load offset finish.... \r\n");
    }
    else
    {
        Serial.printf("bmm150 load offset failed.... \r\n");
    }
}

bool bmm150_begin()
{
    if (bmm150_initialization() == BMM150_OK)
        return true;
    else
        return false;
}

void bmm150_read_data(float *data)
{
    bmm150_read_mag_data(&dev);

    // minus sign for x and z because component is mounted up side down
    data[0] = -dev.data.x;
    data[1] = dev.data.y;
    data[2] = -dev.data.z;
}
