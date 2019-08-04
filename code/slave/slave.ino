/*!
 *  @file slave.ino
 *  @brief arduino slave file
 *  
 *  This is the main for slave Nano controller, which handles the request/data from
 *  - ESP8266
 *  - Joystick
 *  and perform output actions to:
 *  - RGB LED
 * 
 *  @author Sajjad Hussain
 *
 *  @date 02.08.2019
 */

//*****************INCLUDE FILES*****************
#include <AltSoftSerial.h>


//*****************CONSTANTS/DEFINES*****************
/** Joystick is in Middle */
#define   JstickMid   1   
/** Joystick is in Right */
#define   JstickRight 2  
/** Joystick is in Left */
#define   JstickLeft  3      
/** Joystick is in Down */
#define   JstickDown  4      
/** Joystick is in Up */
#define   JstickUp    5     

//*****************HARDWARE PINS*****************

/** LED General Purpose */
const int L1 = 13;
/** RGB BLUE PIN  */
const int RGBB = 12; 
/** RGB GREEN PIN  */
const int RGBG = 11;  
/** RGB RED PIN  */
const int RGBR = 10;  
/** Trigger Interrupt to Master  */
const int trig2Master = 6;  


//*****************FUNCTION PROTOTYPES*****************

void sendCmd2Master(unsigned char cmd);
void setRGBColor(unsigned char color);

//*****************GLOBAL VARIABLES*****************
/** Count 4ms Timer 2 triggers */
unsigned char cnt4ms;             
/** Count 40ms Timer 2 triggers */
unsigned char cnt40ms;             
/** Serial Port */
AltSoftSerial s; 
/** Joystick Middle position */
boolean inMiddle;
/** Counter to count how much time it is in Middle Position */
unsigned char cntMiddle;
/** Serial Port State */
unsigned char rxState;  
/** Serial Port input character */
unsigned char inChar;   

//*****************FUNCTION DEFINITIONS*****************

/**************************************************************************/
/*!
    @brief  initialization of Arduino Nano
    @returns void
*/
/**************************************************************************/
void setup() 
{
  //delay(1000);

  pinMode(L1, OUTPUT);
  pinMode(RGBR, OUTPUT);
  pinMode(RGBG, OUTPUT);
  pinMode(RGBB, OUTPUT);
  
  digitalWrite(L1,LOW);
  digitalWrite(RGBR,HIGH); 
  digitalWrite(RGBG,LOW); 
  digitalWrite(RGBB,HIGH); 
  delay(500);
  digitalWrite(RGBR,LOW);  
  digitalWrite(RGBB,LOW);   

  Serial.begin(115200);
  //while (!Serial) {
  //  delay(1); // will pause Zero, Leonardo, etc until serial console opens
  //}
  for (char l=0;l<8;l++)
  {
    setRGBColor(l);
    delay(1000);
  }
  Serial.println("Hello IntEnseChol!");
  //Setup Serial
  s.begin(9600);
  pinMode(trig2Master, OUTPUT);  
  digitalWrite(trig2Master,HIGH);
  //Timer2 Settup
  TIMSK2 = (TIMSK2 & B11111001) | 0x06;
  TCCR2B = (TCCR2B & B11111000) | 0x06;//Divider=256
  OCR2A = 249;
  OCR2B = 249;
  
}
/**************************************************************************/
/*!
    @brief  Main loop Arduino Nano
    @returns void
*/
/**************************************************************************/
void loop()
{
  //Recieve Serial Data from ESP
  if (s.available()>0)
  {
    inChar=s.read();
    Serial.print("A");Serial.println(inChar);
    setRGBColor(inChar);
  }

}//loop
 
