
/*  PulseSensor™ Starter Project   http://www.pulsesensor.com
 *   
This an Arduino project. It's Best Way to Get Started with your PulseSensor™ & Arduino. 
-------------------------------------------------------------
1) This shows a live human Heartbeat Pulse. 
2) Live visualization in Arduino's Cool "Serial Plotter".
3) Blink an LED on each Heartbeat.
4) This is the direct Pulse Sensor's Signal.  
5) A great first-step in troubleshooting your circuit and connections. 
6) "Human-readable" code that is newbie friendly." 

*/
#include <ArduinoJson.h> 
#include <SPI.h>

#include <WiFi101.h>
#define PubNub_BASE_CLIENT WiFiClient
#include <PubNub.h>

static char ssid[] = "ocadu-embedded";      //SSID of the wireless network
static char pass[] = "internetofthings";    //password of that network
int status = WL_IDLE_STATUS;                // the Wifi radio's status

const static char pubkey[] = "pub-c-eec091f5-d78c-4758-a3ff-a10cc91e8dd4";  // Publishing Savayas Pubnub Account
const static char subkey[] = "sub-c-f11cada8-cefc-11e7-b83f-86d028961179";  //Suscribed to Savayas Pubnub Account

const static char pubChannel[] = "Emilia"; //choose a name for the channel to publish messages to // Emilia
const static char subChannel[] = "Savaya"; //choose a name for the channel to publish messages to // Savaya

unsigned long lastRefresh = 0;
int publishRate = 2000;


//  Pulse Pin Variables
int PulseSensorPurplePin = A0;        // Pulse Sensor PURPLE WIRE connected to ANALOG PIN 0
int LED13 = 13;   //  The on-board Arduion LED

// Motor Pin Variables 
int motorPin = 11;

// Pulse Pin DATA / Signal information                           
int Threshold;           // Savs pulse data that is being published
int Threshold2;           // Emilias pulse pin data (so where I have Threshold she should have Threshold2). 


// The SetUp Function:
void setup() {
 pinMode(LED13,OUTPUT); // pin that will blink to your heartbeat!
 pinMode(motorPin, OUTPUT); // motor that will go off to your heartbeat! 
 
  Serial.begin(11520);         // Set's up Serial Communication at certain speed. 
  pinMode(PulseSensorPurplePin, INPUT); // The data that is being sent to the output functions (LED and Motor) 

  connectToServer();
  
}

// The Main Loop Function
void loop() {

  Threshold2 = analogRead(PulseSensorPurplePin);    // Read the PulseSensor's value. (savs data being read). (Emilia should have Threshold)
                                                   // Assign this value to the "Signal" variable.

   Serial.println(Threshold2);                    // Send the Signal value to Serial Plotter. (savs data being read). 

   //int midVal = (topVal+lowerVal)/2;
   publishToPubNub();
   
   
   if(Threshold > 1000){                          // If the signal is above "550", then "turn-on" Arduino's on-Board LED.  //Emilia has this at Threshold 
     digitalWrite(LED13, HIGH); 
     digitalWrite(motorPin, HIGH);
   //  publishToPubNub();

      
   } else {
     digitalWrite(LED13,LOW); 
     digitalWrite(motorPin, LOW);    //  Else, the sigal must be below "550", so "turn-off" this LED.
    // publishToPubNub();


  delay(1000);
   }

 if(millis()-lastRefresh>=publishRate)     //theTimer to trigger the reads 
  {
  readFromPubNub();

    lastRefresh=millis();   
   
   }



  }
   
void connectToServer()
{
  WiFi.setPins(8,7,4,2); //This is specific to the feather M0
 
  status = WiFi.begin(ssid, pass);                    //attempt to connect to the network
  Serial.println("***Connecting to WiFi Network***");


 for(int trys = 1; trys<=10; trys++)                    //use a loop to attempt the connection more than once
 { 
    if ( status == WL_CONNECTED)                        //check to see if the connection was successful
    {
      Serial.print("Connected to ");
      Serial.println(ssid);
  
      PubNub.begin(pubkey, subkey);                      //connect to the PubNub Servers
      Serial.println("PubNub Connected"); 
      break;                                             //exit the connection loop     
    } 
    else 
    {
      Serial.print("Could Not Connect - Attempt:");
      Serial.println(trys);

    }

    if(trys==10)
    {
      Serial.println("I don't this this is going to work");
    }
    delay(1000);
 }

  
}


void publishToPubNub() // I am reading her pulse - publishing the pulse through motor and light 
{
  WiFiClient *client;
  StaticJsonBuffer<800> messageBuffer;                    //create a memory buffer to hold a JSON Object
  JsonObject& pMessage = messageBuffer.createObject();    //create a new JSON object in that buffer
  
 ///the imporant bit where you feed in values
  pMessage["Savaya"] = Threshold;                    //add the parameter "val2" to the object and give it a value (Savs data)
  pMessage["Emilia"] = Threshold2;                   //Emilias Data 

///                                                       //you can add/remove parameter as you like
  
  //pMessage.prettyPrintTo(Serial);   //uncomment this to see the messages in the serial monitor
  
  
  int mSize = pMessage.measureLength()+1;                     //determine the size of the JSON Message
  char msg[mSize];                                            //create a char array to hold the message 
  pMessage.printTo(msg,mSize);                               //convert the JSON object into simple text (needed for the PN Arduino client)
  
  client = PubNub.publish(pubChannel, msg);                      //publish the message to PubNub

  if (!client)                                                //error check the connection
  {
    Serial.println("client error");
    delay(1000);
    return;
  }
  
  if (PubNub.get_last_http_status_code_class() != PubNub::http_scc_success)
  {
    Serial.print("Got HTTP status code error from PubNub, class: ");
    Serial.print(PubNub.get_last_http_status_code_class(), DEC);
  }
  
  while (client->available()) 
  {
    Serial.write(client->read());
  }
  client->stop();
  Serial.println("Successful Publish");


  
}



//////void readFromPubNub() // I am reading her pulse to publish her light and motor 

void readFromPubNub()
{
  StaticJsonBuffer<1200> inBuffer;                    //create a memory buffer to hold a JSON Object
  WiFiClient *sClient =PubNub.history(subChannel,1); //using a 1 to say the last piece of history published 

  if (!sClient) {
    Serial.println("message read error");
    delay(1000);
    return;
  }

  while (sClient->connected()) 
  {
    while (sClient->connected() && !sClient->available()) ; // wait
    char c = sClient->read();
    JsonObject& sMessage = inBuffer.parse(*sClient);
    
    if(sMessage.success()) // when a message is sent and successful 
    {
      //sMessage.prettyPrintTo(Serial); //uncomment to see the JSON message in the serial monitor
      Threshold = sMessage["Savaya"];  //
//      Serial.print("Threshold");
//      Serial.println(Threshold);
     Threshold2 = sMessage["Emilia"];
//      Serial.print("Threshold2");
//      Serial.println(Threshold2);
      
    }
    
    
  }
  
  sClient->stop();

}

