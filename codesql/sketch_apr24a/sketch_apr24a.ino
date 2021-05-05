#include <ACS712.h>





#define BLYNK_PRINT Serial    
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

char auth[] = "_qpHJMk447tF0GwGWevKI-717CBGiSJp";
char ssid[] = "TuanSang";
char pass[] = "0343215116";

#define SENSOR  0
long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
boolean ledState = LOW;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
float flowMilliLitres;
unsigned int totalMilliLitres;
float flowLitres;
float totalLitres;
float h;
 
void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}

const int sensorIn = A0;
int mVperAmp = 185; // use 185 for 5A, 100 for 20A Module and 66 for 30A Module

double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;

void setup(){ 
      pinMode(A0, INPUT);
      Serial.begin(115200);
      delay(10);
      Serial.println(F("Init...."));
//lưu lượng nước
      Serial.begin(9600);
      Blynk.begin(auth, ssid, pass);

      pinMode(SENSOR, INPUT_PULLUP);
 
      pulseCount = 0;
      flowRate = 0.0;
      flowMilliLitres = 0.0;
      totalMilliLitres = 0;
      previousMillis = 0;
 
      attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);
}

void loop(){
 Voltage = getVPP();
 VRMS = (Voltage/2.0) *0.707; // sq root
 AmpsRMS = (VRMS * 1000)/mVperAmp;
 float Wattage = (220*AmpsRMS)-18; //Observed 18-20 Watt when no load was connected, so substracting offset value to get real consumption.
 Serial.print(AmpsRMS);
 Serial.println(" Amps RMS ");
 Serial.print(Wattage); 
 Serial.println(" Watt ");

//lưu lượng nước
    pulse1Sec = pulseCount;
    pulseCount = 0;
 
    
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();
 
    
    flowMilliLitres = (flowRate / 60) * 1000;
    flowLitres = (flowRate / 60);
 
    
    totalMilliLitres += flowMilliLitres;
    totalLitres += flowLitres;
    
     h=totalMilliLitres/(5*5);
    Serial.print("Flow rate: ");
    Serial.print(float(flowMilliLitres));  
    Serial.print("mL/s");
    Serial.print("\t");       
    
    Serial.print("chieucao: ");
    Serial.print(float(h));  
    Serial.print("cm");
    Serial.print("\t");      

   
   Serial.print(flowMilliLitres);
   Serial.println("flowMilliLitres:");
   Blynk.virtualWrite(V6, flowMilliLitres);

   Serial.print(h);
   Serial.println("chieucao:");
   Blynk.virtualWrite(V5, h);
   delay(200);
   Serial.print(Wattage);
   Serial.println("Công suất:");
   Blynk.virtualWrite(V7, Wattage);
   delay(200);
   
  Blynk.run();
}

float getVPP()
{
  float result;
  
  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  int minValue = 1024;          // store min value here
  
   uint32_t start_time = millis();

   while((millis()-start_time) < 1000) //sample for 1 Sec
   {
       readValue = analogRead(sensorIn);
       // see if you have a new maxValue
       if (readValue > maxValue) 
       {
           /*record the maximum sensor value*/
           maxValue = readValue;
       }
       if (readValue < minValue) 
       {
           /*record the maximum sensor value*/
           minValue = readValue;
       }
/*       Serial.print(readValue);
       Serial.println(" readValue ");
       Serial.print(maxValue);
       Serial.println(" maxValue ");
       Serial.print(minValue);
       Serial.println(" minValue ");
       delay(1000); */
    }
   
   // Subtract min from max
   result = ((maxValue - minValue) * 5)/1024.0;
      
   return result;

   
 }
