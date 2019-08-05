/*!
 * @file userdefined.h
 *
 * @brief variable and function definitions
 *
 * This is the files that contains the variables, macros, pins definitions and user defined functions.
 * 
 * @author Sajjad Hussain
 *
 *  @date 02.08.2019
 */

//**********************INCLUDE FILES******************************

//#define RELEASE // uncomment to switch servers to release version, enable sending to madavi and luftdaten.info, and supress some debug output

#include <ESP8266WiFi.h>
#include <Ticker.h>           // Ticker Library
#include <Wire.h>             // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"      // legacy include: #include "SSD1306.h"
//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include "images.h"           // Include custom images
#include "Adafruit_DRV2605.h"
#include <Adafruit_LSM9DS1.h>

//***************************CONSTANTS/DEFINES*************************************
///Debounce time (ms) for push buttons
#define DEBOUNCE_TIME 10   
///Holdtime time start after these (ms) for push buttons                   
#define HOLDSTART_TIME 1000                   
///mqqt port                        
#define mqtt_port 1883 
///mqtt user (option)
#define MQTT_USER "Sajjad"       
///mqtt password (option)             
#define MQTT_PASSWORD "#sajjan2"              
///mqtt channel to publish audio messages
#define PUBLISH__AUDIO_CH "intensechoi/controller/audio"  
///mqtt channel to publish normal messages
#define PUBLISH_CH "intensechoi/controller/get"           
///mqtt channel to subcribe for normal messages
#define SUBSCRIBE_CH "intensechoi/controller/set"         


//***********************GLOBAL VARIABLES**************************
///mqtt broker, may be different
const char* mqtt_server = "broker.mqtt-dashboard.com";    
//const char* mqtt_server = "broker.hivemq.com";   //mqtt broker, http://www.hivemq.com/demos/websocket-client/ 
//const char* mqtt_server = "192.168.178.33";    //mqtt broker, may be different
/// Your Wifi SSID
const char* ssid = "UPC0870375";                
//const char* ssid ="tecoLecture"; 
/// Your Wifi Password               
const char* password = "Ghnfdd6byuxp";                
//const char* password = "PerComSS16";                



///States for incomming MQTT messages
unsigned char wState;         
///Your WiFi Client
WiFiClient wifiClient;        
///Array of hold a device and 8 mappings in the recieved MQTT message
String arr[9];                
///Array of 8 mapped device from the recieved MQTT message
String device[8]={"light", "tv", "-", "thermo", "-","window", "-", "-"}; 
///Array of 8 mapped actions from the recieved MQTT message
String actions[8]={"on", "off", "red", "blue", "green","dim", "-", "-"};
///Selected device from the MQTT Messages
String mapDevice[1] = {device[0]};    
///Selected action from the MQTT Messages
String mapAction[1] = {actions[0]};    
///contexed device from the MQTT Messages
String contextDevice[1] = {device[0]};  
///Joystick or Accelerometer differnt move events
boolean moveEvent;              
///Serial Port states, to handle
unsigned char rxState;          
///Push Button States, to handle clicked, released, hold and hold end
unsigned char btnState=0;       
///Serial Port recieved character
unsigned char rxData;                   
///handles two screens of the display to cater 8 device/actions
boolean secondScreen;                   
///moveEvent index
unsigned char optIndex;                 
///handles different display modes/screens 1,2,3,4
unsigned char displayMode=10;           
///handles different vibration modes, 1,2,3,4
unsigned char vibrationMode=-1;         
///handles different buzzer moder 1,2,3,4
unsigned char buzzerMode=-1;            
///older display modes/screens
unsigned char oldDisplayMode=10;        
///older vibration modes,
boolean oldVibrationMode=-1;      
///older buzzer moder
boolean oldBuzzerMode=-1;         
///Count the time in a screen mode
unsigned char cntDispScreenTime;        
///Microphone Audio Buffer read from ADC
static unsigned char aud[40960];        
///Microphone Audio Buffer to be sent to MQTT
char audMQTT[128];                    
///Count 125us samples from microphone
static unsigned int cnt125us;           
///Recording is done
boolean recDone;              
///Start recording
boolean recording;            
/// if there is a serial character
boolean rxflag=false;         
/// if this serial character is valid
boolean rxValid=false;        
/// Audio packet index
static int pktInx;            
/// Audio packet counting
static int pktCnt;            
/// handles accelerometers statemachine
unsigned char gState =0;      
/// counts accelerometers logging time
unsigned char cntms=0;        
/// accelerometers current direction
unsigned char curDirection;   
///number of readings to average out the accelerometer readings
const int numReadings = 5;   
///number of axis of accelerometers
const int numAxis =3;         
///  the accelerometer reading history
float readings[numAxis][numReadings];  
/// the accelerometer index of the current reading
int readIndex[numAxis];              
/// the accelerometer running total
float total[numAxis];                  
/// the accelerometer average
float average[numAxis];                
/// mapped acclerometer x axis value
float axm;          
/// mapped acclerometer y axis value
float aym;          
/// mapped acclerometer z axis value
float azm;          
///sensor event for accelerometer
sensors_event_t a; 
///sensor event for magnetomete
sensors_event_t m; 
///sensor event for gyroscope
sensors_event_t g;  
/// previous values of 25ms logged
unsigned long pre25ms;    
/// previous values of 500ms logged
unsigned long pre500ms;    
/// previous values of 3000ms logged
unsigned long pre3000ms;    
/// previous values of 4000ms logged
unsigned long pre4000ms;    
/// publish another action when holding action button >4s
boolean anotherAction=false; 

