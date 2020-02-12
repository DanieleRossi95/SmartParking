#include <SoftwareSerial.h>
//#include <Servo.h>
#include <NewPing.h>
#include <LiquidCrystal_I2C.h>
#include <SharpIR.h>
#include <Adafruit_VL53L0X.h>

#define BUZZER_PIN 2
#define BUTTON_PIN 3
#define REDLED_PIN 4
#define GREENLED_PIN 5
#define IR_PIN 6
#define TRIGGER_PIN 7
#define ECHO_PIN 8
#define MAX_DIST 200
#define LOX_PIN 9
#define LOX_ADDRESS 0x29
#define SHARPIR_PIN A0
#define MODEL 430
#define RX_PIN 12
#define TX_PIN 13
// numero totale di parcheggi
#define slotNumber 3

bool parkFull = false;
bool buttonPressed = false;
// booleano che diventa true nel momento in cui la sbarra del parcheggio si alza e si aspetta l'entrata di un auto
bool carWaiting = false;
// booleano attraverso il quale si gestisce il controllo della distanza per stabilire se un auto sta parcheggio o meno
bool getDistance[slotNumber] = {true, true, true};
// booleano che è true quando il parcheggio è libero, mentre è false quando il parcheggio è occupato
bool canPark[slotNumber] = {true, true, true};
// booleano che è true nel momento in cui una macchina sta entrando nel parcheggio, è false invece quando una macchina
// è già entrata o deve ancora entrare
bool carInside = false;
// booleano che è true nel momento in cui una macchina sta parcheggiando o sta uscendo da uno slot, 
// così che gli altri slot rimangono idle
bool carParking = false;
bool carIn = false;
bool initialize = true;
int slotsLeft = 3;
unsigned long carCurrentMillis = 0; 
unsigned long carPreviousMillis = 0;
unsigned long sensorCurrentMillis[slotNumber] = {0,0,0}; 
unsigned long sensorPreviousMillis[slotNumber] = {0,0,0};
int dist[slotNumber] = {0,0,0};
int previousDistance[slotNumber] = {1000,1000,1000};
// soglia della distanza al di sotto della quale un auto si deve trovare per occupare un parcheggio
int sensorThreshold[slotNumber] = {6,6,6};
// tempo di attesa dell'entrata di una macchina nel parcheggio, superato questo tempo la sbarra si chiude
int carInterval = 2500;
// tempo di attesa del sensore ad ultrasuoni per verificare se un auto ha parcheggiato oppure no,
// così da evitare di considerare il parcheggio occupato anche quando un auto gli passa solo vicino 
int sensorInterval = 3000;
// soglia corrispondente alla differenza di distanza misurata tra due intervalli di tempo, 
// se viene superata significa che un auto sta entrando o sta lasciando un parcheggio 
int distanceThreshold = 20;
int i,n = 0;

String ao = "A";
String slot[slotNumber] = {"Slot1:", "Slot2:", "Slot3:"};
String s[slotNumber] = {"", "", ""};

char slotn[slotNumber] = {'0','1','2'};
String stream = "";
bool done = false;

//Servo servo;
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DIST);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
SharpIR sharpIR(SHARPIR_PIN, MODEL);

SoftwareSerial ArduinoUNO(RX_PIN,TX_PIN);

void setup() {
  Serial.begin(115200);
  ArduinoUNO.begin(9600);
  Serial.println();
  
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(IR_PIN, INPUT);
  pinMode(REDLED_PIN, OUTPUT);
  pinMode(GREENLED_PIN, OUTPUT);
  digitalWrite(REDLED_PIN, HIGH);
  digitalWrite(GREENLED_PIN, LOW);
  //servo.attach(SERVO_PIN);
  //servo.write(0);

  if(!lox.begin(LOX_ADDRESS)) {
    Serial.println("Failed to boot first VL53L0X");
    while(1);
  }
  delay(10);

  for(i=0; i<slotNumber; i++) {
    s[i] = slot[i] + ao;
  }
  writeLCD(true, s); 
}

