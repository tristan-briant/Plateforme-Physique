#include <M5core2.h>
#include "SlowSerial.h"
#include "pref-util.h"
#include <EEPROM.h>
#include <driver/ledc.h>

// #include <SoftwareSerial.h>

const int YPosSpeedLine = 320 - 75; // position de la ligne speed
const int YPosModeLine = 0;         // position de la ligne Mode
// const int ButtonSizeX = 50;          // Taille réelle des boutons
// const int ButtonSizeY = 50;
const int ButtonSize = 50; // Taille affichée des boutons

SlowSerial SerialS;
char StringRX[16]; // chaine de caractère lu sur le port série
char StringTX[16]; // chaine de caractère lu sur le port série pout transmission
int indexRXTX;
bool newCharAvailable = false;

void change_parameter();
void initWIFI();
void listenWIFI();
char wifiAvailable();
void wifiWrite(char c);

float vbus; // bus voltage to test  short circuit

enum mode
{
  OFF,
  BLINK,
  TRANSMITTER,
  RECEIVER,
  REPEATER
};

const char *ModeDescription[] = {
    "OFF",
    "Laser",
    //"Clignote",
    "Emetteur",
    "Recepteur",
    "WIFI"};

const char *message[] = {
    "SORBONNE UNIV.",
    "-- BONJOUR -- "};
// char message2[] = "-- BONJOUR -- ";

const int modeMAX = sizeof ModeDescription / sizeof(char *);
int ModeSelect = BLINK;

