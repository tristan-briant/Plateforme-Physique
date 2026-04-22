

class SlowSerial
{
public:
    long comSpeed;
    gpio_num_t rxpin, txpin;
    bool inverse;

    void begin(long baud, int RXpin = 19, int TXpin = 27, bool inv = true);
    int write(char b);
    char read();
};