/**************************************************************************/
/*!
    @brief  ISR for timer2 to trigger after 4ms
    @returns void
*/
/**************************************************************************/
ISR(TIMER2_COMPA_vect)
{
  cnt4ms++;
  if((cnt4ms%4)==0)//16ms
  {
    int x = analogRead(A0);
    int y = analogRead(A1);
    //X
    if ((x<384) && (y<640) && (y>384) && (inMiddle))
    {
      Serial.print("Right\n");
      sendCmd2Master(JstickRight);
      inMiddle = false;
    }
    else if ((x>640) && (y<640) && (y>384)&& (inMiddle) )
    {
      Serial.print("Left\n");
      sendCmd2Master(JstickLeft);
      inMiddle = false;
    }
   //Y
    else   if ((y<384) && (x<640) && (x>384)&& (inMiddle))
    {
      Serial.print("Down\n");
      sendCmd2Master(JstickDown);
      inMiddle = false;
    }
    else if ((y>640) && (x<640) && (x>384)&& (inMiddle))
    {
      Serial.print("Up\n");
      sendCmd2Master(JstickUp);
      inMiddle = false;
    }
    else if ( (y<576) && (y>448) && (x<576) && (x>448))
    {
      //Serial.print("Middle\n");
      //sendCmd2Master(JstickMid);
      cntMiddle++;
      if(cntMiddle==50)
      {
        cntMiddle = 0;
        inMiddle = true;
      }
    }
  }
  
  if((cnt4ms%25)==0)//100ms
  {
    int potentio = analogRead(A3);
    //int a  = potentio/WSENSITIVITY;
    //pSensitiyed = a*WSENSITIVITY;
    // print out the value you read:
    //Serial.println(pSensitiyed);
  }
  else if(cnt4ms==250)//1000ms
  {
   cnt4ms = 0;
   
  }
}
/**************************************************************************/
/*!
    @brief  ISR for timer2 to trigger after 40ms
    @returns void
*/
/**************************************************************************/
ISR(TIMER2_COMPB_vect){
  cnt40ms++;
  if(cnt40ms==250)
  {
   cnt40ms = 0;
   //state2 = ~state2;
   //digitalWrite(RED_PIN, state2);
  }
}
/**************************************************************************/
/*!
    @brief  Send a command to serial port
    @param cmd  a command to indicate different actions of joystick to ESP8266 (master)
    @returns void
*/
/**************************************************************************/
void sendCmd2Master(unsigned char cmd)
{
  delayMicroseconds(100);
  digitalWrite(trig2Master,LOW); 
  delayMicroseconds(100);
  digitalWrite(trig2Master,HIGH);
  s.write(cmd);
}
/**************************************************************************/
/*!
    @brief  Set a particular color of RGB LED
    @param color  the color number from 0-7 (RED, GREEN, BLUE etc.... WHITE).
    @returns void
*/
/**************************************************************************/
void setRGBColor(unsigned char color)
{
  switch(color)
  {
    case 0:
      ///off
      digitalWrite(RGBR, LOW);
      digitalWrite(RGBG, LOW);
      digitalWrite(RGBB, LOW);
      break;
    case 1:
      /// Red (turn just the red LED on):
      digitalWrite(RGBR, HIGH);
      digitalWrite(RGBG, LOW);
      digitalWrite(RGBB, LOW);
      break;
    case 2:
      /// Green (turn just the green LED on):
      digitalWrite(RGBR, LOW);
      digitalWrite(RGBG, HIGH);
      digitalWrite(RGBB, LOW);
      break;
    case 3:
      /// Blue (turn just the blue LED on):
      digitalWrite(RGBR, LOW);
      digitalWrite(RGBG, LOW);
      digitalWrite(RGBB, HIGH);
      break;
    case 4:
      /// Yellow (turn red and green on):
      digitalWrite(RGBR, HIGH);
      digitalWrite(RGBG, HIGH);
      digitalWrite(RGBB, LOW);
      break;
    case 5:
      /// Cyan (turn green and blue on):
      digitalWrite(RGBR, LOW);
      digitalWrite(RGBG, HIGH);
      digitalWrite(RGBB, HIGH);
      break;
    case 6:
      /// Purple (turn red and blue on):
      digitalWrite(RGBR, HIGH);
      digitalWrite(RGBG, LOW);
      digitalWrite(RGBB, HIGH);
      break;
   case 7:
    /// White (turn all the LEDs on):
    digitalWrite(RGBR, HIGH);
    digitalWrite(RGBG, HIGH);
    digitalWrite(RGBB, HIGH);
    break;
  }
}
