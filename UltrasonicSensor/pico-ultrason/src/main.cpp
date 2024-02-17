/****************************************************************************************************************************
  basic_pwm.ino
  For RP2040 boards
  Written by Dr. Benjamin Bird

  A basic example to get you up and running.

  Library by Khoi Hoang https://github.com/khoih-prog/RP2040_PWM
  Licensed under MIT license

  The RP2040 PWM block has 8 identical slices. Each slice can drive two PWM output signals, or measure the frequency
  or duty cycle of an input signal. This gives a total of up to 16 controllable PWM outputs. All 30 GPIO pins can be driven
  by the PWM block
*****************************************************************************************************************************/
#include <Arduino.h>
// #define _PWM_LOGLEVEL_ 0 // message du PWM
#include "RP2040_PWM.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include <pico/multicore.h>
// #include "hardware/timer.h"
#include "pico.h"
// #include "hardware/structs/systick.h"
// #include "non_blocking_timer.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
// #include "fast_atan.h"
float atan2LUTif(float y, float x);
float distance;
// #define DEBUG_PULSE

// Channel 0 is GPIO26
#define CAPTURE_CHANNEL 0
#define CAPTURE_DEPTH 100 // 50 , 5000Hz   100 4000Hz 200 2000Hz
#define Freq 4000

///////////// Gain selection //////////////////////////
const float MM_TO_VOLT = 100.0f / 3.25f;
const float MM5_TO_VOLT = 20.0f / 3.25f;
const float CM_TO_VOLT = 10.0f / 3.25f;
const float CM5_TO_VOLT = 2.0f / 3.25f;

float factor = 0;

const int pinGain1 = 19;
const int pinGain2 = 20;

float I, Q;

float turn = 0;
float turn_frac, turnTotal, turnTotal0 = 0;
float turnTotal_avg = 0;
const float epsilon = 0.1;

uint16_t capture_buf[CAPTURE_DEPTH];

float cosinus[CAPTURE_DEPTH] = {0};
float sinus[CAPTURE_DEPTH] = {0};

uint dma_chan;
dma_channel_config cfg;

// creates pwm instance
RP2040_PWM *PWM2;
RP2040_PWM *PWM1;
RP2040_PWM *PWMI;
RP2040_PWM *PWMQ;

bool flag = false; // indique si des donnée sont dispos
int buf[CAPTURE_DEPTH];

void loopTask(uint /*gpio*/, uint32_t /*event_mask*/);

void cb(uint /*gpio*/, uint32_t /*event_mask*/)
{
#ifdef DEBUG_PULSE
  gpio_put(3, 1);
#endif
  // flag = false;
  //    digitalWrite(3, 0);

  dma_channel_configure(dma_chan, &cfg,
                        capture_buf,   // dst
                        &adc_hw->fifo, // src
                        CAPTURE_DEPTH, // transfer count
                        true           // start immediately
  );

  adc_run(true);

  // Once DMA finishes, stop any new conversions from starting, and clean up
  // the FIFO in case the ADC was still mid-conversion.
  uint16_t i, q;
  i = (32768 + I * 32767);
  q = (32768 + Q * 32767);

  if (Serial1.available())
  {
    Serial1.read();
    // Serial1.flush();
    //  Serial1.write(255);                // byte d'amorce
    Serial1.write((uint8_t)(i / 256)); // On profite que le core1 est dipso pour faire la communication série
    Serial1.write((uint8_t)(i % 256)); // On profite que le core1 est dipso pour faire la communication série

    Serial1.write((uint8_t)(q / 256)); // On profite que le core1 est dipso pour faire la communication série
    Serial1.write((uint8_t)(q % 256));
  }
  // turn_frac = atan2LUTif(Q, I) * 0.15915f;

  dma_channel_wait_for_finish_blocking(dma_chan);

  adc_run(false);

  adc_fifo_drain();

  for (int i = 0; i < CAPTURE_DEPTH; ++i)
    buf[i] = capture_buf[i];
  flag = true;

  // remet les buffer ready pour la prochaine fois
  /*dma_channel_configure(dma_chan, &cfg,
                        capture_buf,   // dst
                        &adc_hw->fifo, // src
                        CAPTURE_DEPTH, // transfer count
                        true           // start immediately
  );*/

#ifdef DEBUG_PULSE
  gpio_put(3, 0);
#endif

  // Serial1.begin(2000000);
  // delayMicroseconds(100);
  // Serial1.write(159);
  // Serial1.end();
}

