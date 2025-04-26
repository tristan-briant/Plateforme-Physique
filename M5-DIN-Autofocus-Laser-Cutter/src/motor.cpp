#include <MCP23017.h>

const int pinMotor[2][4] = {{0, 1, 2, 3}, {4, 5, 6, 7}};
int position[2] = {0, 0};

const int delayStep = 50;

bool wireBusy; // To avoid conflict on i2c when reading rtc
bool isRunning;

#define MCP23017_ADDR 0x20
MCP23017 mcp1 = MCP23017(MCP23017_ADDR);
const int SDA_Pin = GPIO_NUM_13, SCL_Pin = GPIO_NUM_15;

int readMCP()
{
    return mcp1.readPort(MCP23017Port::B);
}

void beginMCP()
{
    Wire.begin(SDA_Pin, SCL_Pin);
    mcp1.init();
    mcp1.portMode(MCP23017Port::A, 0x0);
    mcp1.portMode(MCP23017Port::B, 0xFF, 0xFF, 0xFF);
    /*mcp1.pinMode(8, INPUT_PULLUP, true);
    mcp1.pinMode(9, INPUT_PULLUP, true);
    mcp1.pinMode(10, INPUT_PULLUP, true);*/
    //mcp1.digitalWrite(10, 1);
    // mcp1.pinMode(11, INPUT_PULLUP, true);
}

void stepMotors(int step1, int step2)
{

    while (wireBusy)
        delayMicroseconds(100); // wait until i2c is free

    wireBusy = true;
    /// full 4 step
    int k1 = step1 > 0 ? 0 : 4;
    int k2 = step2 > 0 ? 0 : 4;

    for (int k = 0; k < 4; k++)
    {
        int p = 0;

        if (step1 != 0)
        {
            p = 1 << k1;
            k1 = (k1 + step1) % 4;
        }

        if (step2 != 0)
        {
            p |= 1 << (k2 + 4);
            k2 = (k2 + step2) % 4;
        }

        mcp1.writePort(MCP23017Port::A, p);

        delayMicroseconds(delayStep);
    }

    wireBusy = false;
}

void step(int dir)
{
    unsigned char p;
    if (dir == 1)
        p = 0b00000000;
    else
        p = 0b00000010;
    mcp1.writePort(MCP23017Port::A, p);
    //delayMicroseconds(10);
    if (dir == 1)
        p = 0b00000100;
    else
        p = 0b00000110;
    mcp1.writePort(MCP23017Port::A, p);
    //delayMicroseconds(delayStep - 10);
}

/*void step(int motor, int dir, bool halfstep)
{
    int stepNumber;
    if (halfstep)
        stepNumber = 8;
    else
        stepNumber = 4;

    static int k[2] = {0, 0};
    k[motor] = (k[motor] + dir) % stepNumber;
    if (k[motor] < 0)
        k[motor] += stepNumber;

    while (wireBusy)
    {
        delayMicroseconds(100); // wait until i2c is free
    }

    /// full step
    if (!halfstep)
    {
        int p = 1 << (k[motor] + 4 * motor);
        mcp1.writePort(MCP23017Port::A, p);
        // mcp1.writeGPIOAB(p);
        // for (int i = 0; i < 4; i++)
        //   mcp1.digitalWrite(pinMotor[motor][i], i == k[motor] ? HIGH : LOW);
        // delay(delayStep);
        delayMicroseconds(delayStep);
    }
    else
    { /// half step
        if (k[motor] % 2 == 0)
            for (int i = 0; i < 4; i++)
                mcp1.digitalWrite(pinMotor[motor][i], i == (k[motor] / 2) ? HIGH : LOW);
        else
        {
            mcp1.digitalWrite(pinMotor[motor][k[motor] / 2], HIGH);
            mcp1.digitalWrite(pinMotor[motor][(k[motor] / 2 + 1) % 4], HIGH);
            mcp1.digitalWrite(pinMotor[motor][(k[motor] / 2 + 2) % 4], LOW);
            mcp1.digitalWrite(pinMotor[motor][(k[motor] / 2 + 3) % 4], LOW);
        }
        delayMicroseconds(delayStep);
    }
}*/

void stopMotor()
{
    while (wireBusy)
        delayMicroseconds(100); // wait until i2c is free
    wireBusy = true;

    mcp1.writePort(MCP23017Port::A, 0b00000111);

    wireBusy = false;
}

bool isMotorRunning()
{
    return isRunning;
}

/*
bool handleMotor()
{

    if (ActualPosition[0] == position[0] && ActualPosition[1] == position[1] && isRunning)
    {
        isRunning = false;
        stopMotor();
        return isRunning;
    }

    int st[2] = {0, 0};

    for (int motor = 0; motor < 2; motor++)
    {
        if (ActualPosition[motor] > position[motor])
            st[motor] = -1;
        else if (ActualPosition[motor] < position[motor])
            st[motor] = +1;

        ActualPosition[motor] += st[motor];
    }

    if (st[0] != 0 || st[1] != 0)
    {
        isRunning = true;
        stepMotors(st[0], st[1]);
    }

    return isRunning;
}
*/