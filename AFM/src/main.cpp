#include <M5stack.h>

///// Motor pins, constants and variables
const int NSTEP_MAX = 32 * 13; // 32 step per tooth (64*8 for 360°)
int motor_x, motor_y;
int target_x, target_y;
const int TEMP = 1200; // time in µs for 1/4 step
int BackLash = 5;

int PinMotor[] = {16, 17, 2, 5};
int PinSwitchMotor = 26;

//////// Photodiodes pin and variables
const int PinPhotoDiode1 = 36;
const int PinPhotoDiode2 = 35;
float signal1; // signal from Phd1
float signal2; // signal from Phd2
float sig;     // measured depth 0<signal<1
const float gain = 5.0;

//////// Image parameters
const int IMAG_SIZE = 200;
TFT_eSprite img = TFT_eSprite(&M5.Lcd);

///// Scan parameters
const int ScanSize = NSTEP_MAX * 0.8;
const int NLines = 25;
float data[IMAG_SIZE][NLines];

////////// Function Declaration //////////////
TaskHandle_t TaskMeasure, TaskGUI, TaskMotor, TaskOperation;
void step(int motor, int nstep = 1);
void TaskInitialize(void *pvParameters);
void TaskScan(void *pvParameters);
void AbordAnyMotorTask();
void sendPythonScript();
int colorFromRGB(float r, float g, float b);
int colorScale(float x);
void loopMeasure(void *pvParameters);
void loopMotor(void *pvParameters);
void loopGUI(void *pvParameters);

void setup()
{
  Serial.begin(115200);
  M5.begin();
  pinMode(25, OUTPUT); // Deactivate loudSpeaker

  // Pins configuration
  for (int i = 0; i < 4; i++)
    pinMode(PinMotor[i], OUTPUT);

  pinMode(PinSwitchMotor, OUTPUT);

  /// Create the image
  img.setColorDepth(8);
  img.createSprite(IMAG_SIZE, IMAG_SIZE);

  target_x = target_y = motor_x = motor_y = 0;

  // Launch main loops
  xTaskCreatePinnedToCore(loopMeasure, "TaskMeasure", 4000, NULL, 1, &TaskMeasure, 1);
  xTaskCreatePinnedToCore(loopGUI, "TaskGUI", 4000, NULL, 1, &TaskGUI, 0);
  xTaskCreatePinnedToCore(loopMotor, "TaskMotor", 4000, NULL, 1, &TaskMotor, 1);
}

void loop() // not used
{
}

void loopMeasure(void *pvParameters) // Manage the detection
{
  const float alpha = 0.05; // averaging factor 0<alpha<1
  signal1 = signal2 = 0;

  while (true) // Never return
  {
    float x1 = analogRead(35) / 4095.0;
    float x2 = analogRead(36) / 4095.0;

    signal1 = alpha * x1 + (1 - alpha) * signal1;
    signal2 = alpha * x2 + (1 - alpha) * signal2;

    sig = tanh(gain * (signal2 - signal1)) * 0.5 + 0.5;

    delayMicroseconds(100);
  }
}

void loopMotor(void *pvParameters) // Manage the motor position
{
  while (true) // Never return
  {
    if (target_x != motor_x)
      step(1, target_x - motor_x);

    if (target_y != motor_y)
      step(2, target_y - motor_y);

    delay(1);
  }
}

void loopGUI(void *pvParameters) // Manage LCD and buttons
{
  while (true) // Never return
  {
    M5.update();

    ////////// Draw the interface //////////////
    M5.Lcd.setTextDatum(TC_DATUM);
    M5.Lcd.setTextSize(2);
    M5.Lcd.drawString("Abord", 70, 215);
    M5.Lcd.drawString("Scan", 160, 215);
    M5.Lcd.drawString("Init.", 250, 215);

    M5.Lcd.fillRect(0, 0, 5, 200 * (1 - signal1), BLACK);
    M5.Lcd.fillRect(0, 200 * (1 - signal1), 5, 200 * signal1, RED);
    M5.Lcd.fillRect(6, 0, 5, 200 * (1 - signal2), BLACK);
    M5.Lcd.fillRect(6, 200 * (1 - signal2), 5, 200 * signal2, RED);

    float s = constrain(1 - sig, 0, 0.9);
    M5.Lcd.fillRect(18, 0, 5, 200 * s, BLACK);
    M5.Lcd.fillRect(18, 200 * s, 5, 200 * (1 - s), colorScale(sig));

    M5.Lcd.drawRect(115 - 1, 5 - 1, IMAG_SIZE + 2, IMAG_SIZE + 2, WHITE);
    img.pushSprite(115, 5);

    /////////// Manage button or USB //////////////

    char c = 0;
    if (Serial.available()) // Check if command is sent on the USB
      c = Serial.read();

    if (M5.BtnA.wasPressed() || c == 'a') // Abord
    {
      AbordAnyMotorTask();
      target_x = 0;
      target_y = 0;
    }

    if (M5.BtnB.wasPressed() || c == 's') // Scan
    {
      AbordAnyMotorTask();
      xTaskCreatePinnedToCore(TaskScan, "TaskCalibrate", 4000, NULL, 1, &TaskOperation, 1); // Task with Motor must run on Core 1 (no delay in step)
    }

    if (M5.BtnC.wasPressed() || c == 'i') // Initialize
    {
      AbordAnyMotorTask();
      xTaskCreatePinnedToCore(TaskInitialize, "TaskInitialize", 4000, NULL, 1, &TaskOperation, 1); // Task with Motor must run on Core 1 (no delay in step)
    }

    if (c == 'p') // Send Data inside a python script
      sendPythonScript();

    delay(10);
  }
}

