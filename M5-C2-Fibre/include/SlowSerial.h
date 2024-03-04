

class SlowSerial
{
public:
    long comSpeed;
    int rxpin, txpin;
    bool inverse;

    void begin(long baud, int RXpin = 19, int TXpin = 27, bool inv = true);
    int write(char b);
    char read();
};