// variable used for the key press
///if Context Push Button is pressed?
volatile boolean contextKeyPressed = false;   
///if Context Push Button gives first edge?
volatile boolean contextFirstEdge = false;    
///if Context Push Button starts holding?
volatile boolean contextHolding = false;      
///if voice Push Button is pressed?
volatile boolean voiceKeyPressed = false;   
///if voice Push Button gives first edge?
volatile boolean voiceFirstEdge = false;    
///if voice Push Button starts holding?
volatile boolean voiceHolding = false;      
///if action Push Button is pressed?
volatile boolean actionKeyPressed = false;   
///if action Push Button gives first edge?
volatile boolean actionFirstEdge = false;   
///if action Push Button starts holding?
volatile boolean actionHolding = false;      

// variable used for the debounce
///occurance time of Context key pressed
unsigned long timeContextKeyPress = 0;      
///last occurance time of Context key pressed
unsigned long timeContextLastPress = 0;      
///occurance time of Voice key pressed
unsigned long timeVoiceKeyPress = 0;      
///last occurance time of Voice key pressed
unsigned long timeVoiceLastPress = 0;      
///occurance time of Action key pressed
unsigned long timeActionKeyPress = 0;      
///last occurance time of Action key pressed
unsigned long timeActionLastPress = 0;      

/// handles the states of Context button
boolean swContextState;         
/// handles the states of Voice button
boolean swVoiceState;         
/// handles the states of Action button
boolean swActionState;         



/// instance of Wifi Client
PubSubClient client(wifiClient);   
/// instance of UDP Packet transmission to test audio
WiFiUDP Udp;                        
/// instance of driver motor
Adafruit_DRV2605 drv;               
/// instance of accelerometer gyroscope and magnetometer
Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();
/// Serial port with Rx,Tx pins
SoftwareSerial s(4,0);                    
/// I2C Interface with Address, SDA, SCL
SSD1306Wire  oled(0x3d, 2, 14);           

//**********************************HARDWARE PINS*************************************

/// Context Push Button
const int swContext = 12;  
/// Voice Push Button   
const int swVoice = 13;     
/// Action Push Button
const int swAction = 5;     
/// pasive buzzer pin
const int buzzerPin = 16;   
///trigger interrupt from the slave
const int interruptPin = 15;  
///microphone input
const int mic = A0;           

//*****************************FUNCTION PROTOTYPES*******************************


int frequency(char note);
void smooth(int axis, float val) ;
void setVibrationMode(unsigned char vm);
void setBuzzerMode(unsigned char bm);
void sendCmd2Slave(unsigned char cmd);
void parsing( byte *payload);
void reconnect(void) ;
void setup_wifi(void);
void setDisplayMode(unsigned char om);
void callback(char* topic, byte *payload, unsigned int length) ;
void ICACHE_RAM_ATTR ContextKeyIsPressed(void);
void ICACHE_RAM_ATTR VoiceKeyIsPressed(void);
void ICACHE_RAM_ATTR ActionKeyIsPressed(void);
void ICACHE_RAM_ATTR SerialTriggerISR(void);
void processButtons(void);
void processRxInput(void);
void setup_lsm9ds1(void);
void read_lsm9ds1(void);
void setup_drv2605(void);
void setup_oled(void);
void readMic(void);
///call this function again and again, using Ticker
void refreshOLED(void);
void refreshSubscribe(void);
//void publishing(String devState, String actState = "NULL");
///timer for adc sample
Ticker tmrMic(readMic, 120, 0, MICROS_MICROS);//125
//Ticker tmrMic(readMic, 128, 0, MICROS_MICROS);



//**************************FUNCTION DEFINITIONS*****************************

/**************************************************************************/
/*!
    @brief  publish a message with device and its state
    @param devState the first part of the message
    @param actState the second part of the message
    @returns void
*/
/**************************************************************************/
//sending a message to MQTT
void publishing(String devState, String actState = "NULL")
{
StaticJsonBuffer<300> JSONbuffer;
JsonObject& JSONencoder = JSONbuffer.createObject();
char JSONmessageBuffer[50];
  JSONencoder["device"] = devState;
  if (actState!= "NULL")
    JSONencoder["cmd"] = actState;
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  //Serial.println("Sending message to MQTT topic..");
  Serial.println(JSONmessageBuffer);
 
  if (client.publish(PUBLISH_CH, JSONmessageBuffer) == true) 
  {
    Serial.println("Success sending message");
  } else 
  {
    Serial.println("Error sending message");
  }
}

