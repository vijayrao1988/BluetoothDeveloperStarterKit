#include <Power.h>
#include <wsrc.h>
#include <Bounce2.h>

/*
 * Author: Vijay Rao
 *
 * Version History
 * V1.0: 
 * First version
 *
 *
 */
#define solenoidP 8
#define solenoidN 7
#define button 2
#define flowSensor 4


#include <CurieBLE.h>
#include <Time.h>
#include <TimeLib.h>
#include <TimeAlarms.h>

String inputString = "";         // a String to hold incoming data
boolean stringComplete = false;  // whether the string is complete
Bounce interruptButtonBouncer = Bounce();
static volatile uint16_t volume;

//Alarm IDs
AlarmId id[32];

// BLE objects
BLEPeripheral blePeripheral;


// GAP properties
char device_name[] = "Pebble";

// Characteristic Properties


//Pebble Characteristic Properties
unsigned char CurrentTimeService_CurrentTime_props = BLERead | BLEWrite | 0;
unsigned char BatteryService_BatteryLevel_props = BLERead | 0;
unsigned char PotsService_Pots_props = BLERead | BLEWrite | 0;
unsigned char TimePointService_NewPoint_props = BLEWrite | 0;
unsigned char ValveControllerService_Command_props = BLEWrite | 0;


char AttributeValue[32];

// Services and Characteristics
//Pebble Services and Characteristics
BLEService BatteryService("6E521ABEB56F4B058465A1CEE41BB141");
BLECharacteristic BatteryService_BatteryLevel("6E52C8E5B56F4B058465A1CEE41BB141", BatteryService_BatteryLevel_props, 1);
//1 byte = Total Length
//1 byte = uint8 Level
//BLEDescriptor BatteryService_BatteryLevel_CharacteristicPresentationFormat("2904", 0);

BLEService CurrentTimeService("6E52A9DAB56F4B058465A1CEE41BB141");
BLECharacteristic CurrentTimeService_CurrentTime("6E52AC46B56F4B058465A1CEE41BB141", CurrentTimeService_CurrentTime_props, 7);
//7 bytes = Total Length
//1 byte = uint8 hours
//1 byte = uint8 minutes
//1 byte = uint8 seconds
//1 byte = uint8 Days
//1 byte = uint8 Months
//2 byte = uint16 Years

BLEService PotsService("6E529F14B56F4B058465A1CEE41BB141");
BLECharacteristic PotsService_Pots("6E52E386B56F4B058465A1CEE41BB141", PotsService_Pots_props, 1);
//1 byte = Total Length
//1 byte = uint8 Number of Pots

BLEService TimePointService("6E52214FB56F4B058465A1CEE41BB141");
BLECharacteristic TimePointService_NewPoint("6E529480B56F4B058465A1CEE41BB141", TimePointService_NewPoint_props, 9);
//9 bytes = Total Length
//1 byte = uint8 Index (Time Point Number)
//1 byte = uint8 Day of the Week
//1 byte = uint8 hours
//1 byte = uint8 minutes
//1 byte = uint8 seconds
//2 bytes = uint16 Duration
//2 bytes = uint16 Volume

BLEService ValveControllerService("6E52C714B56F4B058465A1CEE41BB141");
BLECharacteristic ValveControllerService_Command("6E52CFDBB56F4B058465A1CEE41BB141", ValveControllerService_Command_props, 1);


