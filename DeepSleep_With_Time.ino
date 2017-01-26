
#include <JeeLib.h> // Low power functions library
#include <LCD5110_Basic.h> // nokia library
#include <SPI.h> // include SDCard libraries
#include <SD.h> // include SDCard libraries

// NOKIA 5110 LCD DATA
LCD5110 myGLCD(9, 3, 5, 7, 6);
extern uint8_t arduino_logo[];
extern uint8_t mordomus_logo[];
extern uint8_t save_img[];
extern uint8_t settings_img[];
extern uint8_t working1[];
extern uint8_t working2[];
extern uint8_t working3[];
extern uint8_t working4[];
extern uint8_t oshw_logo[];
extern uint8_t done_img[];
extern uint8_t temp_icon[];
extern uint8_t sleep_icon[];

extern uint8_t TinyFont[];
extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];
extern uint8_t BigNumbers[];

// VARIABLES
int sleepStatus = 0;              // variable to store a request for sleep
int count = 0;                    // counter
int wakeup_counter  = 1;          // counts how many times arduino wake up
int sleeptimer = 5;               // how many seconds will stay waked up?
int sleep_time = 1;               // cycle every x minutes
int timetosleep = 10000;          // max time available = 60 seconds
int x = 0;
float v_perc;                     // percentage 
int check = 0;

#define ThermistorPIN 0           // Analog Pin 0
double temp;
int led_pin = 13;
ISR(WDT_vect) { Sleepy::watchdogEvent(); } // Setup the watchdog

String sdcard_status;
String conteudo_a_gravar; 
String filename = "temp.txt"; // defines the name of the file
File myFile;

float Aref = 1.260; // change this to the actual Aref voltage of ---YOUR--- Arduino, or adjust to get accurate voltage reading (1.000- 1.200)

unsigned int total; // A/D output
float voltage1; // converted to volt
int Apin_to_read = 5; // analog pin to read voltage

  
void setup() {

  //analogReference(DEFAULT); 
  
  pinMode(8, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH); // LED ON
  digitalWrite(8, LOW);   // Backlight ON
  
  Serial.begin(9600);
  
  myGLCD.InitLCD(); // init LCD
  sdcard_init(); // init SD Card

  // design Logo 
  myGLCD.drawBitmap(0, 0, mordomus_logo, 84, 48);
  myGLCD.setFont(SmallFont);  
  myGLCD.print("MORDOMUS v0.3", CENTER, 40);
  delay(1000);

  

} // END SETUP 

void loop() {

 
  // here goes the code
  if (count == 1){

      myGLCD.clrScr();

      String conteudo_a_gravar = "";      
      myGLCD.drawBitmap(0, 0, temp_icon, 48, 48);  
  
      temp = Thermistor(analogRead(ThermistorPIN));         // read ADC and convert it to Celsius
      Serial.print(", Celsius: "); printDouble(temp,1);     // display Celsius
      Serial.println("");
      
      myGLCD.setFont(SmallFont);   
      myGLCD.print(sdcard_status, 32, 40);
         
      myGLCD.setFont(MediumNumbers); // set MediumNumbers font
      myGLCD.printNumF(temp, 1, RIGHT, 16);
      delay (100);
     
      conteudo_a_gravar = temp;  
      writetocard(conteudo_a_gravar);
      
  }
  
  // compute the serial input
  if (Serial.available()) {
    int val = Serial.read();
    if (val == 'S') {
      Serial.println("Serial: Entering Sleep mode");
      delay(100);     // this delay is needed, the sleep
                      //function will provoke a Serial error otherwise!!
      count = 0;
      sleepNow();     // sleep function called here
    }
    if (val == 'd'){
      deleteFile();
    }
    if (val == 'r')
    {
      readFileContent();
    }
  }
 
  // check if it should go to sleep because of time
  if (count > sleeptimer) {
      Serial.println("Timer: Entering Sleep mode");
      delay(100);     // this delay is needed, the sleep
                      //function will provoke a Serial error otherwise!!
      count = 0;
      sleepNow();     // sleep function called here

  }

  // display information about the counter
  Serial.print("Awake for ");
  Serial.print(count);
  Serial.println(" sec");
  myGLCD.setFont(SmallFont);
  myGLCD.printNumI(count, RIGHT, 40);
  
  count++;
  delay(1000); // wait for a second
 
} // END LOOP


