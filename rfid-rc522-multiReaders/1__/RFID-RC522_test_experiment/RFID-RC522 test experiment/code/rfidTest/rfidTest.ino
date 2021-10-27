#include"rfid.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
LiquidCrystal_I2C lcd(0x27,16,2);
RFID rfid;
#define relayPin 8

uchar serNum[5];

void setup()
{
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  rfid.begin(7, 5, 4, 3, 6, 2);//rfid.begin(IRQ_PIN,SCK_PIN,MOSI_PIN,MISO_PIN,NSS_PIN,RST_PIN)
  delay(100);
  rfid.init();
    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin,HIGH);
  //Serial.begin(9600);
   lcd.setCursor(0,0);
  lcd.print("    Welcome!    ");
  delay(2000);
  
}
void loop()
{
  uchar status;
  uchar str[MAX_LEN];
  status = rfid.request(PICC_REQIDL, str);
  if (status != MI_OK)
  {
    return;
  }
  
  rfid.showCardType(str);
  status = rfid.anticoll(str);
  
  if (status == MI_OK)
  {
    //Serial.print("The card's number is: ");
    lcd.setCursor(0,0);
    lcd.print(" ID: ");
    memcpy(serNum, str, 5);
    rfid.showCardID(serNum);
   // Serial.println();
    
    // Check people associated with card ID
    uchar* id = serNum;
    if( id[0]==0x4B && id[1]==0xE6 && id[2]==0xD1 && id[3]==0x3B ) 
    {
     digitalWrite(relayPin,LOW);
      // Serial.println("Hello Dannel!");
      lcd.setCursor(0,1);
      lcd.print(" Hello Dannel! ");
      delay(2000);
      lcd.clear();
      digitalWrite(relayPin,HIGH);
    } 
    else if(id[0]==0x5A && id[1]==0xE4 && id[2]==0xC9 && id[3]==0x55) 
    {
      digitalWrite(relayPin,LOW);
      //Serial.println("Hello SunFounder");
      lcd.setCursor(0,1);
      lcd.print("Hello SunFounder");
      delay(2000);
      lcd.clear();
      digitalWrite(relayPin,HIGH);
    } 
    else
    {
      //Serial.println("Hello unkown guy!");
       lcd.setCursor(0,1);
      lcd.print("Hello unkown guy");
      delay(2000);
      lcd.clear();
    }
  }
  lcd.setCursor(0,0);
  lcd.print("    Welcome!    ");
   delay(2000);
  rfid.halt(); //command the card into sleep mode 
}