/**************************************************************************/
/*!
    @brief  Setting up the LSM9DS1 Accelerometer
    @returns void
*/
/**************************************************************************/
void setup_lsm9ds1(void)
{
	// Try to initialise and warn if we couldn't detect the chip
	if (!lsm.begin())
	{
	  Serial.println("Oops ... unable to initialize the LSM9DS1. Check your wiring!");
	  //while (1);
	}
	Serial.println("Found LSM9DS1...");
	lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_16G);
	lsm.setupMag(lsm.LSM9DS1_MAGGAIN_16GAUSS);
	lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_2000DPS);
	// zero-fill all the arrays:
	for (int axis = 0; axis < numAxis; axis++) {
	  readIndex[axis] = 0;
	  total[axis] = 0;
	  average[axis] = 0;
	  for (int i = 0; i<numReadings; i++){
		  readings[axis][i] = 0;
	  }
	}
}
/**************************************************************************/
/*!
    @brief  Read the LSM9DS1 and detect the Right, Left, Away and Close Movements
    @returns void
*/
/**************************************************************************/
void read_lsm9ds1(void)
{
  lsm.read();  // ask it to read in the data 
  sensors_event_t a, m, g, temp;
  lsm.getEvent(&a, &m, &g, &temp); 
  axm = a.acceleration.x;
  aym = a.acceleration.y;
  azm = a.acceleration.z;
  smooth(0, axm);
  smooth(1, aym);
  smooth(2, azm);

  cntms++;
  
  switch(gState)
  {
    case 0:
        if(average[0]<-3)
        {
          curDirection = 1;//left
          //Serial.println("One");
        }
        if(average[0]>1.5)
        {
          curDirection = 2;//right
          //Serial.println("Two");
        }
        if(average[2]<-3)
        {
          curDirection = 3;//away
          //Serial.println("Three");
        }
        if(average[2]>3)
        {
          curDirection = 4;//closer
          //Serial.println("Four");
        }
        if (curDirection>0) gState = 1;
        break;
    case 1:
      if(cntms==20)
      {
        curDirection = 0;
        gState = 0;
        cntms = 0;
        //Serial.println("none");
      }
      else
      {
        switch(curDirection)
        {
          case 1:
            if(average[0]>1) 
            {
              #ifndef RELEASE
                Serial.println("Acc.down");
              #endif
              buzzerMode = 2;oldBuzzerMode=true;
              optIndex = 6;
              moveEvent = true;
              curDirection = 0; 
              gState = 0; 
              cntms = 0;
            } 
            break;
          case 2: 
            if(average[0]<-2)  
            {
              #ifndef RELEASE
                Serial.println("Acc.up");
              #endif
              buzzerMode = 2;oldBuzzerMode=true;
              optIndex = 5;
              moveEvent = true;
              curDirection = 0; 
              gState = 0; 
              cntms = 0;
            } 
            break;
          case 3: 
            if(average[2]>2)  
            {
              #ifndef RELEASE
                Serial.println("Acc.left");
              #endif
              buzzerMode = 2;oldBuzzerMode=true;
              optIndex = 8;
              moveEvent = true;
              curDirection = 0; 
              gState = 0; 
              cntms = 0;
            } 
            break;
          case 4: 
            if(average[2]<-2) 
            {
              #ifndef RELEASE
                Serial.println("Acc.right");
              #endif
              buzzerMode = 2;oldBuzzerMode=true;
              optIndex = 7;
              moveEvent = true;
              curDirection = 0; 
              gState = 0; 
              cntms = 0;
            } 
            break;
        }//switch(curDirection)
      }//else
      break;
  }//switch(gState)
}
/**************************************************************************/
/*!
    @brief  Setting up the DRV2605 Motor
    @returns void
*/
/**************************************************************************/
void setup_drv2605(void)
{
	if (!drv.begin())
	{
	  Serial.println("Oops ... unable to initialize the DRV2605. Check your wiring!");
	}
	Serial.println("Found DRV2605...");
	// I2C trigger by sending 'go' command 
	drv.setMode(DRV2605_MODE_INTTRIG); // default, internal trigger when sending GO command

	drv.selectLibrary(1);
}
/**************************************************************************/
/*!
    @brief  Setting up the OLED Display
    @returns void
*/
/**************************************************************************/
void setup_oled(void)
{
    // Initialising the UI will init the display too.
  
  oled.init();
  //oled.flipScreenVertically();
  oled.setFont(ArialMT_Plain_10);
  //Testing icons and texts
  oled.clear();
  oled.drawXbm(0, 0, 128, 64, kit);
  oled.display();
  delay(2000);
  oled.clear();
  oled.drawXbm(0, 0, 128, 64, teco);
  oled.display();
  delay(2000);
  oled.clear();
  oled.drawXbm(0, 0, 128, 64, swc);
  oled.display();
  delay(2000);

  
  //Used to empty the buffer, but after oled.end(), you have to reinitialize the oled.
  //oled.resetDisplay();
  //oled.end();
  //oled.~OLEDDisplay();
  // Initialising the UI will init the display too.
  //oled.init();
  //oled.flipScreenVertically();
  //oled.setFont(ArialMT_Plain_10);
  //oled.clear();
  
}
/**************************************************************************/
/*!
    @brief  process serial data from Nano
    @returns void
*/
/**************************************************************************/
void processRxInput(void)
{
  switch(rxData)
  {
    case 1://JoyStick Middle
      #ifndef RELEASE
        Serial.println("Jstick.Mid");
      #endif
      buzzerMode = 3;oldBuzzerMode=true;
      break;
    case 2://JoyStick Right
      optIndex = 3;
      moveEvent = true;
      #ifndef RELEASE
        Serial.println("Jstick.Right");
      #endif
      buzzerMode = 3;oldBuzzerMode=true;
      break;
    case 3://JoyStick Left
      optIndex = 4;
      moveEvent = true;
      #ifndef RELEASE
        Serial.println("Jstick.Left");
      #endif
      buzzerMode = 3;oldBuzzerMode=true;
      break;
    case 4://JoyStick Down
      optIndex = 2;
      moveEvent = true;
      #ifndef RELEASE
        Serial.println("Jstick.Down");
      #endif
      buzzerMode = 3;oldBuzzerMode=true;
      break;
    case 5://JoyStick Up
      optIndex = 1;
      moveEvent = true;
      #ifndef RELEASE
        Serial.println("Jstick.Up");
      #endif
      buzzerMode = 3;oldBuzzerMode=true;
      break;
    default:
      break;
  }

}
/**************************************************************************/
/*!
    @brief  Detects the Interrupt From Nano to start serial data
    @returns void
*/
/**************************************************************************/
void ICACHE_RAM_ATTR SerialTriggerISR() 
{
  rxflag=true;  // rxState = 0;
  //#ifndef RELEASE
  //  Serial.println("T");
  //#endif

}
/**************************************************************************/
/*!
    @brief  Routine to Handle the Push Button Presses and Holds
    @returns void
*/
/**************************************************************************/
void processButtons(void)
{
     switch(btnState)
     {
     case 0://idle
      break;
     //==================================== CONTEXT BUTTON PRESSED, RELEASED, HOLDING, HOLD_END STATES ================================================
     case 1://contextKeyPressed
          btnState = 0;
          timeContextKeyPress = millis();
          if ( timeContextKeyPress - timeContextLastPress >= DEBOUNCE_TIME)
          {
              switch(swContextState){
              case 0:
				//================== KeyPressed is detected. =================================
                Serial.println("C.Pressed");// KeyPressed is detected.
                btnState = 2;
                swContextState = 1;
                break;
              case 1:
                if(contextHolding)
                {
					//================== Key Holding ends here =================================
                  Serial.println("C.HoldEnd");//Key Holding ends here
                  //Perform the following while contect hold is released
                  if(wState == 1)
                  {
                    if(moveEvent)
                    {
                      moveEvent = false;
                      publishing(device[optIndex-1]);
                      #ifndef RELEASE
                      Serial.println("publishing device");
                      #endif
                      mapDevice[0]=device[optIndex-1];
                      displayMode = 2;
                      vibrationMode = 2;oldVibrationMode =true;
                      wState = 4;
                    }
                  }
                }else
                {
				  //================== Key un-Pressed is detected. =================================
                  Serial.println("C.Released");  // Key un-Pressed is detected.
                  //Perform the following function
                  //contextPress = true;
                  if(wState==1)
                  {
                    publishing("pointing");
                    #ifndef RELEASE
                    Serial.println("publishing pointing");
                    #endif
                    vibrationMode = 2;oldVibrationMode =true;
                    wState = 2;
                  }else if((wState==3) || (wState==5))// 6)
                  {
                    #ifndef RELEASE
                    Serial.println("publishing none");
                    #endif
                    publishing("none");
                    vibrationMode = 0;oldVibrationMode =true;
                    
                    wState = 0;
                  }
                }
                contextHolding = false;
                btnState = 0;
                swContextState = 0;
                break;
              }//switch                
          }//if time
          timeContextLastPress = timeContextKeyPress;
      break;
     case 2:
       if ( millis() - timeContextLastPress >= HOLDSTART_TIME)
       {
		  //================== Key Holding starts here =================================
          Serial.println("C.Hold");//Key Holding starts here
          contextHolding = true;
          btnState = 0;
       }
      break;
     //==================================== VOICE BUTTON PRESSED, RELEASED, HOLDING, HOLD_END STATES ================================================
     case 3://voiceKeyPressed
          btnState = 0;
          timeVoiceKeyPress = millis();
          if ( timeVoiceKeyPress - timeVoiceLastPress >= DEBOUNCE_TIME)
          {
              switch(swVoiceState){
              case 0:
				//================== KeyPressed is detected. =================================
                Serial.println("V.Pressed");// KeyPressed is detected.
                
                btnState = 4;
                swVoiceState = 1;
                break;
              case 1:
                if(voiceHolding)
                {
				  //================== Key Holding ends here =================================
                  Serial.println("V.HoldEnd");//Key Holding ends here
                  //Disable recoding the Mic
                  //timer1_disable();
                  tmrMic.stop();
                  recDone = true;
                  vibrationMode = 2;oldBuzzerMode=true;
                  buzzerMode = 3;oldVibrationMode=true;
                }else
                {
				  //================== Key un-Pressed is detected. =================================
                  Serial.println("V.Released");  // Key un-Pressed is detected.

                }
                voiceHolding = false;
                btnState = 0;
                swVoiceState = 0;
                break;
              case 2:
                break;
              }//switch                
          }//if time
          timeVoiceLastPress = timeVoiceKeyPress;
      break;
     case 4:
       if ( millis() - timeVoiceLastPress >= HOLDSTART_TIME)
       {
          //================== Key Holding starts here =================================
          //Perform the following action
          
          recording = true;
          //timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);//80MHz/16=5MHz
          //timer1_write(625); //125us=.2us x 625
          //tmrMic.start();
          vibrationMode = 2;oldVibrationMode=true;
          buzzerMode = 2;oldBuzzerMode=true;
          Serial.println("V.Hold");//Key Holding starts here
          voiceHolding = true;
          btnState = 0;
       }
      break;
     //==================================== ACTION BUTTON PRESSED, RELEASED, HOLDING, HOLD_END STATES ================================================
     case 5://actionKeyPressed
          btnState = 0;
          timeActionKeyPress = millis();
          if ( timeActionKeyPress - timeActionLastPress >= DEBOUNCE_TIME)
          {
              switch(swActionState){
              case 0:
                //================== KeyPressed is detected. =================================
				        Serial.println("A.Pressed");// KeyPressed is detected.
                btnState = 6;
                swActionState = 1;
                break;
              case 1:
                if(actionHolding)
                {
                  //================== Key Holding ends here =================================
				          Serial.println("A.HoldEnd");//Key Holding ends here
                  //perform the following while action hold is released
                  if((wState==3) || (wState==5))
                  {
                    if(moveEvent)
                    {
                      moveEvent = false;
                      publishing(mapDevice[0],actions[optIndex-1]);
                      #ifndef RELEASE
                      Serial.println("publishing action");
                      #endif
                      mapAction[0]=actions[optIndex-1];

                      displayMode = 4;
                      wState = 6;
                    }
                  }
                  //tmrAction.stop();
                }else
                {
                  //================== Key un-Pressed is detected. =================================
				          Serial.println("A.Released");  // Key un-Pressed is detected.
                  
                }
                actionHolding = false;
                btnState = 0;
                swActionState = 0;
                break;
              case 2:
                break;
              }//switch                
          }//if time
          timeActionLastPress = timeActionKeyPress;
      break;
     case 6:
       if ( millis() - timeActionLastPress >= HOLDSTART_TIME)
       {
          //================== Key Holding starts here =================================
		      Serial.println("A.Hold");//Key Holding starts here
          //tmrAction.start();
          pre4000ms = millis();
          actionHolding = true;
          anotherAction = true;
          btnState = 0;
       }
      break;
    ///////////////////////////////////////////////////////////////////////////////
    default:
      break;
}//switch
}