// THERMISTOR DATA
double Thermistor(int RawADC) {

 // Inputs ADC Value from Thermistor and outputs Temperature in Celsius
 //  requires: include <math.h>
 //  Utilizes the Steinhart-Hart Thermistor Equation:
 //    Temperature in Kelvin = 1 / {A + B[ln(R)] + C[ln(R)]^3}
 //    where A = 0.001129148, B = 0.000234125 and C = 8.76741E-08
 
 long Resistance;  double Temp;  // Dual-Purpose variable to save space.
 
 Resistance=10000.0*((1024.0/RawADC) - 1);  // Assuming a 10k Thermistor.  Calculation is actually: Resistance = (1024 /ADC -1) * BalanceResistor
// For a GND-Thermistor-PullUp--Varef circuit it would be Rtherm=Rpullup/(1024.0/ADC-1)
 Temp = log(Resistance); // Saving the Log(resistance) so not to calculate it 4 times later. // "Temp" means "Temporary" on this line.
 Temp = 1 / (0.001129148 + (0.000234125 * Temp) + (0.0000000876741 * Temp * Temp * Temp));   // Now it means both "Temporary" and "Temperature"
 Temp = Temp - 273.15;  // Convert Kelvin to Celsius                                         // Now it only means "Temperature"
 

 // BEGIN- Remove these lines for the function not to display anything
  Serial.print("ADC: "); Serial.print(RawADC); Serial.print("/1024");  // Print out RAW ADC Number
  Serial.print(", Volts: "); printDouble(((RawADC*5.050)/1024.0),3);   // 4.860 volts is what my USB Port outputs.
  Serial.print(", Resistance: "); Serial.print(Resistance); Serial.print("ohms");
 // END- Remove these lines for the function not to display anything

 // Uncomment this line for the function to return Fahrenheit instead.
 //Temp = (Temp * 9.0)/ 5.0 + 32.0; // Convert to Fahrenheit
 return Temp;  // Return the Temperature
 
} // END THERMISTOR

void printDouble(double val, byte precision) {
  // prints val with number of decimal places determine by precision
  // precision is a number from 0 to 6 indicating the desired decimal places
  // example: printDouble(3.1415, 2); // prints 3.14 (two decimal places)
  Serial.print (int(val));  //prints the int part
  if( precision > 0) {
    Serial.print("."); // print the decimal point
    unsigned long frac, mult = 1;
    byte padding = precision -1;
    while(precision--) mult *=10;
    if(val >= 0) frac = (val - int(val)) * mult; else frac = (int(val) - val) * mult;
    unsigned long frac1 = frac;
    while(frac1 /= 10) padding--;
    while(padding--) Serial.print("0");
    Serial.print(frac,DEC) ;
  }
} // END PRINTDOUBLE


void sleepNow()         // here we put the arduino to sleep
{

     
     myGLCD.clrScr();     
     myGLCD.drawBitmap(0, 0, sleep_icon, 84, 48);
     
     myGLCD.setFont(SmallFont);  
     myGLCD.print("zZ", 8, 8); 
     myGLCD.print(" ", RIGHT, 40);
     myGLCD.printNumI(sleep_time, RIGHT, 32);
     myGLCD.print("min", RIGHT, 40);      
     
     check = 0;
     readvoltage(check);

     if (voltage1 < 4.25) {
     digitalWrite(8, HIGH); // Backlight OFF 
     digitalWrite(13, LOW); // Led OFF
     }
     
     
     //myGLCD.enableSleep();

     uptime(); // function to show uptime at LCD
     
     Serial.println("Sleep...");
     Serial.println(" ");
     Serial.println(" ");
     
     delay(50);        
     for (byte x = 0; x < sleep_time; ++x)
     Sleepy::loseSomeTime(timetosleep);

     //myGLCD.disableSleep();

       
  wakeup_counter = wakeup_counter +1;
  
  wakeUpNow();

} // END SLEEPNOW



void wakeUpNow()        // here the interrupt is handled after wakeup
{

    digitalWrite(8, LOW);     // Backlight ON
    digitalWrite(13, HIGH);   // LED ON    
    
    /*Serial.println(" ");
    Serial.println("Power Up!");
    myGLCD.clrScr();   
    
    check = 2;  
    readvoltage(check);
    */
    
} // END WAKEUPNOW




