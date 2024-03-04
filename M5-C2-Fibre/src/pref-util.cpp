// #include <Preferences.h>
#include <EEPROM.h>

// Preferences preferencesdata;

int name_number;

void save_name(int n)
{
    EEPROM.begin(4);
    EEPROM.writeInt(0, n);
    EEPROM.end();
}

int load_name()
{
    int n;
    EEPROM.begin(4);
    n = EEPROM.readInt(0);
    EEPROM.end();

    return n;

}

void increment_name(int inc = +1)
{

    int ndev = load_name() + inc;
    ndev = constrain(ndev, 0, 99);

    save_name(ndev);
}

char *name = new char[20];

char *getDeviceName()
{
    int n = load_name();
    sprintf(name, "FIBRE-BOX-%02d", n);
    return name;
}

void clear_name()
{
   save_name(0);
}