void task()
{

  pinMode(3, OUTPUT);

  dma_channel_configure(dma_chan, &cfg,
                        capture_buf,   // dst
                        &adc_hw->fifo, // src
                        CAPTURE_DEPTH, // transfer count
                        true           // start immediately
  );

  gpio_set_irq_enabled_with_callback(6, GPIO_IRQ_EDGE_RISE, true, &cb);
}

void setup()
{
  for (int i = 0; i < CAPTURE_DEPTH; i++)
  {
    cosinus[i] = cos(2 * PI * i / 12.5);
    sinus[i] = sin(2 * PI * i / 12.5);
  }

  Serial1.begin(2000000, SERIAL_8N1);
  // Serial.begin(2000000, SERIAL_8N1);

  pinMode(pinGain1, INPUT_PULLUP);
  pinMode(pinGain2, INPUT_PULLUP);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);

  uint8_t GainSelection;
  GainSelection = digitalRead(pinGain1) + 2 * digitalRead(pinGain2);
  // GainSelection = 0;

  if (GainSelection == 0)
    factor = MM_TO_VOLT;
  if (GainSelection == 1)
    factor = MM5_TO_VOLT;
  if (GainSelection == 2)
    factor = CM_TO_VOLT;
  if (GainSelection == 3)
    factor = CM5_TO_VOLT;

  PWM2 = new RP2040_PWM(6, 2000, 50);   // 6 = slice 3
  PWM1 = new RP2040_PWM(21, 40000, 50); // 21 -> slice 2
  // PWMI = new RP2040_PWM(14, 20000, 0);  // 14 et 15 slice 7
  PWMQ = new RP2040_PWM(15, 20000, 0);

  PWM1->setPWM(21, 40000, 50);
  PWM2->setPWM(6, Freq, 50);

  PWM1->setPWM(13, 1000000, 50);


  // trop compliqué de calculer soit même les freq et wrap ---> aboutit à des désychros de l'ADC
  /*int pin = 6;
  gpio_set_function(pin, GPIO_FUNC_PWM);
  int slice = pwm_gpio_to_slice_num(pin);
  pwm_set_wrap(slice, 31241); // 31250 = 4000Hz
  pwm_set_gpio_level(pin, 15624);
  pwm_set_enabled(slice, true);

  int pin2 = 2;
  gpio_set_function(pin2, GPIO_FUNC_PWM);
  int slice2 = pwm_gpio_to_slice_num(pin2);
  pwm_set_wrap(slice2, 3125);
  pwm_set_gpio_level(pin2, 1562);
  pwm_set_enabled(slice2, true); // must be running before changing the counter

  delay(2);

  // pwm_hw->slice[1].ctr = 0;
  // pwm_hw->slice[5].ctr = 783; // 781 to have quadrature but 791 seems better and every 8 or 4 increments -> something weird !
*/
  pinMode(4, OUTPUT);

  adc_gpio_init(26 + CAPTURE_CHANNEL);

  adc_init();
  adc_select_input(CAPTURE_CHANNEL);
  adc_fifo_setup(
      true,  // Write each completed conversion to the sample FIFO
      true,  // Enable DMA data request (DREQ)
      1,     // DREQ (and IRQ) asserted when at least 1 sample present
      false, // We won't see the ERR bit because of 8 bit reads; disable.
      true   // Shift each sample to 8 bits when pushing to FIFO
  );

  // Divisor of 0 -> full speed. Free-running capture with the divider is
  // equivalent to pressing the ADC_CS_START_ONCE button once per `div + 1`
  // cycles (div not necessarily an integer). Each conversion takes 96
  // cycles, so in general you want a divider of 0 (hold down the button
  // continuously) or > 95 (take samples less frequently than 96 cycle
  // intervals). This is all timed by the 48 MHz ADC clock.
  adc_set_clkdiv(0);
  adc_set_round_robin(0); // 1 seul adc

  sleep_ms(250);

  //  Set up the DMA to start transferring data as soon as it appears in FIFO
  dma_chan = dma_claim_unused_channel(true);
  cfg = dma_channel_get_default_config(dma_chan);

  // Reading from constant address, writing to incrementing byte addresses
  channel_config_set_transfer_data_size(&cfg, DMA_SIZE_16);
  channel_config_set_read_increment(&cfg, false);
  channel_config_set_write_increment(&cfg, true);
  channel_config_set_irq_quiet(&cfg, true);

  // Pace transfers based on availability of ADC samples
  channel_config_set_dreq(&cfg, DREQ_ADC);
  multicore_launch_core1(task);
  digitalWrite(LED_BUILTIN, 1);
  // gpio_set_irq_enabled_with_callback(5, GPIO_IRQ_EDGE_RISE, true, &cb);

  gpio_set_irq_enabled_with_callback(6, GPIO_IRQ_EDGE_RISE, true, &loopTask);
}