void setup() {
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(button, INPUT);
  digitalWrite(button, HIGH);
  pinMode(flowSensor, INPUT);
  interruptButtonBouncer .attach(button);
  interruptButtonBouncer .interval(5);
  attachInterrupt(digitalPinToInterrupt(button), beep, FALLING);
  attachInterrupt(digitalPinToInterrupt(flowSensor), count, CHANGE);
  interrupts();
  

  digitalWrite(8, HIGH);
  digitalWrite(7, HIGH);

  digitalWrite(10, HIGH);  // turn the Buzzer on (HIGH is the voltage level)
  digitalWrite(11, HIGH);
  digitalWrite(12, HIGH);
  digitalWrite(13, HIGH);
  delay(50);              // wait for a second
  digitalWrite(10, LOW);  // turn the Buzzer on (HIGH is the voltage level)
  digitalWrite(11, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
  delay(50);              // wait for a second
  digitalWrite(10, HIGH);  // turn the Buzzer on (HIGH is the voltage level)
  digitalWrite(11, HIGH);
  digitalWrite(12, HIGH);
  digitalWrite(13, HIGH);
  delay(50);              // wait for a second
  digitalWrite(10, LOW);  // turn the Buzzer on (HIGH is the voltage level)
  digitalWrite(11, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
  delay(50);              // wait for a second
  digitalWrite(10, HIGH);  // turn the Buzzer on (HIGH is the voltage level)
  digitalWrite(11, HIGH);
  digitalWrite(12, HIGH);
  digitalWrite(13, HIGH);
  delay(100);              // wait for a second
  digitalWrite(10, LOW);  // turn the Buzzer on (HIGH is the voltage level)
  digitalWrite(11, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);

  char batteryLevel = 100;
  const char * batteryLevelPtr = &batteryLevel;
  //BatteryService_BatteryLevel.setValue(batteryLevel);            
  
  Serial.begin(9600);
  //while (! Serial); // Wait until Serial is ready
  Serial.println("setup()");

// set advertising packet content
  blePeripheral.setLocalName(device_name);


// add services and characteristics
  blePeripheral.addAttribute(CurrentTimeService);
  blePeripheral.addAttribute(CurrentTimeService_CurrentTime);
  
  blePeripheral.addAttribute(PotsService);
  blePeripheral.addAttribute(PotsService_Pots);
  
  blePeripheral.addAttribute(BatteryService);
  blePeripheral.addAttribute(BatteryService_BatteryLevel);
  
  blePeripheral.addAttribute(TimePointService);
  blePeripheral.addAttribute(TimePointService_NewPoint);

  blePeripheral.addAttribute(ValveControllerService);
  blePeripheral.addAttribute(ValveControllerService_Command);
  //blePeripheral.addAttribute(ValveControllerService_Start);
  //blePeripheral.addAttribute(ValveControllerService_Stop);
  //blePeripheral.addAttribute(ValveControllerService_Pause);
   
  blePeripheral.setAdvertisedServiceUuid("180F");

  
  Serial.println("attribute table constructed");
  // begin advertising
  blePeripheral.begin();

  BatteryService_BatteryLevel.setValue(batteryLevelPtr);
}


void solenoidOpen() {
   Serial.println("Opening Solenoid.");
   digitalWrite(solenoidP, LOW);
   digitalWrite(solenoidN, HIGH);
   delay(100);              // wait for a second
   digitalWrite(solenoidP, HIGH);
   digitalWrite(solenoidN, HIGH);
}

void solenoidClose() {
   Serial.println("Closing Solenoid.");
   digitalWrite(solenoidP, HIGH);
   digitalWrite(solenoidN, LOW);
   delay(100);              // wait for a second
   digitalWrite(solenoidP, HIGH);
   digitalWrite(solenoidN, HIGH);
}

void beep() {
  //PM.wakeFromDoze();
  noInterrupts();
  digitalWrite(2, LOW);
  pinMode(2, OUTPUT);
  Serial.println("Interrupts disabled. External Button Interrupt Triggered.");
  digitalWrite(13, HIGH);
  delay(50);              // wait for a second
  digitalWrite(13, LOW);
  Alarm.timerOnce(10, debounce);
  // begin advertising
  blePeripheral.begin();
  Serial.println("advertising");
}

void count() {
  volume++;
}

void debounce() {
  Serial.println("Interrupts re-enabled.");
  interrupts();
  pinMode(2, INPUT);
  blePeripheral.end();
  Serial.println("Advertising stopped");
  Alarm.delay(1000);
  //PM.doze();
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}

void loop() {
  // listen for BLE peripherals to connect:
  BLECentral central = blePeripheral.central();
  time_t t;
  
  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    

    // while the central is still connected to peripheral:
    while (central.connected()) {
        Alarm.delay(0);
        //Serial.print("Volume : ");
        //Serial.println(volume);

        

        ////////////////////
        //Time Synchronization Service
        ////////////////////
        if (CurrentTimeService_CurrentTime.written()) {
        // application logic for handling WRITE or WRITE_WITHOUT_RESPONSE on characteristic Current Time Service Current Time goes here
         digitalWrite(13, HIGH);
         delay(50);              // wait for a second
         digitalWrite(13, LOW);
         delay(50);              // wait for a second
   
         Serial.println(CurrentTimeService_CurrentTime.valueLength());
         sprintf(AttributeValue,"%c",NULL);
         strncpy(AttributeValue,(char*)CurrentTimeService_CurrentTime.value(),CurrentTimeService_CurrentTime.valueLength());
         Serial.println(AttributeValue);
         
         Serial.println("CurrentTimeService_CurrentTime.written()");
         Serial.print(AttributeValue[0], HEX);
         Serial.print(",");
         Serial.print(AttributeValue[1], HEX);
         Serial.print(",");
         Serial.print(AttributeValue[2], HEX);
         Serial.print(",");
         Serial.print(AttributeValue[3], HEX);
         Serial.print(",");
         Serial.print(AttributeValue[4], HEX);
         Serial.print(",");
         Serial.print(AttributeValue[5], HEX);
         Serial.print(",");
         Serial.print(AttributeValue[6], HEX);
         Serial.println(".");

         // setting up system time
        setTime(AttributeValue[0], AttributeValue[1], AttributeValue[2], AttributeValue[3], AttributeValue[4], ((256 * (int) AttributeValue[5]) + (int) AttributeValue[6]));
        t = now();
        Serial.print(hour(t));
        Serial.print(":");
        Serial.print(minute(t));
        Serial.print(":");
        Serial.print(second(t));
        Serial.print(",");
        Serial.print(day(t));
        Serial.print("/");
        Serial.print(month(t));
        Serial.print("/");
        Serial.print(year(t));
        Serial.print(",");
        Serial.print(dayStr(weekday(t)));
        Serial.println(".");
        }

        ////////////////////
        //Pots Service
        ////////////////////
        if (PotsService_Pots.written()) {
        // application logic for handling WRITE or WRITE_WITHOUT_RESPONSE on characteristic Pots goes here
         Serial.println(PotsService_Pots.valueLength());
         sprintf(AttributeValue,"%c",NULL);
         strncpy(AttributeValue,(char*)PotsService_Pots.value(),PotsService_Pots.valueLength());
         Serial.println(AttributeValue);
         
         Serial.println("PotsService_Pots.written()");
         Serial.print(AttributeValue[0], HEX);
         Serial.println(".");
         solenoidOpen();
         digitalWrite(13, HIGH);
         delay(50);              // wait for a second
         digitalWrite(13, LOW);
         delay(50);              // wait for a second
   
        }

        ////////////////////
        //Valve Controller Commands
        ////////////////////
        if (ValveControllerService_Command.written()) {
        // application logic for handling WRITE or WRITE_WITHOUT_RESPONSE on characteristic Pots goes here
         Serial.println(ValveControllerService_Command.valueLength());
         sprintf(AttributeValue,"%c",NULL);
         strncpy(AttributeValue,(char*)ValveControllerService_Command.value(),ValveControllerService_Command.valueLength());
         Serial.println(AttributeValue);
         
         Serial.println("ValveControllerService_Command.written()");
         switch(AttributeValue[0])
         {
          case 1:
            Serial.println("Flush written");
          break;

          case 2:
            Serial.println("Start written");
          break;

          case 3:
            Serial.println("Stop written");
          break;

          case 4:
            Serial.println("Pause written");
          break;

          default:
            Serial.println("Unknown command");
          break;
         }
         Serial.println(".");
         solenoidOpen();
         digitalWrite(13, HIGH);
         delay(50);              // wait for a second
         digitalWrite(13, LOW);
         delay(50);              // wait for a second
   
        }


        ////////////////////
        //Time Points Service
        ////////////////////
        if (TimePointService_NewPoint.written()) {
        // application logic for handling WRITE or WRITE_WITHOUT_RESPONSE on characteristic Current Time Service Current Time goes here
         Serial.println(TimePointService_NewPoint.valueLength());
         sprintf(AttributeValue,"%c",NULL);
         strncpy(AttributeValue,(char*)TimePointService_NewPoint.value(),TimePointService_NewPoint.valueLength());
         Serial.println(AttributeValue);
         
         Serial.println("TimePointService_NewWateringTimePoint.written()");
         Serial.print(AttributeValue[0], HEX);
         Serial.print(",");
         Serial.print(AttributeValue[1], HEX);
         Serial.print(",");
         Serial.print(AttributeValue[2], HEX);
         Serial.print(",");
         Serial.print(AttributeValue[3], HEX);
         Serial.print(",");
         Serial.print(AttributeValue[4], HEX);
         Serial.print(",");
         Serial.print(AttributeValue[5], HEX);
         Serial.print(",");
         Serial.print(AttributeValue[6], HEX);
         Serial.print(",");
         Serial.print(AttributeValue[7], HEX);
         Serial.print(",");
         Serial.print(AttributeValue[8], HEX);
         Serial.println(".");

         if(AttributeValue[0] == 0)
         {
          //reset all alarms for time point index 0
          int i;
          for(i = 0; i < 32; i++)
          {
            Alarm.free(id[i]);
          }
          Serial.println("Old time points erased.");

         }
         else
         {
          //set alarm as per time point index (1 - 32)
          switch(AttributeValue[1])
          {
            case 1: //Sunday
              id[AttributeValue[0]-1] = Alarm.alarmRepeat(dowSunday, (int) AttributeValue[2], (int) AttributeValue[3], (int) AttributeValue[4], solenoidOpen);
              Serial.print("Setting Alarm - Sunday ");
              Serial.print(AttributeValue[2], DEC);
              Serial.print(":");
              Serial.print(AttributeValue[3], DEC);
              Serial.print(":");
              Serial.println(AttributeValue[4], DEC);
            break;

            case 2: //Monday
              id[AttributeValue[0]-1] = Alarm.alarmRepeat(dowMonday, (int) AttributeValue[2], (int) AttributeValue[3], (int) AttributeValue[4], solenoidOpen);
              Serial.print("Setting Alarm - Monday ");
              Serial.print(AttributeValue[2], DEC);
              Serial.print(":");
              Serial.print(AttributeValue[3], DEC);
              Serial.print(":");
              Serial.println(AttributeValue[4], DEC);
            break;

            case 3: //Tuesday
              id[AttributeValue[0]-1] = Alarm.alarmRepeat(dowTuesday, (int) AttributeValue[2], (int) AttributeValue[3], (int) AttributeValue[4], solenoidOpen);
              Serial.print("Setting Alarm - Tuesday ");
              Serial.print(AttributeValue[2], DEC);
              Serial.print(":");
              Serial.print(AttributeValue[3], DEC);
              Serial.print(":");
              Serial.println(AttributeValue[4], DEC);
            break;

            case 4: //Wednesday
              id[AttributeValue[0]-1] = Alarm.alarmRepeat(dowWednesday, (int) AttributeValue[2], (int) AttributeValue[3], (int) AttributeValue[4], solenoidOpen);
              Serial.print("Setting Alarm - Wednesday ");
              Serial.print(AttributeValue[2], DEC);
              Serial.print(":");
              Serial.print(AttributeValue[3], DEC);
              Serial.print(":");
              Serial.println(AttributeValue[4], DEC);
            break;

            case 5: //Thursday
              id[AttributeValue[0]-1] = Alarm.alarmRepeat(dowThursday, (int) AttributeValue[2], (int) AttributeValue[3], (int) AttributeValue[4], solenoidOpen);
              Serial.print("Setting Alarm - Thursday ");
              Serial.print(AttributeValue[2], DEC);
              Serial.print(":");
              Serial.print(AttributeValue[3], DEC);
              Serial.print(":");
              Serial.println(AttributeValue[4], DEC);
            break;

            case 6: //Friday
              id[AttributeValue[0]-1] = Alarm.alarmRepeat(dowFriday, (int) AttributeValue[2], (int) AttributeValue[3], (int) AttributeValue[4], solenoidOpen);
              Serial.print("Setting Alarm - Friday ");
              Serial.print(AttributeValue[2], DEC);
              Serial.print(":");
              Serial.print(AttributeValue[3], DEC);
              Serial.print(":");
              Serial.println(AttributeValue[4], DEC);
            break;

            case 7: //Saturday
              id[AttributeValue[0]-1] = Alarm.alarmRepeat(dowSaturday, (int) AttributeValue[2], (int) AttributeValue[3], (int) AttributeValue[4], solenoidOpen);
              Serial.print("Setting Alarm - Saturday ");
              Serial.print(AttributeValue[2], DEC);
              Serial.print(":");
              Serial.print(AttributeValue[3], DEC);
              Serial.print(":");
              Serial.println(AttributeValue[4], DEC);
            break;
          }
         }
         
         
         digitalWrite(13, HIGH);
         delay(50);              // wait for a second
         digitalWrite(13, LOW);
         delay(50);              // wait for a second
         
        }


        ////////////////////
        //Battery Service
        ////////////////////
        if (BatteryService_BatteryLevel.read()) {
          Serial.println("Battery Level Read");
        }
        
    }
    // when the central disconnects, print it out:
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}