void AbordAnyMotorTask()
{
  if (TaskOperation)
    vTaskDelete(TaskOperation);

  target_x = motor_x;
  target_y = motor_y;
}

void TaskInitialize(void *pvParameters)
{
  target_x = motor_x - NSTEP_MAX;
  while (motor_x != target_x) // Wait until motor reach target
    delay(1);
  target_x = motor_x = -NSTEP_MAX / 2; // reset  motor position

  target_y = motor_y + NSTEP_MAX;
  while (motor_y != target_y) // Wait until motor reach target
    delay(1);
  target_y = motor_y = +NSTEP_MAX / 2; // reset  motor position

  target_x = target_y = 0;

  vTaskDelete(NULL); // Terminate task properly
}

void TaskScan(void *pvParameters)
{
  target_x = -ScanSize / 2;
  target_y = ScanSize / 2;
  while (motor_x != target_x || motor_y != target_y) // Wait until motor reach target
    delay(1);

  img.fillSprite(BLACK);

  for (int line = 0; line < NLines; line++)
  {
    target_y = 0.5 * ScanSize - ScanSize * line / NLines;
    target_x = -ScanSize / 2;
    while (motor_x != target_x || motor_y != target_y) // Wait until motor reach target
      delay(1);

    target_x = ScanSize / 2;

    while (motor_x != target_x)
    {
      int posx = map(motor_x, -0.5 * ScanSize, 0.5 * ScanSize, 0.0, IMAG_SIZE);
      int posy = map(motor_y, -0.5 * ScanSize, 0.5 * ScanSize, IMAG_SIZE, 0);
      img.fillRect(posx, posy, 1, IMAG_SIZE / NLines + 1, colorScale(sig));
      data[posx][line] = sig;

      delay(5);
    }
  }

  target_x = target_y = 0;
  vTaskDelete(NULL); // Terminate task properly
}

void step(int motor, int nstep)
{
  int direction = 1; // 1=>forward -1=>backward

  if (nstep < 0)
  {
    direction = -1;
    nstep = -nstep;
  }

  if (motor == 1)
    digitalWrite(PinSwitchMotor, HIGH);
  else
    digitalWrite(PinSwitchMotor, LOW);

  for (int k = 0; k < nstep; k++)
  {
    if (direction > 0)
      for (int i = 0; i < 4; i++)
      {
        digitalWrite(PinMotor[i], HIGH);
        delayMicroseconds(TEMP);
        digitalWrite(PinMotor[i], LOW);
      }
    else
      for (int i = 3; i >= 0; i--)
      {
        digitalWrite(PinMotor[i], HIGH);
        delayMicroseconds(TEMP);
        digitalWrite(PinMotor[i], LOW);
      }

    if (motor == 1)
      motor_x += direction;
    else
      motor_y += direction;
  }
}

int colorFromRGB(float r, float g, float b) // Convert rgb composants 0< <1 to lcd color
{
  r = constrain(r, 0, 1);
  g = constrain(g, 0, 1);
  b = constrain(b, 0, 1);

  int R = 0x1F * r;
  R *= 0x800;
  int G = 0x3F * g;
  G *= 0x20;
  int B = 0x1F * b;

  return R + G + B;
}

int colorScale(float x)
{
  if (x < 1 / 3.0)
    return colorFromRGB(0, 0, x * 2 + 1 / 3.0);
  if (x < 2 / 3.0)
    return colorFromRGB((x - 1 / 3.0) * 3, 0, 1);

  return colorFromRGB(1, (x - 2 / 3.0) * 3, 1 - (x - 2 / 3.0) * 3);
}

void sendPythonScript()
{
  Serial.print("from mpl_toolkits import mplot3d\n");
  Serial.print("import numpy as np\n");
  Serial.print("import matplotlib.pyplot as plt\n");
  Serial.printf("x = np.outer(np.linspace(-1, 1, %d), np.ones(%d))\n", NLines, IMAG_SIZE);
  Serial.printf("y = np.outer(np.linspace(-1, 1, %d), np.ones(%d)).T\n", IMAG_SIZE, NLines);
  Serial.print("fig = plt.figure()\n");
  Serial.print("ax = plt.axes(projection ='3d')\n");
  Serial.print("data=np.array([\n");

  for (int line = 0; line < NLines; line++)
  {
    Serial.print("\t[");
    for (int x = 0; x < IMAG_SIZE; x++)
    {
      Serial.printf("%5.3f", data[x][line]);
      if (x < IMAG_SIZE - 1)
        Serial.print(",");
    }
    Serial.print("]");

    if (line < NLines - 1)
      Serial.print(",\n");

    delay(10); // pause for other task
  }

  Serial.println("])");
  Serial.println("ax.plot_surface(x, y, data, cmap ='viridis', edgecolor ='green')");
  Serial.println("plt.show()");
}
