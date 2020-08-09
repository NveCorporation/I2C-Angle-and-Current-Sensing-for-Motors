/**************************************************************************************  
Demonstrates motor motor rotation and current sensing using NVE I²C Smart Sensors. 
AG966 angle sensor, I²C address = 16 (I2CADDR=OPEN; Arduino address = 0x24)
AG969 current sensor, I²C address = 72 (I2CADDR=LOW; Arduino address = 0x08)
16x2 OpenLCD display, Arduino I²C address = 0x72
Trinket I²C master connections: SDA=#0; SCL=#2; LED=#1; USB for programming=#3/#4 
See https://www.nve.com/webstore/catalog for I2C sensors and breakout boards
**************************************************************************************/
#include <TinyWireM.h> //ATTiny85 I²C Master library
//Use Wire.h for Arduino and replace "TinyWireM." with "Wire."
int angle, old_angle, angle_change, RPM, current; //Sensor values
byte Dir; //Angle sensor direction, sample counter
long angle_time, old_time; //Timer values used for RPM calculation
const int currentOffset = 0; //Current calibration factors
const long mA_per_mT = 40000;

void setup() {
TinyWireM.begin();
TinyWireM.beginTransmission(0x72); //Write to LCD (I²C address 0x72)
TinyWireM.write('|'); //Send setting character
TinyWireM.write('-'); //Send clear display character
TinyWireM.write('|');
TinyWireM.write(157); //Max backlight Red
TinyWireM.write('|');
TinyWireM.write(187); //Max backlight Green
TinyWireM.write('|');
TinyWireM.write(217); //Max backlight Blue
TinyWireM.endTransmission();
}
void loop() {
//Read angle 
TinyWireM.beginTransmission(0x24); //Angle sensor
TinyWireM.write(0); //Select angle
TinyWireM.endTransmission();
TinyWireM.requestFrom(0x24,2);
old_angle = angle;
old_time = angle_time;
angle = TinyWireM.read(); //Read MSB
angle=(angle<<8)|TinyWireM.read(); //Read LSB

//Calculate RPM
angle_time=micros();
angle_change=abs(angle-old_angle);
if (angle_change>1800) angle_change=3600-angle_change; //Correct for zero crossing
RPM=angle_change*16667L/(angle_time-old_time);

//Read direction
TinyWireM.beginTransmission(0x24);
TinyWireM.write(3); //Select angle sensor direction output
TinyWireM.endTransmission();
TinyWireM.requestFrom(0x24,2);
Dir = TinyWireM.read(); //Read dummy byte
Dir = TinyWireM.read();

//Read current
TinyWireM.requestFrom(8,2); //Current sensor (Arduino I²C address = 0x24; 2 bytes)
current = TinyWireM.read(); //Read MSB
current = mA_per_mT*(current<<8|TinyWireM.read())/10000+currentOffset; //Get LSB; convert to 100ths of amps 

//Display RPM
TinyWireM.beginTransmission(0x72); //Address display 
TinyWireM.write(254); //Send command character
TinyWireM.write(128 + 0 + 0); //Change position (128) of cursor to row 1 (0), col. 0
TinyWireM.write(0x30+RPM/100); //Hundreds of RPM (in ASCII) 
TinyWireM.write(0x30+(RPM%100)/10); //Tens of RPM
TinyWireM.write(0x30+(RPM%10)); //Ones
TinyWireM.write(' '); TinyWireM.write('r'); TinyWireM.write('p'); TinyWireM.write('m'); TinyWireM.write(' '); 

//Display direction
if (Dir) (TinyWireM.write('C')); else TinyWireM.write(' '); 
TinyWireM.write('C'); TinyWireM.write('W'); TinyWireM.write(' '); 

//Display angle
TinyWireM.endTransmission();
TinyWireM.beginTransmission(0x72); //Address display 
TinyWireM.write(0x30+angle/1000); //Hundreds of degrees (in ASCII) 
TinyWireM.write(0x30+(angle%1000)/100); //Tens of degrees 
TinyWireM.write(0x30+(angle%100)/10); //Degrees 
TinyWireM.write(0xDF); //Degree sign
TinyWireM.endTransmission();

//Display current
TinyWireM.beginTransmission(0x72); //Address display 
if (current<0) (TinyWireM.write('-')); else TinyWireM.write('+'); 
current=abs(current);
TinyWireM.write(0x30+current/100); //Amps (in ASCII)
TinyWireM.write('.');
TinyWireM.write(0x30+(current%100)/10); //Tenths of amps 
TinyWireM.write(0x30+(current%10)); //Hundredths of amps 
TinyWireM.write(' '); TinyWireM.write('A'); TinyWireM.write('m'); TinyWireM.write('p');
TinyWireM.endTransmission();

delay(100); //10 samples/sec (max. RPM = 0.5 rev x 600 samples/min. = 300 RPM)
}
