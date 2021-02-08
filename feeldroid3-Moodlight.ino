#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include "time.h" 
#include "esp_attr.h"
#ifdef __AVR__
 #include <avr/power.h> 
 #include <stdlib.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN 32

const char* ssid     = "Fritzbox"; //Enter(replace) Station Name here BSSID
const char* password = "wifipassword33"; //Replace Password here

const char* ntpServer = "ntp1.t-online.de";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

 
float weight = 0.01; // how much your values are 'smoothed'
int hour=1;
int ADC1 = 33;
int ADC2 = 35;
float MQ135 = 0;
float MQ9 =0;
float MQ135_volt = 0;
float MQ9_volt =0;
int gaugeMQ9 = 0;
int gaugeMQ135 = 0;
int R=0;
int G=0;
int bright= 0;
int interrupt = 0;
int buttonPin = 5;
int ledPin = 18;
int ledState = LOW;         // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 200;    // the debounce time; increase if the output flickers
static unsigned long last_interrupt_time = 0;
static unsigned long lastpresscode = 0;
float MQ9_array[50];
float MQ135_array[50];
int pressed = 1;
int brightness= 20;
static unsigned long wait_last =0; 
int fade = 0;


// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 72

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);
//GRB!!

#define DELAYVAL 50 // Time (in milliseconds) to pause between pixels

void IRAM_ATTR blink() {
   //Interrupt for the momentarz switch with debounce and counter
  
   unsigned long interrupt_time = millis();
   
   // If interrupts come faster than 200ms, assume it's a bounce and ignore
   if (interrupt_time - last_interrupt_time > 500)
   {          
     pressed = pressed + 1;
     //delay(50);
     }  
   
   last_interrupt_time = interrupt_time;
   } 


void disconnectWifi(){
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF); 
}

void setup() {
Serial.begin(115200);

pinMode(ADC1, INPUT);
pinMode(ADC2, INPUT);
pinMode(ledPin, OUTPUT);
pinMode(buttonPin, INPUT_PULLUP);
attachInterrupt(digitalPinToInterrupt(buttonPin), blink, CHANGE);
  
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  //pixels.clear(); // Set all pixel colors to 'off'
  pixels.show(); // Initialize all pixels to 'off
  
  //color
  

  //pixels.setBrightness(5);
  digitalWrite(ledPin, ledState);
  connectWifi();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);   
  hour = printLocalTime();
  disconnectWifi(); 
  //Sensorwarmup
  delay(5000);
}

uint32_t white = pixels.Color(255,255,255);
uint32_t black = pixels.Color(0,0,0);
uint32_t red = pixels.Color(0,100,0);
uint32_t green = pixels.Color(100,0,0);
uint32_t yellow = pixels.Color(100,100,0);

int printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
  }
  char timeHour[3];
   //int timeHour= timeinfo.hour);
   
   //Serial.println(&timeinfo,"%H");
   //Serial.println(&timeinfo,"%H:%M:%S");
   strftime(timeHour,3, "%H", &timeinfo);
   int timeHour_int = atoi(timeHour);
   //Serial.println(timeHour_int);   

  return timeHour_int;
}

void connectWifi(){
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  if (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    WiFi.begin(ssid, password);
    
    while(WiFi.status() != WL_CONNECTED){
      pixels.setPixelColor(40, pixels.Color(00, 0, 100));
      pixels.show();
    delay(3500);}    
  }
  Serial.println("");
  Serial.println("WiFi connected.");  
}

void gasdetect(){
        
        digitalWrite(ledPin, HIGH);  
        pixels.setBrightness(brightness);        
         
        //MQ135 = analogRead(ADC1);
        //MQ9 = analogRead(ADC2);
         
        //Filter       
        MQ135 = (1.0-weight) * MQ135   +   weight * analogRead(ADC1);          
        MQ9 = (1.0-weight) * MQ9   +   weight * analogRead(ADC2);        
        
        MQ135_volt = (MQ135 * 3.3 ) / (4095);       
        MQ9_volt = (MQ9 * 3.3 ) / (4095);          
        
        gaugeMQ9 = map(MQ9,0,2000,0,31);
        gaugeMQ135 = map(MQ135, 0,500,1,39);     
        //Serial.println(gaugeMQ135);
        //Serial.println(gaugeMQ9);

        //Warninglevels
        
        if (gaugeMQ135 <= 3){pixels.fill(white,0,gaugeMQ135);}
        if (gaugeMQ135 >3 && gaugeMQ135 <=15){pixels.fill(yellow,0,gaugeMQ135);}
        if (gaugeMQ135 >15 && gaugeMQ135 <39){pixels.fill(red,0,gaugeMQ135);}

        if (gaugeMQ9 <= 5){pixels.fill(green,40,gaugeMQ9);}
        if (gaugeMQ9 >5 && gaugeMQ9 <=10){pixels.fill(yellow,40,gaugeMQ9);}
        if (gaugeMQ9 >10 && gaugeMQ9 <39){pixels.fill(red,40,gaugeMQ9);}        
        
        pixels.show();        
        pixels.clear();  
}        
 

void lamp(){
           digitalWrite(ledPin, HIGH); 
       
           pixels.setBrightness(brightness) ;
           
           if (WiFi.status() == WL_CONNECTED){
           hour = printLocalTime();
           
           //pixels.show();
           //disconnectWifi(); 
           Serial.println(hour);
           }
           
           else{
           connectWifi();
           //"Connecting WiFi" red dot
           pixels.setPixelColor(40, pixels.Color(100, 0, 00));
           pixels.show();
           
           //Serial.println("Waiting");
           //Serial.println(hour);
           }
              
           if (hour >=18 && hour < 24){
                  //Fade from whtish at 18 to redish at 24 
                 int fadeG=map(hour,18,24,204,5);
                 int fadeR=map(hour,18,14,220,255);
                 int fadeB=map(hour,18,24,204,5);
                 uint32_t eve = pixels.Color(fadeG,255, fadeB);                 
                 pixels.fill(eve,0,72);
                 pixels.show();
              
           }
           
           if (hour > 7 && hour < 18){           
                 //Maximum White over the working hours
                 pixels.fill(white,0,72);
                 pixels.show();              
           }
           
           if (hour >= 0 && hour < 8 ){
              //Switch off between 0 and 8
              pixels.setBrightness(0);
              pixels.show();
           }         
                             
      } 
                      
           //Serial.println(hour);
        
        


void loop(){ 
    //Serial.println(last_interrupt_time);
    //pixels.clear();
    while (pressed == 1){      
      gasdetect();
      }
        
    while (pressed == 2){
      lamp();
      }
           
    while (pressed == 3){
      digitalWrite(ledPin, LOW); 
      disconnectWifi();
      pixels.clear();
      pixels.show(); 
      pressed = 0;                   
      }
   
}



  
  





        

      