/**************************************************************************/
/*!
    @brief  //Handle Push Button Interrupt for Context
    @returns void
*/
/**************************************************************************/
void ICACHE_RAM_ATTR ContextKeyIsPressed()
{
   btnState = 1;
}
/**************************************************************************/
/*!
    @brief  //Handle Push Button Interrupt for Voice
    @returns void
*/
/**************************************************************************/
void ICACHE_RAM_ATTR VoiceKeyIsPressed()
{
   btnState = 3;
}
/**************************************************************************/
/*!
    @brief  Handle Push Button Interrupt for Action
    @returns void
*/
/**************************************************************************/
void ICACHE_RAM_ATTR ActionKeyIsPressed()
{
   btnState = 5;
}

/*
void ICACHE_RAM_ATTR readMic()
{
  timer1_write(625); //125us=.2us x 625
  aud[cnt125us++] = map(analogRead(A0),0,1023,0,255);
  if(cnt125us==(1024*40))
  {
    //cnt125us=0;
    timer1_disable();
    //recDone = true;
  }
}*/


/**************************************************************************/
/*!
    @brief  resend the subscrib message to MQTT again and again, using Ticker
    @returns void
*/
/**************************************************************************/
void refreshSubscribe()
{
  publishing(mapDevice[0],actions[optIndex-1]);
  #ifndef RELEASE
  Serial.println("publishing action");
  #endif
  mapAction[0]=actions[optIndex-1];
  displayMode = 4;
  wState = 6;
}

