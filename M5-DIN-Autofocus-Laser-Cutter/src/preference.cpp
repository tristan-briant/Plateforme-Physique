#include <Preferences.h>


Preferences preferencesdata;

bool load_value(const char *name, const char *key, double *target)
{
    double value = 0;
    bool error = false;

    if (preferencesdata.begin(name, true))
    {
        if (preferencesdata.isKey(key))
            preferencesdata.getBytes(key, (uint8_t *)target, sizeof(double));
        else
            error = true;

        preferencesdata.end();
    }
    else
        error = false;

    return error;
}

void save_value(const char *name, const char *key, double data)
{
    if (preferencesdata.begin(name, false))
    {
        preferencesdata.putBytes(key, (uint8_t *)&data, sizeof(double));
        preferencesdata.end();
    }
}

void save_ZPEN(double zpen) {
    if (preferencesdata.begin("PEN", false))
    {
        double data[] = {zpen};
        preferencesdata.putBytes("ZPEN", (uint8_t *)data, sizeof(double));
        preferencesdata.end();
    }
}

double load_ZPEN()
{
    if (preferencesdata.begin("PEN", true))
    {
        double data[1];
        preferencesdata.getBytes("ZPEN", (uint8_t *)data, sizeof(double));
        preferencesdata.end();
        return data[0];
    }
}
