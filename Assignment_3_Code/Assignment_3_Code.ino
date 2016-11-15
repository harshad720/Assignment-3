// This #include statement was automatically added by the Particle IDE.
#include "SparkFunMicroOLED/SparkFunMicroOLED.h"

// This #include statement was automatically added by the Particle IDE.
#include "OneWire/OneWire.h"

// This #include statement was automatically added by the Particle IDE.
#include "neopixel/neopixel.h"

// This #include statement was automatically added by the Particle IDE.
#include "neopixel/neopixel.h"
#include "application.h"
#include "OneWire/OneWire.h"

OneWire ds = OneWire(D4);  // 1-wire signal on pin D4
unsigned long lastUpdate = 0;
#define PIXEL_PIN D2
#define PIXEL_COUNT 64
#define PIXEL_TYPE WS2812B
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);
void rainbow(uint8_t wait);
uint32_t Wheel(byte WheelPos);
//OLED 
//1 is blue 2 is green 3 is red
MicroOLED oled;

int m1[8][8]; //matrix for displaying the graph

int arr[8]; //holds the values for the graph

void print() //prints the matrix values on the neopixel matrix
{
    Serial.println();
    for(int i=0;i<8;i++)
    {
        for(int j=0;j<8;j++)
        {
            if(m1[i][j]==1)
            {
                strip.setPixelColor((i*8)+j,0, 0, 255);
            }
            else if(m1[i][j]==2)
            {
                strip.setPixelColor((i*8)+j, 0, 255, 0);
            }
            else if(m1[i][j]==3)
            {
                strip.setPixelColor((i*8)+j,255, 0, 0);
            }
            Serial.print(m1[i][j]);
        }
        Serial.println();
    }
}

void refresh() //resets the matrix to 0
{
    for(int i=0;i<64;i++)
    {
        strip.setPixelColor(i,0,0,0);
    }
    for(int i=0;i<8;i++)
    {
        for(int j=0;j<8;j++)
        {
            m1[i][j]=0;
        }
    }
}

void plotGraph(int x[]) //adds values in the matrix m1
{
    for(int i=0;i<8;i++)
    {
        for(int j=0;j<8;j++)
        {
            if((x[i]%8)>=j)
            {
                switch(x[i]/8)
                {
                    case 1: m1[i][j]=1;break;
                    case 2: m1[i][j]=2;break;
                    case 3: m1[i][j]=3;
                        tone(D0,3000,500);break;
                }
            }
        }
    }
    print();
    strip.show();
}

void updateArray(int data)  //updates the values in the array arr
{
    for(int i=0;i<7;i++)
    {
        arr[i]=arr[i+1];
    }
    arr[7]=data;
}

void printOnOLED(String title, int font) //displays the value on the OLED screen
{
    oled.clear(PAGE);
    oled.setFontType(font);
    // Try to set the cursor in the middle of the screen
    oled.setCursor(0, 0);
    // Print the title:
    oled.print(title);
    oled.display();
    delay(1000);
    oled.clear(PAGE);
}

void setup() 
{
    Serial.begin(9600);
    pinMode(D3, OUTPUT);
    pinMode(D5, OUTPUT);
    digitalWrite(D3, LOW);
    digitalWrite(D5, HIGH);
    strip.begin();
    strip.show();
    strip.setBrightness(10);
    Particle.subscribe("tdata",myHandler);
    refresh();
    for(int i=0;i<8;i++)
    {
        arr[i]=i;
    }
    oled.begin();    // Initialize the OLED
    oled.clear(ALL); // Clear the display's internal memory
    oled.display();  // Display what's in the buffer (splashscreen)
    delay(1000);     // Delay 1000 ms
    oled.clear(PAGE); // Clear the buffer.
    randomSeed(analogRead(A0) + analogRead(A1));
}



void myHandler (const char *event, const char *data)    //handles the data coming from the cloud and sends it to the different functions 
{
    Serial.print("\n\n\n DATA:::");
    Serial.println(data);
    int i;
    sscanf(data, "%d", &i);
    //display_temp(i);
    refresh();
    updateArray(i);
    printOnOLED(data,2);
}



void display_temp(int data) //function that displays the temprature on a LED strip
{
    for(int i=0;i<strip.numPixels();i++)
    {
        if(i<data)
        strip.setPixelColor(i, strip.Color(255, 0, 0));
        else
        //strip.setPixelColor(i, strip.Color(0, 0, 255));
        strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
    strip.show();
    delay(100);
}

void loop(void) {
    
    //---------------temprature sensor---------
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;

  if ( !ds.search(addr)) {
    ds.reset_search();
    delay(250);
    return;
  }
  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  Serial.println();

  // we have a good address at this point
  // what kind of chip do we have?
  // we will set a type_s value for known types or just return
type_s = 0;
  // this device has temp so let's read it

  ds.reset();               // first clear the 1-wire bus
  ds.select(addr);          // now select the device we just found
  // ds.write(0x44, 1);     // tell it to start a conversion, with parasite power on at the end
  ds.write(0x44, 0);        // or start conversion in powered mode (bus finishes low)

  // just wait a second while the conversion takes place
  // different chips have different conversion times, check the specs, 1 sec is worse case + 250ms
  // you could also communicate with other devices if you like but you would need
  // to already know their address to select them.

  delay(1000);     // maybe 750ms is enough, maybe not, wait 1 sec for conversion
  
  // we might do a ds.depower() (parasite) here, but the reset will take care of it.

  // first make sure current values are in the scratch pad

  present = ds.reset();
  ds.select(addr);
  ds.write(0xB8,0);         // Recall Memory 0
  ds.write(0x00,0);         // Recall Memory 0

  // now read the scratch pad

  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE,0);         // Read Scratchpad
  if (type_s == 2) {
    ds.write(0x00,0);       // The DS2438 needs a page# to read
  }
  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s == 2) raw = (data[2] << 8) | data[1];
  byte cfg = (data[4] & 0x60);

  switch (type_s) {
    case 1:
      raw = raw << 3; // 9 bit resolution default
      if (data[7] == 0x10) {
        // "count remain" gives full 12 bit resolution
        raw = (raw & 0xFFF0) + 12 - data[6];
      }
      celsius = (float)raw * 0.0625;
      break;
    case 0:
      if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
      if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
      if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
      // default is 12 bit resolution, 750 ms conversion time
      celsius = (float)raw * 0.0625;
      break;

    case 2:
      data[1] = (data[1] >> 3) & 0x1f;
      if (data[2] > 127) {
        celsius = (float)data[2] - ((float)data[1] * .03125);
      }else{
        celsius = (float)data[2] + ((float)data[1] * .03125);
      }
  }
    char x[10];
  fahrenheit = celsius * 1.8 + 32.0;
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  Serial.print(fahrenheit);
  Serial.println(" Fahrenheit");
  int q=(int)celsius;
  sprintf(x,"%d",q);
  Particle.publish("tdata",x);
  
  plotGraph(arr);
  
  
}

