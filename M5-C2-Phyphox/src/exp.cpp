const char experiment[]=
R"rawliteral(<phyphox xmlns="http://phyphox.org/xml" xmlns:editor="http://phyphox.org/editor/xml" version="1.7" editor:version="1.0" locale="">
    <title>CO₂-Monitor Maintenance</title>
    <category>CO₂-Sensor</category>
    <color>23AA47</color>
    <description>Maintenance and configuration of the CO₂ sensor.</description>
    <icon format="string">CO₂</icon>
    <data-containers>
        <container size="1" static="false">co_in</container>
        <container size="1" static="false">temp_in</container>
        <container size="1" static="false">hum_in</container>
        <container size="1" static="false">time_in</container>
        <container size="1" init="0" static="false">calibrationMode</container>
        <container size="1" init="0" static="false">temp_offset</container>
        <container size="1" init="0" static="false">temp_offset10</container>
        <container size="1" init="0" static="false">calButton</container>
        <container size="6" static="false">byteArray</container>
        <container size="1" init="0" static="false">statusCoCalibration</container>
        <container size="1" init="0" static="false">currentOffset</container>
        <container size="1" init="0" static="false">version</container>
        <container size="6" static="false">byteArray (1)</container>
    </data-containers>
    <input>
        <bluetooth editor:uuid="103" editor:posx="19.866668701171875" editor:posy="401.8666687011719" id="CO2" name="CO2"  mode="notification" rate="1" subscribeOnStart="false">
            <output char="cddf1002-30f7-4671-8b43-5e40ba53514a" conversion="singleByte">currentOffset</output>
            <output char="cddf1002-30f7-4671-8b43-5e40ba53514a" conversion="singleByte" offset="1">statusCoCalibration</output>
            <output char="cddf1002-30f7-4671-8b43-5e40ba53514a" conversion="singleByte" offset="2">version</output>
            <output char="cddf1002-30f7-4671-8b43-5e40ba53514a" conversion="float32LittleEndian" offset="3">co_in</output>
            <output char="cddf1002-30f7-4671-8b43-5e40ba53514a" conversion="float32LittleEndian" offset="7">temp_in</output>
            <output char="cddf1002-30f7-4671-8b43-5e40ba53514a" conversion="float32LittleEndian" offset="11">hum_in</output>
            <output char="cddf1002-30f7-4671-8b43-5e40ba53514a" conversion="float32LittleEndian" offset="15">time_in</output>
        </bluetooth>
    </input>
    <output>
        <bluetooth editor:uuid="106" editor:posx="1595.86669921875" editor:posy="347.8666687011719" id="CO2"  >
            <input char="cddf1003-30f7-4671-8b43-5e40ba53514a" conversion="byteArray">byteArray (1)</input>
        </bluetooth>
    </output>
    <analysis sleep="0.2"  onUserInput="false">
        <multiply editor:uuid="152" editor:posx="500" editor:posy="100">
            <input as="factor" type="value">10</input>
            <input as="factor" clear="false">temp_offset</input>
            <output as="product">temp_offset10</output>
        </multiply>
        <append editor:uuid="155" editor:posx="755.86669921875" editor:posy="292.86669921875">
            <input as="in" type="value">0</input>
            <output as="out">calButton</output>
        </append>
        <append editor:uuid="153" editor:posx="1210.300048828125" editor:posy="259.3000030517578">
            <input as="in" type="value">0</input>
            <input as="in">calibrationMode</input>
            <input as="in" type="value">0</input>
            <input as="in" type="value">0</input>
            <input as="in" type="value">0</input>
            <input as="in" clear="false">temp_offset10</input>
            <output as="out">byteArray</output>
        </append>
        <if editor:uuid="154" editor:posx="1216.86669921875" editor:posy="528.8666687011719" less="false" equal="true" greater="false">
            <input as="a" clear="false">calButton</input>
            <input as="b" type="value">0</input>
            <input as="true" type="empty" />
            <output as="result">byteArray (1)</output>
        </if>
    </analysis>
    <views>
        <view label="CO₂">
            <value editor:uuid="162" editor:posx="407.8666687011719" editor:posy="475.76666259765625" label="CO₂" size="1" precision="2" scientific="false" unit="ppm" factor="1" >
                <input>co_in</input>
            </value>
            <separator editor:uuid="163" height="1" color="404040">
            </separator>
            <value editor:uuid="164" editor:posx="410.8666687011719" editor:posy="672.86669921875" label="Temperature" size="1" precision="2" scientific="false" unit="°C" factor="1" >
                <input>temp_in</input>
            </value>
            <separator editor:uuid="165" height="1" color="404040">
            </separator>
            <value editor:uuid="166" editor:posx="403.8666687011719" editor:posy="780.86669921875" label="Humidity" size="1" precision="2" scientific="false" unit="%" factor="1" >
                <input>hum_in</input>
            </value>
            <separator editor:uuid="167" height="1" color="404040">
            </separator>
            <info editor:uuid="168" label="Place the CO₂-Monitor outside or in front of an opened window to guarantee fresh air. Wait for 5 minutes to exchange the air in the measuring chamber. Afterwards press the calibration button below to set the CO₂-Value to 400 ppm" >
            </info>
            <separator editor:uuid="169" height="1" color="404040">
            </separator>
            <separator editor:uuid="170" height="0.1" color="white">
            </separator>
            <separator editor:uuid="171" height="1" color="404040">
            </separator>
            <value editor:uuid="172" editor:posx="415.433349609375" editor:posy="379.83331298828125" label="Status:" size="1" precision="2" scientific="false"  factor="1" >
                <input>statusCoCalibration</input>
                <map min="0" max="0">waiting for calibration</map>
                <map min="1" max="1">calibration received</map>
            </value>
            <separator editor:uuid="173" height="1" color="404040">
            </separator>
            <separator editor:uuid="174" height="0.1" color="white">
            </separator>
            <separator editor:uuid="175" height="1" color="404040">
            </separator>
            <button editor:uuid="176" editor:posx="753.5999755859375" editor:posy="415.79998779296875" label="Calibrate with fresh air">
                <input type="value">1</input>
                <output>calButton</output>
                <input type="value">1</input>
                <output>calibrationMode</output>
            </button>
        </view>
        <view label="Temperature">
            <value editor:uuid="177" editor:posx="406.73333740234375" editor:posy="577.7333374023438" label="SCD30 Temperature" size="1" precision="2" scientific="false" unit="°C" factor="1" >
                <input>temp_in</input>
            </value>
            <separator editor:uuid="178" height="1" color="404040">
            </separator>
            <info editor:uuid="179" label="Due to the housing, the temperature near the sensor may heat up, causing it to indicate a higher temperature than the actual ambient temperature. To reduce this effect, you may enter a temperature offset." >
            </info>
            <separator editor:uuid="180" height="1" color="404040">
            </separator>
            <separator editor:uuid="181" height="0.1" color="white">
            </separator>
            <separator editor:uuid="182" height="1" color="404040">
            </separator>
            <edit editor:uuid="183" editor:posx="198.86666870117188" editor:posy="123.86666870117188" label="Offset" signed="false" decimal="true"   unit="°C" factor="1" default="0">
                <output>temp_offset</output>
            </edit>
            <separator editor:uuid="184" height="1" color="404040">
            </separator>
            <value editor:uuid="185" editor:posx="416.29998779296875" editor:posy="288.29998779296875" label="Current used offset" size="1" precision="2" scientific="false" unit="°C" factor="0.1" >
                <input>currentOffset</input>
            </value>
            <separator editor:uuid="186" height="1" color="404040">
            </separator>
            <separator editor:uuid="187" height="0.1" color="white">
            </separator>
            <separator editor:uuid="188" height="1" color="404040">
            </separator>
            <button editor:uuid="189" editor:posx="752.7333374023438" editor:posy="540.7333374023438" label="Set Temperatureoffset">
                <input type="value">2</input>
                <output>calButton</output>
                <input type="value">2</input>
                <output>calibrationMode</output>
            </button>
        </view>
    </views>
    <export>
    </export>
</phyphox>
)rawliteral";