/**************************************************************************/
/*!
    @brief  call this function again and again, using Ticker
    @returns void
*/
/**************************************************************************/
void refreshOLED1()
{
  //digitalWrite(RGBG, !(digitalRead(RGBG)));  //Invert Current State of LED  
}
/**************************************************************************/
/*!
    @brief  Callback function is called when there is MQTT message
    @param topic  topic where mqtt message is comping from
    @param payload  the actual message
    @param length   the message length
    @returns void
*/
/**************************************************************************/
void callback(char* topic, byte *payload, unsigned int length) 
{
  char tt;
  //print recevied messages on the serial console
  #ifndef RELEASE
  Serial.println("-------new message from broker-----");
  Serial.print("channel:");
  Serial.println(topic);
  Serial.print("data:");  
  Serial.write(payload, length);
  Serial.println();
  #endif
  switch(wState)
  {
    case 0:
      parsing(payload);
      for (tt = 0; tt < 8; tt++) //Iterate through results
      {
        device[tt] = arr[tt];
          #ifndef RELEASE
          Serial.print(tt,DEC);Serial.print(" ");Serial.println(arr[tt]);
          #endif
      }
      mapDevice[0]=arr[8];
      if(mapDevice[0] == "none")
      {
          //Serial.println(" found");
          displayMode = 1;
          vibrationMode = 0;oldVibrationMode =true;
          buzzerMode = 0;oldBuzzerMode =true;
          wState = 1;
      }
      break;
    case 2:
      parsing(payload);
      for (tt = 0; tt < 8; tt++) //Iterate through results
      {
        actions[tt] = arr[tt];
        #ifndef RELEASE
        Serial.print(tt,DEC);Serial.print(" ");Serial.println(arr[tt]);
        #endif
      }
      mapDevice[0]=arr[8];
      if(mapDevice[0] == "none")
      {
          //Serial.println("none found");
          displayMode = 1;
          vibrationMode = 1;oldVibrationMode =true;
          wState = 0;
      }else
      {
          //Serial.println("actions found");
          displayMode = 3;
          vibrationMode = 3;oldVibrationMode =true;
          wState = 3;
      }
      //publishing("light", "up");
      break;
    case 4:
      parsing(payload);
      for (tt = 0; tt < 8; tt++) //Iterate through results
      {
        actions[tt] = arr[tt];
        #ifndef RELEASE
        Serial.print(tt,DEC);Serial.print(" ");Serial.println(arr[tt]);
        #endif
      }
      mapDevice[0]=arr[8];
      if(mapDevice[0] == "none")
      {
          //Serial.println("none found");
          displayMode = 1;
          wState = 0;
      }else
      {
          //Serial.println("actions found");
          displayMode = 3;
          vibrationMode = 3;oldVibrationMode =true;
          wState = 5;
      }
      break;
  }

}

