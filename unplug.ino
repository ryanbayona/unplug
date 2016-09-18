#include <SoftwareSerial.h>
SoftwareSerial mySerial(7, 8);

String msg = String("");
int SmsContentFlag = 0;
#define RELAY1 11

void setup()
{
  //setup 2 chanel relay
  Serial.begin(9600);
  pinMode(RELAY1, OUTPUT); 
  
  //turn on the modem  
  mySerial.begin(19200);               // the GPRS baud rate   
  Serial.begin(19200); 
  pinMode(9, OUTPUT);
  digitalWrite(9,LOW);
  delay(1000);
  digitalWrite(9,HIGH);
  delay(2000);
  digitalWrite(9,LOW);
  delay(3000);
  
  

  GprsTextModeSMS();
  

}
 
void loop()
{
   
   
    char SerialInByte;
    if(Serial.available())
    {
       mySerial.print((unsigned char)Serial.read());
    }  
    else if(mySerial.available())
    {
        char SerialInByte;
        SerialInByte = (unsigned char)mySerial.read();
        Serial.print( SerialInByte );
        if( SerialInByte == 13 )
        {
          ProcessGprsMsg();
         }
         if( SerialInByte == 10 ){
            // EN: Skip Line feed
         }
         else {
           // EN: store the current character in the message string buffer
           msg += String(SerialInByte);
         }
     }   
}
 
// EN: Make action based on the content of the SMS. 
// Notice than SMS content is the result of the processing of several GPRS shield messages.

void ProcessSms( String sms )
{
  Serial.print( "ProcessSms for [" );
  Serial.print( sms );
  Serial.println( "]" );
 
  if( sms.indexOf("tv on") >= 0 ){
    //controlling the relay based on sms received
    
    digitalWrite(RELAY1,0);           // Turns ON Relays 1
    //reply to sender
    mySerial.print("\r");
    delay(1000);                    //Wait for a second while the modem sends an "OK"
    mySerial.print("AT+CMGF=1\r");    //Because we want to send the SMS in text mode
    delay(1000);
  
    mySerial.print("AT+CMGS=\"+639155851517\"\r");    //Start accepting the text for the message
  
    delay(1000);
    mySerial.print("TV is ON\r");   //The text for the message
    delay(1000);
    mySerial.write(0x1A);  //Equivalent to sending Ctrl+Z 


    
    delay(2000);     
    return;
  }
  if( sms.indexOf("tv off") >= 0 ){
    digitalWrite(RELAY1,1);           // Turns OFF Relays 1
//reply to sender
    mySerial.print("\r");
    delay(1000);                    //Wait for a second while the modem sends an "OK"
    mySerial.print("AT+CMGF=1\r");    //Because we want to send the SMS in text mode
    delay(1000);
  
    mySerial.print("AT+CMGS=\"+639155851517\"\r");    //Start accepting the text for the message
  
    delay(1000);
    mySerial.print("TV is OFF\r");   //The text for the message
    delay(1000);
    mySerial.write(0x1A);  //Equivalent to sending Ctrl+Z 


    
    delay(2000);     
    
    return;
  }
}

// EN: Request Text Mode for SMS messaging

void GprsTextModeSMS(){
  mySerial.println( "AT+CMGF=1" );
}
 
void GprsReadSmsStore( String SmsStorePos ){
  // Serial.print( "GprsReadSmsStore for storePos " );
  // Serial.println( SmsStorePos ); 
  mySerial.print( "AT+CMGR=" );
  mySerial.println( SmsStorePos );
}
 
// EN: Clear the GPRS shield message buffer

void ClearGprsMsg(){
  msg = "";
}
 
// EN: interpret the GPRS shield message and act appropiately

void ProcessGprsMsg() 
{
    Serial.println("");
    Serial.print( "GPRS Message: [" );
    Serial.print( msg );
    Serial.println( "]" );
    if( msg.indexOf( "Call Ready" ) >= 0 )
   {
       Serial.println( "*** GPRS Shield registered on Mobile Network ***" );
       GprsTextModeSMS();
   }
 
  // EN: unsolicited message received when getting a SMS message
  // FR: Message non sollicité quand un SMS arrive
  if( msg.indexOf( "+CMTI" ) >= 0 )
  {
     Serial.println( "*** SMS Received ***" );
     // EN: Look for the coma in the full message (+CMTI: "SM",6)
     //     In the sample, the SMS is stored at position 6
     int iPos = msg.indexOf( "," );
     String SmsStorePos = msg.substring( iPos+1 );
     Serial.print( "SMS stored at " );
     Serial.println( SmsStorePos );
 
     // EN: Ask to read the SMS store
     GprsReadSmsStore( SmsStorePos );
  }
 
  // EN: SMS store readed through UART (result of GprsReadSmsStore request)  
  if( msg.indexOf( "+CMGR:" ) >= 0 )
  {
    // EN: Next message will contains the BODY of SMS
    SmsContentFlag = 1;
    // EN: Following lines are essentiel to not clear the flag!
    ClearGprsMsg();
    return;
  }
 
  // EN: +CMGR message just before indicate that the following GRPS Shield message 
  //     (this message) will contains the SMS body

  if( SmsContentFlag == 1 )
  {
    Serial.println( "*** SMS MESSAGE CONTENT ***" );
    Serial.println( msg );
    Serial.println( "*** END OF SMS MESSAGE ***" );
    ProcessSms( msg );
  }
 
  ClearGprsMsg();
  // EN: Always clear the flag
  SmsContentFlag = 0; 
}
//source
//http://www.elecrow.com/wiki/index.php?title=GPRS/GSM_Shield_v1.0#Light_Status
