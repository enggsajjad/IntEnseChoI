
@file master.ino
@mainpage INTELLIGENT ENVIRONMENT SECURES CHOICE OF INTERACTION (INTENSECHOI)
@section intro_sec Introduction
This the main page for the Haptic Controller which is the core of the project. It consists of two microcontrollers i.e. a master microcontroller and a slave microcontroller.
Master microcontroller is ESP8266 Sparkfun Thing Dev
----> https://www.sparkfun.com/products/13711
This controller interacts with the slave microcontroller i.e. Arduino Nano.
And, it provides the controller to interact with different object using its 
connected input sensors. It also provide notifications using different output
options.\n
Slave microcontroller is Arduino Nano 
----> https://store.arduino.cc/arduino-nano
This is used to extend the digital and analog pins of the master microcontroller. And these two are inter-connected using serial communication with a trigger input.

@section hw_sec Hardware
There are two boards used, one as a master and the other as a slave.\n
1. Sparkfun ESP8266 Thing Dev ----> https://www.sparkfun.com/products/13711
2. Arduino Nano ----> https://store.arduino.cc/arduino-nano
@subsection hw_sec1 Master Hardware
Board:   ESP8266 Thing Dev from Sparkfun\n
Peripherals:\n
1. Input: Thress Push Buttons Connected with three states Clicked, Hold and Released state\n
2. Input: Accelerometer LSM9DS1\n
3. Input: Joystick from Nano\n
4. Input: Microphone Max9814\n
5. Output: RGB LED to Nano\n
6. Output: 128x64 OLED Display SSD1306\n
7. Output: Active Buzzer\n
8. Output: Vibration Motor Driver DRV2605\n
9. Nano: Connected using Softerial Rx,Tx and a Trigger Pin

@subsection hw_sec2 Slave Hardware
Board:   Arduino Nano\n
Peripherals: \n
1. Input Sensor: Joystick connected to Analoge inputs\n
2. Ouput RGB LED:\n
3. ESP8266: Connected using Softerial Rx,Tx and a Trigger Pin\n

@subsection libraries Libraries
Following libraries are used during the project.
name=Adafruit DRV2605 Library                             , version=1.1.0\n
name=Adafruit GFX Library                                 , version=1.4.8\n
name=Adafruit LSM9DS1 Library                             , version=1.0.1\n
name=Adafruit MQTT Library                                , version=1.0.3\n
name=Adafruit NeoPixel                                    , version=1.1.8\n
name=Adafruit SSD1306                                     , version=1.2.9\n
name=Adafruit Unified Sensor                              , version=1.0.3\n
name=AltSoftSerial                                        , version=1.4\n
name=ArduinoJson                                          , version=5.13.5\n
name=ESP8266 and ESP32 Oled Driver for SSD1306 display    , version=4.0.0\n
name=NeoSWSerial                                          , version=3.0.5\n
name=PinChangeInterrupt                                   , version=1.2.6\n
name=PubSubClient                                         , version=2.7\n
name=Scheduler                                            , version=0.4.4\n
name=SparkFun Haptic Motor Driver                         , version=1.1.1 

@subsection testing Testing
Testing MQTT MEssages\n
MQTTBox: Test3\n
Topic to publish: intensechoi/controller/set\n
Topic to subscribe messages: intensechoi/controller/get\n
Topic to subscribe audio: intensechoi/controller/audio\n
Protocol: mqtt/tcp\n
Host: broker.mqtt-dashboard.com\n
Username: "Sajjad"\n
Password: "#sajjan"\n
Examples:\n
  {"device": "none", "mapping": ["light", "thermo", "fan","-", "window", "-","-", "tv"]}\n
  {"device": "light", "mapping": ["up", "down", "open","-", "close", "-","-", "dim"]}\n

@section author Author

Written by TECO.edu and Chair for Pervasive Computing Systems (pcs.tm.kit.edu)

@section license License

This code belongs to teco.edu and must be reffered when used. BSD license, all text here must be included in any redistribution.

 @date 02.08.2019