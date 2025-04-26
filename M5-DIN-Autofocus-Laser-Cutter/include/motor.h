
void step(int motor, int dir, bool halfstep = false);
void stopMotor();
void beginMCP();
int readMCP();
void setPosition(int motor, float pos, bool unlimited = false);
bool handleMotor();
bool isMotorRunning();
void resetMotorPositionStart();
void resetMotorPositionEnd();
void initPosition(float x, float y);
void stepMotors(int step1, int step2=0);
void step(int motor, int dir, bool halfstep);
void step(int dir);