void loop() {  
  if(ArduinoUNO.available() > 0) {
    char c = ArduinoUNO.read();
    if(c == 'i') {
      ArduinoUNO.print(slotsLeft);
      ArduinoUNO.print('\n');
      for(i=0; i<slotNumber; i++) {
        ArduinoUNO.print(s[i][s[i].length()-1]);
        ArduinoUNO.print('\n');
        ArduinoUNO.print(sensorThreshold[i]);
        ArduinoUNO.print('\n');  
        delay(100);
      }
      // confirm sending data termination
      ArduinoUNO.print('d');
    }
    else {
      for(i=0; i<slotNumber; i++) {
        if(c == slotn[i]) {
          done = false;
          while(!done) {
            while(ArduinoUNO.available() > 0) {
              c =  ArduinoUNO.read();
              if(c != 'd') 
                stream += c;
              else {
                done = true;
                ArduinoUNO.print('k');
                int st = stream.toInt();
                if(st >= 2)
                  sensorThreshold[i] = st;
                Serial.print(i);
                Serial.print(": ");
                Serial.println(st);
                stream = "";
              }    
            }
          }
        }
      }
    }
     
  }
  
  int buttonState = digitalRead(BUTTON_PIN);
  if(digitalRead(REDLED_PIN) == HIGH) {
    if(buttonState == HIGH) {
      if(!buttonPressed && !parkFull) {
        if(digitalRead(REDLED_PIN) == HIGH) {
          carInside = true;
          digitalWrite(BUZZER_PIN, HIGH);
          //servo.write(90);
          digitalWrite(GREENLED_PIN, HIGH);
          digitalWrite(REDLED_PIN, LOW);
          delay(500);
          digitalWrite(BUZZER_PIN, LOW);
        }  
        buttonPressed = true;
      }
    }
    else {
      buttonPressed = false;
    }
  }
  
  if(digitalRead(GREENLED_PIN) == HIGH) {
    carCurrentMillis = millis();
    if(!carWaiting) {
      carPreviousMillis = carCurrentMillis;
      carWaiting = true;
    }
    if(carCurrentMillis - carPreviousMillis > carInterval) {
      if(digitalRead(IR_PIN) == HIGH) {
        //carPreviousMillis = carCurrentMillis;
        carWaiting = false;
        delay(500);
        digitalWrite(BUZZER_PIN, HIGH);
        //servo.write(0);
        digitalWrite(REDLED_PIN, HIGH);
        digitalWrite(GREENLED_PIN, LOW);
        delay(500);
        digitalWrite(BUZZER_PIN, LOW);
      }
      else {
        if(carInside) {
          if(slotsLeft == 0) {
            parkFull = true;
          }
          else {
            carInside = false;
            carIn = true;
          }
        }
      }
    }
  }

  // get distance
  checkDistance(sonar.ping_cm(), 0);
  
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);
  if (measure.RangeStatus != 3) 
    checkDistance((int) measure.RangeMilliMeter / 10, 1);
  
  checkDistance(sharpIR.distance(), 2);

  for(i=0; i<slotNumber; i++) {
    sensorCurrentMillis[i] = millis();
    if(getDistance[i]) {
      previousDistance[i] = dist[i];
      getDistance[i] = false;
    }

    /*if(i == 0) {
      Serial.print("dist1: ");
      Serial.println(dist[0]);
      Serial.print("previousdist1: ");
      Serial.println(previousDistance[0]);
    }
    else if(i == 1) {
      Serial.print("dist2: ");
      Serial.println(dist[1]);
      Serial.print("previousdist2: ");
      Serial.println(previousDistance[1]);
    }
    else {
      Serial.print("dist3: ");
      Serial.println(dist[2]);
      Serial.print("previousdist3: ");
      Serial.println(previousDistance[2]);
    }*/
    
    if((dist[i] != previousDistance[i]) && (abs(dist[i] - previousDistance[i]) < distanceThreshold)) {
      sensorPreviousMillis[i] = sensorCurrentMillis[i];
    }  
    else
      getDistance[i] = true;
    if(dist[i] <= sensorThreshold[i] && carIn && canPark[i]) {  
      if(sensorCurrentMillis[i] - sensorPreviousMillis[i] > sensorInterval) {
        getDistance[i] = true;
        carIn = false;
        canPark[i] = false;
        ao = "O";
        s[i] = slot[i] + ao;
        writeLCD(false, s);
        slotsLeft--;
        
        // notify NodeMCU
        n = 0;
        done = false;
        while(!done && n<30) {
          ArduinoUNO.print(i);
          ArduinoUNO.print('O');
          ArduinoUNO.print('d');
          delay(500);
          while(ArduinoUNO.available() > 0) { 
            char c = ArduinoUNO.read();
            if(c == 'k')
              done = true;
          }
          n++;
        }
      }
    }
    else if(dist[i] > sensorThreshold[i] && !canPark[i]) {
      if(sensorCurrentMillis[i] - sensorPreviousMillis[i] > sensorInterval) {
        getDistance[i] = true;
        canPark[i] = true;
        ao = "A";
        s[i] = slot[i] + ao;
        writeLCD(false, s);
        slotsLeft ++;
        
        // notify NodeMCU
        n = 0;
        done = false;
        while(!done && n<30) {
          ArduinoUNO.print(i);
          ArduinoUNO.print('A');
          ArduinoUNO.print('d');
          delay(500);
          while(ArduinoUNO.available() > 0) { 
            char c = ArduinoUNO.read();
            if(c == 'k')
              done = true;
          }
          n++;
        }
      }
    }
  }
  //Serial.println("fine ciclo");
  //delay(1000);
  
}

void writeLCD(bool sl, String s[]) {
  if(sl) {
    lcd.init();
    lcd.backlight();
    lcd.clear();
  }  

  int n;
  for(n=0; n<slotNumber; n++) {
    if(n == 0)
      lcd.setCursor(0,0);
    else if(n == 1)
      lcd.setCursor(8,0);
    else if(n == 2)
      lcd.setCursor(0,1);
    lcd.print(s[n]);    
  }
}

void checkDistance(int d, int index) {
  if(d != 0)
    if(index == 1 && d < 4)
      dist[index] = 819;
    else
      dist[index] = d;  
}
