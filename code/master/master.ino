/*!
 *  @file master.ino
 *  @brief arduino master file
 *  
 *  This is the main for master ESP8266 controller, which handles the request/data from
 *  - Wifi
 *  - push buttons (3)
 *  - Joystick
 *  - Accelerometer
 *  and perform output actions to:
 *  - OLED display
 *  - Vibration Motor
 *  - RGB LED
 *  - Buzzer
 * 
 *  @author Sajjad Hussain
 *
 *  @date 02.08.2019
*/

#include "userdefined.h"


/**************************************************************************/
/*!
    @brief  initialization of Master
    @returns void
*/
/**************************************************************************/
void setup() {
  
  delay(1000);
  Serial.begin(115200);
  
  //while (!Serial) {
  //  delay(1); // will pause Zero, Leonardo, etc until serial console opens
  //}
  Serial.println("Hello IntEnseChol!");

  //Setting up LEDS
  sendCmd2Slave(1);//LED COLOR 1
  delay(500);//for led
  
  // Setting up Interrupts
  pinMode(swContext, INPUT); 
  pinMode(swVoice, INPUT);
  pinMode(swAction, INPUT);
  pinMode(buzzerPin, OUTPUT);

  //Setting up Serail
  s.begin(9600);
  //setup Lsm9ds1
  setup_lsm9ds1();

  //Setup Motor
  setup_drv2605();
  
  //setting wifi
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();
  //Testing Audio Transfer using UDP, comment the above one wifi setup and comment in the loop() as well
  /*WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
  */
  
  sendCmd2Slave(3);//setRGBColor(3);
  
  //enable trigger interrupt
  attachInterrupt(digitalPinToInterrupt(interruptPin), SerialTriggerISR, FALLING);
  attachInterrupt( digitalPinToInterrupt(swContext), ContextKeyIsPressed, CHANGE );
  attachInterrupt( digitalPinToInterrupt(swVoice), VoiceKeyIsPressed, CHANGE );
  attachInterrupt( digitalPinToInterrupt(swAction), ActionKeyIsPressed, CHANGE );
  
  //Setting up the OLED
  setup_oled();
  
  //timer1_attachInterrupt(readMic);

}
/**************************************************************************/
/*!
    @brief  reading the Microphone with timer function
    @returns void
*/
/**************************************************************************/
void readMic() 
{
  aud[cnt125us++] = map(analogRead(mic),0,1023,0,255);
  
  if (cnt125us == (1024*40)) 
  {
    tmrMic.stop();
  }
}
/**************************************************************************/
/*!
    @brief  Main loop of Master
    @returns void
*/
/**************************************************************************/
void loop() 
{
  
  yield();
  tmrMic.update();
  //Serial Data from Nano
  if (rxflag)
  {
    if(s.available())
    {
      rxflag = false;
      rxData = s.read();
      Serial.println(rxData);
      processRxInput();
    }
  }
  //Trying the reconnect Wifi
  if (!client.connected())
    reconnect();
  client.loop();

  //Recording Audio
  //yield();
  if(recording)
  {
    recording = false;
    Serial.println("Start Speaking");
    //delay(2000);
    //timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);//80MHz/16=5MHz
    //timer1_write(625); //125us=.2us x 625
    tmrMic.start();
    
  }
  if(recDone)
  {
    recDone = false;
    
    Serial.print("cnt125us ");Serial.println(cnt125us);
    /*
    Serial.println("Start Sending");
    //Testing using UDP
    for(pktCnt=0; pktCnt<40;pktCnt++)
    {
      Udp.beginPacket("192.168.0.164", 1234);//my room
      for(pktInx=0; pktInx<1024;pktInx++)
      {
        Udp.write(aud[1024*pktCnt+pktInx]);
        
      }
      Udp.endPacket();
     delay(10);
    }
    //for(pktCnt=0; pktCnt<40960;pktCnt++)
    //{
    //  Serial.print(pktCnt,DEC);Serial.print(" ");Serial.println(aud[pktCnt],DEC);
    //  yield(); 
    //}
    */
    //Testing using Wifi MQTT
    /*
    for(pktInx=0;pktInx<256;pktInx++)
      audMQTT[pktInx]= char('A'+(pktInx%26));//65+(i%26)
    audMQTT[256-1]= '\0';
    for(pktInx=0;pktInx<160;pktInx++)
    {
      audMQTT[0]=char((pktInx/100)+48);
      audMQTT[1]=char(((pktInx-(pktInx/100)*100)/10)+48);
      audMQTT[2]=char((pktInx%10)+48);
      client.publish("intensechoi/controller/audio",audMQTT);
    }*/
    
    //Real Audio MQTT
    for(pktCnt=0;pktCnt<(160*2);pktCnt++)
    {
      for(pktInx=0;pktInx<128;pktInx++)
        audMQTT[pktCnt*128+pktInx]= aud[pktCnt*128+pktInx];
      client.publish("intensechoi/controller/audio",audMQTT);
    }
    cnt125us=0;
    Serial.println("Sending done");

  }
  //Process Push BUttons, clicks and Holds
  processButtons(); 
  //Handle Buzzer Mode
  if(oldBuzzerMode!=buzzerMode)
  {
    setBuzzerMode(buzzerMode);
  }
  //Handle Vibration Mode
  if(oldVibrationMode!=vibrationMode)
  {
    setVibrationMode(vibrationMode);
  }
  oldBuzzerMode=buzzerMode;
  oldVibrationMode=vibrationMode;
  
  //Handle OLED Screens Mode
  unsigned long curMs = millis();
  if ((curMs - pre3000ms) >= 3000) 
  {
    pre3000ms = curMs;
    setDisplayMode(displayMode+5*secondScreen);
    secondScreen = !secondScreen;
  }
  
  //Handle Sending Actions while Holding Action Button
  if (((curMs - pre500ms) >= 500) && actionHolding) 
  {
    pre500ms = curMs;
    //Action Button is Holding
    Serial.println("Holding...");
    //subscribing(mapDevice[0],actions[optIndex-1]);
    //Serial.println("subscribing action");
    //mapAction[0]=actions[optIndex-1];
    //displayMode = 4;
    //wState = 6;
  }
  //Read LSM
  if ((curMs - pre25ms) >= 25) 
  {
    pre25ms = curMs;
    read_lsm9ds1();
  }

}
