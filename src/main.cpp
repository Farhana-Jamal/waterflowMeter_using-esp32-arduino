#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <ThingerESP32.h>

#define Sensor 4

#define OLED_width 128
#define OLED_height 64
#define OLED_reset -1

Adafruit_SSD1306 display(OLED_width , OLED_height , &Wire , OLED_reset);

#define SSID "Farhana"
#define SSID_PASS "1234567890"

#define USERNAME "Farhana"
#define DEVICE_ID "Waterflow_Meter"
#define DEVICE_CREDENTIAL  "ZFrSokf+_01&fDln"

ThingerESP32 thing(USERNAME , DEVICE_ID , DEVICE_CREDENTIAL);

long currentMillis = 0;
long previousMillis = 0;
volatile byte pulsecount;
byte pulse1Sec = 0;
int interval = 1000;
float flowRate = 0;
float flowMillilitres =0;
float totalMillilitres = 0;
float totalQuantity = 0;
float callibrationFactor = 3.33;

void displayInThinger()
{
  thing.add_wifi(SSID , SSID_PASS);

  thing.set_state_listener([&](ThingerClient::THINGER_STATE state)
  {
    if(state = ThingerClient::NETWORK_CONNECTED)
      display.println("Connected");
    else 
      display.println("Not Connected");
     
  });
  

  thing["Flowrate"]>>outputValue(flowRate);
  thing["Quantity"]>>outputValue(totalMillilitres); 
}

void OLEDdisplay(float totalFlow , float quantity)
{
  display.begin(SSD1306_SWITCHCAPVCC,0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,18);
  display.print("Flowrate : ");
  display.println(totalFlow);
  display.println(" ");
  display.print("Quantity: ");
  display.println(quantity);
  display.display();
  delay(1000);
  display.clearDisplay();

}

void IRAM_ATTR pulseCounter()
{
  pulsecount++;
}

void setup()
{
  Serial.begin(115200);

  pinMode(Sensor,INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(Sensor),pulseCounter,RISING);

  displayInThinger();
}

void calculations()
{
  currentMillis = millis();
  if ((currentMillis - previousMillis) > interval)
  {
    pulse1Sec = pulsecount;
    pulsecount = 0;

    flowRate= (((1000.0 / (millis() - previousMillis)) * pulse1Sec) / callibrationFactor);        //flowRAte in L/min
    previousMillis = millis();

    flowMillilitres = ((flowRate / 60) * 1000);                   // in mL/s

    totalMillilitres = totalMillilitres + flowMillilitres;                          // in mL/s
    totalQuantity = totalMillilitres / 1000;                     //in L/s

    Serial.print("FLOWRATE : ");
    Serial.print(flowRate);
    Serial.println("L/min");

    Serial.print("TOTAL QUANTITY : ");
    Serial.print(totalQuantity);                
    Serial.println("L/s");


  }
}
void loop()
{
  thing.handle();
  calculations();
  OLEDdisplay(flowRate,totalQuantity);
}