void writetocard(String conteudo_a_gravar) {  
  
  
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.

  myFile = SD.open(filename, FILE_WRITE);


  // if the file opened, write it:
  if (myFile) {
    //myGLCD.drawBitmap(0, 0, save_img, 84, 48);
    Serial.print("Writing to ");
    Serial.print(filename);
    Serial.print(" ");


    myGLCD.setFont(SmallFont);  
    myGLCD.print("[*]", 49, 40);  
    Serial.println(conteudo_a_gravar);
    myFile.println(conteudo_a_gravar);

    delay(500);
    myGLCD.print("[ ]", 49, 40);  

    myFile.close(); // close the file

    //myGLCD.clrScr(); // clear the LCD
    //myGLCD.drawBitmap(0, 0, save_img, 84, 48);

    //myGLCD.clrScr();

  } else {

    // if file doesn't open, check if file exists
      if (SD.exists(filename)) {
        Serial.print("ficheiro "); Serial.print(filename); Serial.println(" existe");
        
      } else {
        Serial.print("ficheiro "); Serial.print(filename); Serial.println(" não existe");
        sdcard_init();
        myFile = SD.open(filename, FILE_WRITE);
        myFile.println("FILE CREATED");
        myFile.close(); // close the file
      }

  }

} // END WRITETOCARD

void readvoltage(int check)  {

 
if (check == 0) {
  
   analogReference(INTERNAL); 
   analogRead(Apin_to_read); // one unused reading to clear old sh#t
   
    for (int x = 0; x < 16; x++) { // 16 analogue readings and 1/16 voltage divider = no additional maths
      total = total + analogRead(Apin_to_read); // add each value
    }
    voltage1 = total * Aref / 1024; // convert readings to volt
}

if (check == 1) {
}

 analogReference(INTERNAL); 
 analogRead(Apin_to_read); // one unused reading to clear old sh#t
 
  for (int x = 0; x < 16; x++) { // 16 analogue readings and 1/16 voltage divider = no additional maths
    total = total + analogRead(Apin_to_read); // add each value
  }
  voltage1 = total * Aref / 1024; // convert readings to volt
  float voltage2 = total * Aref / 1024;

  //myGLCD.setFont(MediumNumbers); // set BigNumbers font
  myGLCD.setFont(SmallFont);  
  
  if (voltage1 <= 4.25) {
  Serial.print("Voltage Read: "); Serial.print(voltage1); Serial.print(" | "); Serial.println(check);
  myGLCD.printNumF(voltage1, 2, RIGHT, 0);
  
  float dif = 1.25;
  float consumo;
  consumo = 4.25 - voltage1;
  v_perc = 100-(consumo*100/dif);
  
  //v_perc = (voltage1/4.25)*100;

  myGLCD.printNumI(v_perc, 66, 8);  
  myGLCD.print("%", RIGHT, 8);  
  
  }else{
  
  Serial.print("V: "); Serial.print(voltage1); Serial.print(" | "); Serial.println(check);
  myGLCD.print(" usb", RIGHT, 0);  
 
  }


  total = 0; // reset the analog readings counter
  analogReference(DEFAULT); 
  
} // END READVOLTAGE


void deleteFile() {
  myFile.close();
  SD.remove(filename);
  Serial.print(filename);
  Serial.println(" deleted.");

} // END DELETEFILE


void readFileContent() {

  // open the file for reading:
  myFile = SD.open(filename);
  if (myFile) {
    Serial.print("Reading File Content of: "); Serial.println(filename);

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("Error Opening File.");
    myGLCD.clrScr();
    myGLCD.print("SD Open Err", CENTER, 16);
  }
}


void sdcard_init(){

  
if (!SD.begin(4)) {
    Serial.println("[Off]");
    myGLCD.print("SD Card Fail", CENTER, 16);
    sdcard_status = "[ ]";
  }else{
    Serial.println("[On]");
    myGLCD.print("SD Card OK", CENTER, 16);
    sdcard_status = "[x]";
  }
  delay(250);
  myGLCD.clrScr();  
}

void uptime(){
   
     // arduino uptime running
     int wakeup_hour = 0;
     int wakeup_minutes;
     
     wakeup_minutes = wakeup_counter;
    
     if (wakeup_counter >= 60){
      
     wakeup_hour = wakeup_counter/60;
     wakeup_minutes = wakeup_counter-(60*wakeup_hour);
     }
     
     if (wakeup_counter < 60){
     wakeup_hour = 0; 
     }

  if (wakeup_hour < 10){
    myGLCD.print("0", 0, 40); 
    myGLCD.printNumI(wakeup_hour, 8, 40);
  
  }else{
    myGLCD.printNumI(wakeup_hour, 0, 40);
  }
  
  myGLCD.print("h", 16, 40);
  
  if (wakeup_minutes < 10){
    myGLCD.print("0", 24, 40); 
    myGLCD.printNumI(wakeup_minutes, 32, 40);
  
  }else{
    myGLCD.printNumI(wakeup_minutes, 24, 40);
  }
    
}