/**************************************************************************/
/*!
    @brief  Implement display modes 1,2,3,4 on OLED
    @param om   The modes number
    @returns void
*/
/**************************************************************************/
void setDisplayMode(unsigned char om)
{

  switch(om)
  {
    case 1:
      oled.clear();
      oled.setColor(WHITE);
      oled.drawXbm(5, 13, 12, 12, fup);
      oled.drawXbm(5, 26, 12, 12, fdown);
      oled.drawXbm(5, 39, 12, 12, fleft);
      oled.drawXbm(5, 52, 12, 12, fright);
      oled.setTextAlignment(TEXT_ALIGN_LEFT);
      oled.setFont(ArialMT_Plain_10);
          
      oled.fillRect(21, 0, 96, 12);
      oled.setColor(WHITE);
      oled.drawString(21, 13, device[0]);
      oled.drawString(21, 26, device[1]);
      oled.drawString(21, 39, device[2]);
      oled.drawString(21, 52, device[3]);
    
      oled.setColor(BLACK);
      oled.drawString(21, 0, "Kurzbefehle");
    
      oled.display();
      break;
    case 6:
      oled.clear();
      oled.setColor(WHITE);
      oled.drawXbm(5, 13, 12, 12, up);
      oled.drawXbm(5, 26, 12, 12, down);
      oled.drawXbm(5, 39, 12, 12, left);
      oled.drawXbm(5, 52, 12, 12, right);
      
      oled.setTextAlignment(TEXT_ALIGN_LEFT);
      oled.setFont(ArialMT_Plain_10);
          
      oled.fillRect(21, 0, 96, 12);
      oled.setColor(WHITE);
      oled.drawString(21, 13, device[4]);
      oled.drawString(21, 26, device[5]);
      oled.drawString(21, 39, device[6]);
      oled.drawString(21, 52, device[7]);
    
      oled.setColor(BLACK);
      oled.drawString(21, 0, "Kurzbefehle");
    
      oled.display();
      break;
    case 2:
    case 7:
      oled.clear();
      oled.setTextAlignment(TEXT_ALIGN_LEFT);
      oled.setFont(ArialMT_Plain_16);
      oled.setColor(WHITE);
      oled.drawString(21, 8, "Vorbind mit");
      oled.drawString(21, 32, mapDevice[0]);
      oled.display();
      break;
    case 3:
      oled.clear();
      oled.setColor(WHITE);
      oled.drawXbm(5, 13, 12, 12, fup);
      oled.drawXbm(5, 26, 12, 12, fdown);
      oled.drawXbm(5, 39, 12, 12, fleft);
      oled.drawXbm(5, 52, 12, 12, fright);
      
      oled.setTextAlignment(TEXT_ALIGN_LEFT);
      oled.setFont(ArialMT_Plain_10);
          
      oled.fillRect(21, 0, 96, 12);
      oled.setColor(WHITE);
      oled.drawString(21, 13, actions[0]);
      oled.drawString(21, 26, actions[1]);
      oled.drawString(21, 39, actions[2]);
      oled.drawString(21, 52, actions[3]);
    
      oled.setColor(BLACK);
      oled.drawString(21, 0, mapDevice[0]);
    
      oled.display();
      
      break;
    case 8:
      oled.clear();
      oled.setColor(WHITE);
      oled.drawXbm(5, 13, 12, 12, up);
      oled.drawXbm(5, 26, 12, 12, down);
      oled.drawXbm(5, 39, 12, 12, left);
      oled.drawXbm(5, 52, 12, 12, right);
      
      oled.setTextAlignment(TEXT_ALIGN_LEFT);
      oled.setFont(ArialMT_Plain_10);
          
      oled.fillRect(21, 0, 96, 12);
      oled.setColor(WHITE);
      oled.drawString(21, 13, actions[4]);
      oled.drawString(21, 26, actions[5]);
      oled.drawString(21, 39, actions[6]);
      oled.drawString(21, 52, actions[7]);
    
      oled.setColor(BLACK);
      oled.drawString(21, 0, mapDevice[0]);
    
      oled.display();
      break;
    case 4:
    case 9:
      oled.clear();
      oled.setTextAlignment(TEXT_ALIGN_LEFT);
      oled.setFont(ArialMT_Plain_16);
      oled.setColor(WHITE);
      oled.drawString(21, 8, "Befehl");
      oled.drawString(21, 32, mapAction[0]);
      oled.display();
      cntDispScreenTime++;
      break;
    default:
      break;
  }
}
/**************************************************************************/
/*!
    @brief  setup the Wifi
    @returns void
*/
/**************************************************************************/
void setup_wifi() 
{
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");

    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) 
    {
      delay(500);
      Serial.print(".");
      yield();
    }
    sendCmd2Slave(2);//setRGBColor(2);
    randomSeed(micros());
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    delay(500);//for led
    
}

