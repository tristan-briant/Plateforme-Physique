#include <M5core2.h>
#include <phyphoxBle.h>

/*  In this example we can change the blink interval of our mikrocontroller via phyphox
 */

float freq = 0, ampl = 0;

void receivedData();
void newExperimentEvent();

uint8_t experiment[] =
    R"rawliteral(<phyphox version="1.10">
    <title>toto</title>
    <category>test</category>
    <color>23AA47</color>
    <description></description>
    <icon format="string">Toto</icon>
    <data-containers>
        <container size="1" init="0">valuefreq</container>
		<container size="1" init="0">valueampl</container>
		<container size="1" init="0">valueoffs</container>
    </data-containers>
	
	<input>
        <bluetooth id="toto" name="toto"  mode="notification" rate="1" subscribeOnStart="false">
            <output char="cddf1002-30f7-4671-8b43-5e40ba53514a" conversion="float32LittleEndian" offset="0" >valuefreq</output>
			<output char="cddf1002-30f7-4671-8b43-5e40ba53514a" conversion="float32LittleEndian" offset="4" >valueampl</output>
			<output char="cddf1002-30f7-4671-8b43-5e40ba53514a" conversion="float32LittleEndian" offset="8" >valueoffs</output>
        </bluetooth>
    </input>
	
    <!--<output>
        <bluetooth id="toto">
            <input char="cddf1003-30f7-4671-8b43-5e40ba53514a" conversion="byteArray">valeur</input>
        </bluetooth>
    </output>
	-->
    <analysis sleep="1" onUserInput="false">
    </analysis>

    <views>        
        <view label="Oscillator">
           			
			<edit label="freq" signed="false" decimal="true" unit="Hz"  factor="1" default="1">
                <output>valuefreq</output>
            </edit>
			<edit label="ampl" signed="false" decimal="true" unit="Hz"  factor="1" default="1">
                <output>valueampl</output>
            </edit>
			<edit label="offset" signed="false" decimal="true" unit="Hz"  factor="1" default="1">
                <output>valueoffs</output>
            </edit>
			
            <button label="Set parameter">
                <input type="value">1</input>
                <output>valuefreq</output>			
            </button>	
			
		</view>
    </views>
    <export>
    </export>
</phyphox>
)rawliteral";

long lastTimestamp = 0;
float blinkInterval = 100;
bool led = true;

void setup()
{

    M5.begin();
    Serial.begin(115200);
    PhyphoxBLE::start("toto-1", &experiment[0], sizeof(experiment));
    // PhyphoxBLE::start("toto");
    PhyphoxBLE::configHandler = &receivedData;
    PhyphoxBLE::experimentEventHandler = &newExperimentEvent; // declare which function should be called after receiving an experiment event
    // PhyphoxBLE::printXML(&Serial);

    // PhyphoxBLE::experimentEventHandler=&receivedData;
    //  PhyphoxBLE::setMTU(176);

    // pinMode(LED_BUILTIN, OUTPUT);

    // Experiment
    /*PhyphoxBleExperiment getDataFromSmartphone;
    getDataFromSmartphone.setTitle("Set Blink Interval");
    getDataFromSmartphone.setCategory("Arduino Experiments");
    getDataFromSmartphone.setDescription("User can set Blink Interval of Mikrocontroller LED");

    // View
    PhyphoxBleExperiment::View firstView;
    firstView.setLabel("FirstView"); // Create a "view"

    // Edit
    PhyphoxBleExperiment::Edit Interval;
    Interval.setLabel("Interval");
    Interval.setUnit("ms");
    Interval.setSigned(false);
    Interval.setDecimal(false);
    Interval.setChannel(1);

    firstView.addElement(Interval);
    getDataFromSmartphone.addView(firstView);         // attach view to experiment
    PhyphoxBLE::addExperiment(getDataFromSmartphone); // attach experiment to server*/
}

float x = 0;

void loop()
{

    PhyphoxBLE::poll(); // Only required for the Arduino Nano 33 IoT, but it does no harm for other boards.

    if (millis() - lastTimestamp > blinkInterval)
    {
        lastTimestamp = millis();
        led = !led;
        // digitalWrite(LED_BUILTIN, led);
        M5.Axp.SetLed(led);

        uint8_t Data[20] = {0};
        x = x + 1;
        // Data[0] = x++;
        PhyphoxBLE::write(x);
    }
}

void receivedData()
{
    uint8_t Data[20] = {0};
    // float receivedInterval;
    PhyphoxBLE::read(&Data[0], 12);
    M5.Lcd.textsize = 2;
    M5.Lcd.clear();
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.printf("%d   %d   %d   %d   %d   %d ", Data[0], Data[1], Data[2], Data[3], Data[4], Data[5]);

    freq = Data[2] + 0.01 * Data[3];
    ampl = Data[4] + 0.01 * Data[5];
    M5.Lcd.setCursor(0, 30);
    M5.Lcd.printf("freq = %6.2f   Ampl = %6.2f", freq, ampl);
    // Serial.println(blinkInterval);
}

void newExperimentEvent()
{
    Serial.println("New experiment event received:");
    Serial.print("Event type: ");
    Serial.print(PhyphoxBLE::eventType);
    Serial.println(" (0 = Paused, 1 = Started, 255 = SYNC)");
    Serial.print("Experiment time [ms]: ");
    Serial.println(PhyphoxBLE::experimentTime);
    Serial.print("Unix system time [ms]: ");
    Serial.println(PhyphoxBLE::systemTime);

    if (PhyphoxBLE::eventType == 255)
    {
        float freq = 12.5;
        PhyphoxBLE::write(freq);
    }

    if (PhyphoxBLE::eventType == 1)
    {
        float param[3] = {85.25, 25.1, 1.02};
        // delay(500);
        PhyphoxBLE::write(param, 3);
    }
}