void loop() {}

void loopTask(uint /*gpio*/, uint32_t /*event_mask*/)
{
  static int countDown = 10; // set the zero at start up

  float Ival = 0,
        Qval = 0;

  static float I_old, Q_old;

  int N = CAPTURE_DEPTH;

#ifdef DEBUG_PULSE
  gpio_put(4, 1);
#endif

  // fait dans l'autre coeur
  /*for (int i = 0; i < N; ++i)
    buf[i] = capture_buf[i];*/

  for (int i = 0; i < N; i++)
  {
    Ival += buf[i] * sinus[i];
    Qval += buf[i] * cosinus[i];
  }

  float r = sqrt(Ival * Ival + Qval * Qval);
  Q = Qval / r;
  I = Ival / r;

#ifdef DEBUG_PULSE
  gpio_put(4, 0);
#endif
  // turn_frac = atan2f(Q, I) * 0.15915494309189533576888376337251f;
  turn_frac = atan2LUTif(Q, I) * 0.15915494309189533576888376337251f; // Attention pas de calcul double -> perte de synchro ??

#ifdef DEBUG_PULSE
  gpio_put(4, 1);
#endif

  if (Q >= 0 && Q_old < 0 && I < 0)
    turn--;
  if (Q < 0 && Q_old >= 0 && I < 0)
    turn++;

  turnTotal = turn + turn_frac;
  turnTotal_avg = epsilon * turnTotal + (1 - epsilon) * turnTotal_avg;

  if (countDown)
  {
    turnTotal0 = turnTotal;
    countDown--;
  }

  Q_old = Q;
  I_old = I;

  uint8_t GainSelection = digitalRead(pinGain1) + 2 * digitalRead(pinGain2);
  // GainSelection = 0;

  if (GainSelection == 0)
    factor = MM_TO_VOLT;
  if (GainSelection == 1)
    factor = MM5_TO_VOLT;
  if (GainSelection == 2)
    factor = CM_TO_VOLT;
  if (GainSelection == 3)
    factor = CM5_TO_VOLT;

  distance = constrain(50 + -(turnTotal - turnTotal0) * 4.25f * factor, 0, 100.0f);
  PWMQ->setPWM(14, 100000, distance);

#ifdef DEBUG_PULSE
  gpio_put(4, 0);
#endif
}