/**************************************************************************/
/*!
    @brief  Connect/reconnect to the Wifi
    @returns void
*/
/**************************************************************************/
void reconnect() 
{
// Loop until we're reconnected
while (!client.connected()) 
{
  Serial.print("Attempting MQTT connection...");
  // Create a random client ID
  String clientId = "ESPClient-";
  clientId += String(random(0xffff), HEX);
  // Attempt to connect
  //if (client.connect(clientId.c_str(),MQTT_USER,MQTT_PASSWORD)) 
  if (client.connect(clientId.c_str())) 
  {
    Serial.println("connected");
    //Once connected, publish an announcement...

    publishing("none");
    // ... and resubscribe
    client.subscribe(SUBSCRIBE_CH);
    //Serial.println("debug1");
  } else 
  {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
    // Wait 5 seconds before retrying
    delay(500);
  }
  yield();
  //Serial.println("debug2");
}
}

/**************************************************************************/
/*!
    @brief  Parsing the payload message recevied in callback function
    @param payload   The mqtt payload message
    @returns void
*/
/**************************************************************************/
void parsing( byte *payload)
{
  
  int i;
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& message = jsonBuffer.parseObject((char *)payload);
  if (!message.success()) 
  {
    #ifndef RELEASE
    Serial.println("JSON parse failed");  
    #endif
    return;
  }
  String device1 = message["device"]; //Get sensor type value
  arr[8] = device1;

  for ( i = 0; i < 8; i++) 
  { //Iterate through results
    String mappingValue = message["mapping"][i];  //Implicit cast
    arr[i] = mappingValue;
  }
}
/**************************************************************************/
/*!
    @brief  Send Serial command to Nano (Slave)
    @param cmd   The modes number
    @returns void
*/
/**************************************************************************/
void sendCmd2Slave(unsigned char cmd)
{
    //delay(1);
    delayMicroseconds(100);
    digitalWrite(interruptPin,LOW); 
    //delay(1);
    delayMicroseconds(100);
    digitalWrite(interruptPin,HIGH);
    s.write(cmd);


}