long ComSpeed[] = {10, 30, 50, 100, 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
const int SpeedMAX = sizeof ComSpeed / sizeof(long);
int SpeedSelect = 2;

long Freq[] = {1, 5, 10, 50, 100, 500, 1000, 5000, 10000};
int FreqMAX = sizeof Freq / sizeof(long);
int FreqSelect = 0;
unsigned long HalfPeriod_us = 500000;

int MsBetweenChar = 200; // delay entre chaque caractère en ms

bool redraw = true;
bool paramToBeChanged;

uint16_t color565(uint8_t red, uint8_t green, uint8_t blue)
{
  return ((red & 0b11111000) << 8) + ((green & 0b11111100) << 3) + (blue >> 3);
}

void loopGUI(void *param);
void loopMain(void *param);

Button lt(0, YPosSpeedLine, 120, 80, false, "a");
Button rt(120, YPosSpeedLine, 120, 80, false, "b");
Button lb(0, YPosModeLine, 120, 80, false, "c");
Button rb(120, YPosModeLine, 120, 80, false, "d");

Button ButLaserOnOff(60 - 50, 160 - 50, 100, 100);
Button ButLaserBlink(180 - 50, 160 - 50, 100, 100);
Button ButOFF(120 - 60, 160 - 60, 120, 120);

int messageSelect;
Button Butmessage1(0, 80, 240, 80, false, "mess1");
Button Butmessage2(0, 160, 240, 80, false, "mess2");

bool LaserOn, LaserBlink;

void ButtonHandler(Event &e)
{
  Button &b = *e.button;
  // Serial.printf("finger%d (%3d, %3d)\n", e.finger, e.from.x, e.from.y);

  // Serial.println(e.button->getName());

  if (b == lt)
  {
    if (ModeSelect == BLINK)
      FreqSelect = constrain(FreqSelect - 1, 0, FreqMAX - 1);
    else
      SpeedSelect = constrain(SpeedSelect - 1, 0, SpeedMAX - 1);
  }
  if (b == rt)
  {
    if (ModeSelect == BLINK)
      FreqSelect = constrain(FreqSelect + 1, 0, FreqMAX - 1);
    else
      SpeedSelect = constrain(SpeedSelect + 1, 0, SpeedMAX - 1);
  }

  if (b == lb)
    // ModeSelect = constrain(ModeSelect - 1, 0, modeMAX - 1);
    ModeSelect = (ModeSelect + modeMAX - 1) % modeMAX;

  if (b == rb)
    // ModeSelect = constrain(ModeSelect + 1, 0, modeMAX - 1);
    ModeSelect = (ModeSelect + 1) % modeMAX;

  if (b == ButLaserOnOff)
  {
    LaserOn = !LaserOn;
    LaserBlink = false;
  }

  if (b == ButLaserBlink)
  {
    LaserOn = false;
    LaserBlink = !LaserBlink;
  }

  if (b == ButOFF)
  {
    M5.Axp.PowerOff();
  }

  if (b == Butmessage1)
    messageSelect = 0;
  if (b == Butmessage2)
    messageSelect = 1;

  redraw = true;
  paramToBeChanged = true;
}

void change_parameter()
{
  // Appelé pour recalculer les param : période, reinit du série
  if (ModeSelect == BLINK)
  {
    HalfPeriod_us = (unsigned long)(0.5 * 1e6 / (float)Freq[FreqSelect]);
    SerialS.begin(0);

    if (LaserBlink)
    {
      ledcSetup(0, Freq[FreqSelect], 10);
      ledcAttachPin(27, 0);
      ledcWrite(0, 512);
    }
    else
      ledcDetachPin(27);

    ButLaserBlink.draw();
    ButLaserOnOff.draw();
  }
  else
  {
    ledcDetachPin(27);
    ButLaserBlink.hide();
    ButLaserOnOff.hide();
  }

  if (ModeSelect == TRANSMITTER)
  {
    Butmessage1.draw();
    Butmessage2.draw();
  }
  else
  {
    Butmessage1.hide();
    Butmessage2.hide();
  }

  if (ModeSelect == OFF)
  {
    ButOFF.draw();
  }
  else
  {
    ButOFF.hide();
  }

  if (ModeSelect == TRANSMITTER)
  {
    SerialS.begin(ComSpeed[SpeedSelect], -1); // not listening
  }

  if (ModeSelect == RECEIVER || ModeSelect == REPEATER)
  {
    for (int i = 0; i < 14; i++)
      StringRX[i] = 0;
    indexRXTX = 0;

    SerialS.begin(ComSpeed[SpeedSelect]);
  }
  paramToBeChanged = false;
}

void setup()
{
  M5.begin();
  M5.Lcd.setRotation(0);

  initWIFI();

  Serial2.begin(600, SERIAL_8N1, 19, 27, true);
  Serial.begin(115200);
  M5.Buttons.addHandler(ButtonHandler, E_TOUCH);

  paramToBeChanged = true;

  xTaskCreatePinnedToCore(loopGUI, "Task GUI", 4000, NULL, 0, NULL, 0);
}

long deltat = 0;

void loop()
{
  static char ct, cr = 0;
  static unsigned long tOld_us; // Utilisé pour le clignotement
  static bool state = false;
  unsigned long t_us = micros();
  static int indexToSend;
  static long told;

  deltat = t_us - told;
  told = t_us;

  if (paramToBeChanged)
    change_parameter();

  switch (ModeSelect)
  {
  case OFF:
    delay(10);
    break;

  case BLINK:

    if (!LaserBlink)
    {
      digitalWrite(27, LaserOn);
    }
    // M5.Axp.SetLed(gpio_get_level(GPIO_NUM_27));
    delay(10);
    break;

  case TRANSMITTER:
  {
    const char *msg = message[messageSelect];

    if (SerialS.write(msg[indexRXTX]))
      indexRXTX = (indexRXTX + 1) % 14;
  }
    delay(2);
    break;
  case RECEIVER:
  {
    char c = SerialS.read();

    if (c != 0)
    {
      if (c == 9 || c == 10 || c == 13)
        c = 32;
      else if (c < 20 || c >= 127)
        c = '?';
      // Serial.print(c);
      wifiWrite(c);

      if (indexRXTX < 14)
      {
        StringRX[indexRXTX] = c;
        indexRXTX++;
      }
      else
      {
        for (int i; i < 13; i++)
          StringRX[i] = StringRX[i + 1];
        StringRX[13] = c;
      }
    }
    delay(2);
  }
  break;
  case REPEATER:
    delay(2);
    if (indexToSend < indexRXTX)
    { // Character to send
      if (SerialS.write(StringTX[indexToSend]))
        indexToSend++;
    }
    else // on écoute
    {
      char c = SerialS.read();

      if (c != 0)
      {
        if (c == 9 || c == 10 || c == 13)
          c = 32;
        else if (c < 20 || c >= 127)
          c = '?';

        wifiWrite(c);
        // Serial.print(c);

        if (indexRXTX < 14)
        {
          StringRX[indexRXTX] = c;
          indexRXTX++;
        }
        else
        {
          for (int i; i < 13; i++)
            StringRX[i] = StringRX[i + 1];
          StringRX[13] = c;
        }
      }
    }

    // if (Serial.available())

    //{
    char c = wifiAvailable(); // Serial.read();
    if (c < ' ')              // Si le caractère n'est imprimable on ne l'imprime pas
      break;

    if (indexRXTX < 14)
    {
      StringTX[indexRXTX] = c;
      indexRXTX++;
    }
    else if (indexToSend > 0)
    {
      for (int i = 0; i < 13; i++)
        StringTX[i] = StringTX[i + 1];
      StringTX[13] = c;
      indexToSend--;
    }
    //}

    break;
  }

  vbus = M5.Axp.GetVBusVoltage();
}

void loopGUI(void *param)
{
  u_long t0 = 0, t1;
  // Image des boutons
  const int ButtonSizeX = 50; // Taille réelle des boutons
  const int ButtonSizeY = 75;
  bool falte;

  TFT_eSprite imgButSpeedDown = TFT_eSprite(&M5.Lcd);
  TFT_eSprite imgButSpeedUp = TFT_eSprite(&M5.Lcd);
  TFT_eSprite imgButModeUp = TFT_eSprite(&M5.Lcd);
  TFT_eSprite imgButModeDown = TFT_eSprite(&M5.Lcd);

  imgButSpeedDown.setColorDepth(8);
  imgButSpeedDown.createSprite(ButtonSizeX, ButtonSizeY);
  imgButSpeedDown.fillRoundRect(0, 0, ButtonSizeX, ButtonSizeY, 5, DARKGREEN);
  imgButSpeedDown.fillRect(ButtonSizeX / 2 - 15, ButtonSizeY / 2 - 1, 30, 3, WHITE);

  imgButSpeedUp.setColorDepth(8);
  imgButSpeedUp.createSprite(ButtonSizeX, ButtonSizeY);
  imgButSpeedUp.fillRoundRect(0, 0, ButtonSizeX, ButtonSizeY, 5, DARKGREEN);
  imgButSpeedUp.fillRect(ButtonSizeX / 2 - 15, ButtonSizeY / 2 - 1, 30, 3, WHITE);
  imgButSpeedUp.fillRect(ButtonSizeX / 2 - 1, ButtonSizeY / 2 - 15, 3, 30, WHITE);

  imgButModeDown.setColorDepth(8);
  imgButModeDown.createSprite(ButtonSizeX, ButtonSizeY);
  imgButModeDown.fillRoundRect(0, 0, ButtonSizeX, ButtonSizeY, 5, DARKGREEN);
  imgButModeDown.fillTriangle(ButtonSizeX / 2 + 15, ButtonSizeY / 2 - 15,
                              ButtonSizeX / 2 - 15, ButtonSizeY / 2,
                              ButtonSizeX / 2 + 15, ButtonSizeY / 2 + 15, WHITE);

  imgButModeUp.setColorDepth(8);
  imgButModeUp.createSprite(ButtonSizeX, ButtonSizeY);
  imgButModeUp.fillRoundRect(0, 0, ButtonSizeX, ButtonSizeY, 5, DARKGREEN);
  imgButModeUp.fillTriangle(ButtonSizeX / 2 - 15, ButtonSizeY / 2 - 15,
                            ButtonSizeX / 2 + 15, ButtonSizeY / 2,
                            ButtonSizeX / 2 - 15, ButtonSizeY / 2 + 15, WHITE);

  TFT_eSprite imgBannerRX = TFT_eSprite(&M5.Lcd);
  imgBannerRX.setColorDepth(8);
  imgBannerRX.createSprite(240, 70);

  char str[100];

  while (true)
  {
    M5.update();

    listenWIFI();

    if (Serial.available())
    {
      char c = Serial.read();
      if (c == '+')
        increment_name();
      if (c == '-')
        increment_name(-1);
      if (c == '*')
        clear_name();
      redraw = true;
    }

    if (redraw)
    {
      // Mode Display
      // M5.Lcd.fillScreen(BLACK);

      M5.Lcd.setTextDatum(MC_DATUM);
      M5.Lcd.setFreeFont(&FreeSans12pt7b);
      uint16_t bgcolor = color565(64, 64, 64);
      M5.Lcd.textbgcolor = bgcolor;
      M5.Lcd.textcolor = WHITE;

      M5.Lcd.fillRoundRect(ButtonSizeX, YPosModeLine,
                           240 - 2 * ButtonSizeX, ButtonSizeY, 5, bgcolor);
      M5.Lcd.drawString(ModeDescription[ModeSelect],
                        120, YPosModeLine + ButtonSizeY / 2);

      imgButModeDown.pushSprite(0, YPosModeLine);
      imgButModeUp.pushSprite(240 - ButtonSizeX, YPosModeLine);

      // Speed Display
      if (ModeSelect == BLINK)
        sprintf(str, "%d Hz", Freq[FreqSelect]);
      else
        sprintf(str, "%d bit/s", ComSpeed[SpeedSelect]);

      M5.Lcd.setTextDatum(MC_DATUM);
      M5.Lcd.setFreeFont(&FreeSans12pt7b);

      M5.Lcd.fillRoundRect(ButtonSizeX, YPosSpeedLine, 240 - 2 * ButtonSizeX, ButtonSizeY, 5, bgcolor);
      M5.Lcd.drawString(str, 120, YPosSpeedLine + ButtonSizeY / 2);

      imgButSpeedDown.pushSprite(0, YPosSpeedLine);
      imgButSpeedUp.pushSprite(240 - ButtonSizeX, YPosSpeedLine);

      if (ModeSelect == BLINK)
      {
        M5.Lcd.fillRect(0, ButtonSizeY, 240, 320 - 2 * ButtonSizeY, BLACK);
        int bgcolor = LaserOn ? RED : color565(64, 0, 0);
        M5.Lcd.fillCircle(60, 160, 50, bgcolor);
        M5.Lcd.setFreeFont(&FreeSans18pt7b);
        M5.Lcd.textbgcolor = bgcolor;
        M5.Lcd.drawString(LaserOn ? "ON" : "OFF", 60, 160);

        bgcolor = LaserBlink ? RED : color565(64, 0, 0);
        M5.Lcd.fillCircle(180, 160, 50, bgcolor);
        M5.Lcd.setFreeFont(&FreeSans12pt7b);
        M5.Lcd.textbgcolor = bgcolor;
        M5.Lcd.drawString("BLINK", 180, 160);
      }

      if (ModeSelect == TRANSMITTER)
      {
        M5.Lcd.fillRect(0, ButtonSizeY, 240, 320 - 2 * ButtonSizeY, BLACK);

        M5.Lcd.setFreeFont(&FreeMono12pt7b);

        int y = messageSelect == 0 ? 85 : 165;
        bgcolor = color565(0, 64, 0);
        M5.Lcd.fillRoundRect(0, y, 240, 70, 10, bgcolor);

        M5.Lcd.drawRoundRect(0, 85, 240, 70, 10, DARKGREEN);
        M5.Lcd.textbgcolor = messageSelect == 0 ? bgcolor : BLACK;
        M5.Lcd.drawString(message[0], 120, 120);

        M5.Lcd.drawRoundRect(0, 165, 240, 70, 10, DARKGREEN);
        M5.Lcd.textbgcolor = messageSelect == 1 ? bgcolor : BLACK;
        M5.Lcd.drawString(message[1], 120, 200);
      }

      if (ModeSelect == RECEIVER)
      {
        M5.Lcd.fillRect(0, ButtonSizeY, 240, 320 - 2 * ButtonSizeY, BLACK);
        M5.Lcd.drawRoundRect(0, 160 - 35, 240, 70, 10, DARKGREEN);
        // M5.Lcd.setTextDatum(CL_DATUM);
        // M5.Lcd.setFreeFont(&FreeMono12pt7b);
        // M5.Lcd.drawString(StringRX, 120, 160);
      }

      if (ModeSelect == REPEATER)
      {
        sprintf(str, "%s", getDeviceName());
        M5.Lcd.fillRect(0, ButtonSizeY, 240, 320 - 2 * ButtonSizeY, BLACK);

        M5.Lcd.setTextDatum(CL_DATUM);
        M5.Lcd.textbgcolor = BLACK;
        M5.Lcd.setFreeFont(&FreeSans9pt7b);
        M5.Lcd.drawString("Se connecter sur", 10, 90);
        M5.Lcd.setFreeFont(&FreeSans12pt7b);
        M5.Lcd.drawString(str, 10, 130);

        M5.Lcd.drawString("192.168.4.1", 10, 160);
        M5.Lcd.setFreeFont(&FreeSans9pt7b);

        M5.Lcd.drawString("Desactiver donnees mobiles", 10, 200);
      }

      if (ModeSelect == OFF)
      {
        M5.Lcd.fillRect(0, ButtonSizeY, 240, 320 - 2 * ButtonSizeY, BLACK);
        M5.Lcd.drawCircle(120, 160, 60, RED);
        M5.Lcd.drawCircle(120, 160, 59, RED);
        M5.Lcd.drawCircle(120, 160, 58, RED);

        M5.Lcd.fillRect(120 - 1, 160 - 40, 3, 80, RED);
      }

      if (falte)
      {
        M5.Lcd.fillRect(10, 80, 220, 320 - 2 * 80, RED);
        M5.Lcd.textbgcolor = RED;
        M5.Lcd.textdatum = MC_DATUM;
        M5.Lcd.setFreeFont(&FreeSans12pt7b);
        M5.Lcd.drawString("Court-circuit", 240 / 2, 320 / 2 - 10);
        M5.Lcd.drawString("sur l'alimentation", 240 / 2, 320 / 2 + 10);
      }

      // Serial.println("redraw");
      redraw = false;
    }

    if (!falte)
    {
      if (ModeSelect == RECEIVER)
      {
        // M5.Lcd.setTextDatum(CL_DATUM);
        imgBannerRX.fillSprite(BLACK);
        imgBannerRX.drawRoundRect(0, 0, 240, 70, 10, DARKGREEN);

        imgBannerRX.setFreeFont(&FreeMono12pt7b);
        imgBannerRX.textbgcolor = BLACK;
        imgBannerRX.setTextDatum(CL_DATUM);
        imgBannerRX.drawString(StringRX, 22, 35);
        imgBannerRX.pushSprite(0, 160 - 35);
      }
    }

    if (vbus < 1.0)
    {
      if (!falte)
      {
        falte = true;
        redraw = true;
      }
    }
    else
    {
      if (falte)
      {
        falte = false;
        redraw = true;
      }
    }

    // sprintf(str, "%10d", deltat);
    // M5.Lcd.drawString(str, 10, 10);

    // delay(5);
  }
}