/**************************************************************************/
/*!
    @brief  Implement buzzer modes 1,2,3,4 on beeper
    @param bm   The modes number
    @returns void
*/
/**************************************************************************/
void setBuzzerMode(unsigned char bm)
{
  switch(bm)
  {
    case 0:
      tone(buzzerPin, frequency('a'), 3*113);
      delay(250);
      tone(buzzerPin, frequency('d'), 113);
      break;
    case 1:
      tone(buzzerPin, frequency('a'), 1*113);
      delay(250);
      tone(buzzerPin, frequency('d'), 3*113);
      break;
    case 2:
      tone(buzzerPin, frequency('a'), 3*113);
      break;
   case 3:
      tone(buzzerPin, frequency('d'), 3*113);
      break;
   case 4:
      tone(buzzerPin, frequency('e'), 3*113);
      break;
  }
  
}
/**************************************************************************/
/*!
    @brief  Implement vibration modes 1,2,3,4 on DRV2605
    @param vm   The modes number
    @returns void
*/
/**************************************************************************/
void setVibrationMode(unsigned char vm)
{
  unsigned char temp;
  switch(vm)
  {
    case 0:
      for(temp=0;temp<50;temp++)
      {
        drv.setWaveform(0, 37);
        drv.setWaveform(1, 0);
        drv.go();
        delay(50);
      }
      
      delay(1500);
    
      for(temp=0;temp<50;temp++)
      {
        drv.setWaveform(0, 37);
        drv.setWaveform(1, 0);
        drv.go();
        delay(10);
      }
      break;
    case 1:
      for(temp=0;temp<50;temp++)
      {
        drv.setWaveform(0, 37);
        drv.setWaveform(1, 0);
        drv.go();
        delay(50);
      }
      
      delay(1500);
    
      for(temp=0;temp<50;temp++)
      {
        drv.setWaveform(0, 37);
        drv.setWaveform(1, 0);
        drv.go();
        delay(50);
      }
      break;
    case 2:
      for(temp=0;temp<50;temp++)
      {
        drv.setWaveform(0, 37);
        drv.setWaveform(1, 0);
        drv.go();
        delay(10);
      }
      break;
   case 3:
      for(temp=0;temp<50;temp++)
      {
        drv.setWaveform(0, 37);
        drv.setWaveform(1, 0);
        drv.go();
        delay(50);
      }
      break;
   case 4:

      break;
  }
  
}
/**************************************************************************/
/*!
    @brief  Average the LSM9DS1 readings
    @param axis   The axis number 1,2,3
    @param val   The axis acceleration value
    @returns void
*/
/**************************************************************************/
void smooth(int axis, float val) 
{
    // pop and subtract the last reading:
    total[axis] -= readings[axis][readIndex[axis]];
    total[axis] += val;

    // add value to running total
    readings[axis][readIndex[axis]] = val;
    readIndex[axis]++;

    if(readIndex[axis] >= numReadings)
        readIndex[axis] = 0;

    // calculate the average:
    average[axis] = total[axis] / numReadings;
}
/**************************************************************************/
/*!
    @brief  Generate the audio tone for beeper
    @param note   play the note
    @returns void
*/
/**************************************************************************/
int frequency(char note) 
{
  // This function takes a note character (a-g), and returns the
  // corresponding frequency in Hz for the tone() function.

  int i;
  const int numNotes = 8;  // number of notes we're storing

  // The following arrays hold the note characters and their
  // corresponding frequencies. The last "C" note is uppercase
  // to separate it from the first lowercase "c". If you want to
  // add more notes, you'll need to use unique characters.

  // For the "char" (character) type, we put single characters
  // in single quotes.

  char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
  int frequencies[] = {262, 294, 330, 349, 392, 440, 494, 523};

  // Now we'll search through the letters in the array, and if
  // we find it, we'll return the frequency for that note.

  for (i = 0; i < numNotes; i++)  // Step through the notes
  {
    if (names[i] == note)         // Is this the one?
    {
      return(frequencies[i]);     // Yes! Return the frequency
    }
  }
  return(0);  // We looked through everything and didn't find it,
              // but we still need to return a value, so return 0.
}
