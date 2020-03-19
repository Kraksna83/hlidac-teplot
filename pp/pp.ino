#include <OneWire.h> 
#include <DallasTemperature.h>
#include <LiquidCrystal.h>

//definice teplotniho rozestupu pro sepnuti 1 - 2 - 3 fazi a minimalniho rozestupu mezi spinanim 
const float DELTAT = 0.5;
const long PROSTOJFAZE = 30000; //v milisekundach, default 30s .  0 = bez prostoje. 
const long PROSTOJKOMINA = 30000; //v milisekundach, default 30s 


//spinane piny

const int F1 = 7;
const int F2 = 8;
const int F3 = 9;
const int CHLAD = 13;

//vstupni piny

const int AKOEF = 10; //koeficient, kterym se deli 1024 do skaly teplot. 
const int PTC1 = A0;
const int PTC2 = A1;
const int CUDL = 10;

// glob promenne

int nvev = 0;  //nastavena teplota vevnitr
int nkom = 100; //nastavena teplota v komine
long psepf = 0; //casova znamka posledniho sepnuti nektere faze. 
long psepk = 0; //casova znamka posledniho sepnuti komina
float vev = 0;
float pla = 0;
float kom = 0; 


// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 2, d5 = 3, d6 = 4, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//definice pro dallasy
DeviceAddress D_VEVNITR={ 0x28, 0x64, 0xDB, 0x8B, 0x03, 0x00, 0x00, 0x86 };
DeviceAddress D_PLAST = { 0x28, 0x0C, 0x06, 0x8C, 0x03, 0x00, 0x00, 0xD0 };
DeviceAddress D_KOMIN = { 0x28, 0xCA, 0xD9, 0x8B, 0x03, 0x00, 0x00, 0x4A }; 

#define ONE_WIRE_BUS 6
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


void lcdprint(int line, char* txt){
 int delka = strlen(txt);
 if ( delka < 17 ) {
  lcd.setCursor(0,line);
  lcd.print(txt);
  for ( int i = delka; i < 17; i++  ) {
   lcd.print(" ");  
  }
 }
}


void setup() {
  pinMode(F1,OUTPUT);
  pinMode(F2,OUTPUT);
  pinMode(F3,OUTPUT);

  pinMode(PTC1,INPUT);
  pinMode(PTC2,INPUT);
  pinMode(CUDL,INPUT);
  Serial.begin(57600);
  lcd.begin(16, 2);
  lcdprint(0, "Praci cest");
  lcdprint(1, "Nastav hodnoty");
  delay(2000);
  sensors.begin();
  while (true) {
    nvev = round(analogRead(PTC1) / AKOEF);
    nkom = round(analogRead(PTC2) / AKOEF);
    char buf[15] = "               " ;
    sprintf(buf, "Vevnitr: %d",nvev);
    lcdprint(0,buf);
    buf[15] = "               " ;
    sprintf(buf,"K: %d",nkom);
    lcdprint(1,buf);
    if (digitalRead(CUDL) == HIGH) {
        delay(1000);
         if (digitalRead(CUDL) == HIGH) {break;}
    }
    delay ( 10 );
  }
 
}

void vypisTeplot(){
  char cvev[5] = "";
  char cpla[5] = "";
  char ckom[4] = "";
  dtostrf(vev,5,2,cvev);
  dtostrf(pla,5,2,cpla);
  dtostrf(kom,4,2,ckom);
  char buf[15] = "               " ;
  sprintf(buf, "V:%s P:%s",cvev,cpla);
  Serial.print(buf);
  lcdprint(0,buf);
  buf[15] = "               " ;
  sprintf(buf,"K:%s N:%d:%d",ckom,nvev,nkom);
  lcdprint(1,buf);
  Serial.println(buf);
}

void pozapinejCoTreba(){
  //zmer teploty
  vev = sensors.getTempC(D_VEVNITR);
  pla = sensors.getTempC(D_PLAST);
  kom = sensors.getTempC(D_KOMIN); 
  
  //topne faze
  if ((millis() - psepf) > PROSTOJFAZE) {
    if (vev < nvev) { digitalWrite (F1, HIGH); psepf=millis(); } 
     else { 
      if (vev > nvev) { digitalWrite (F1, LOW); psepf=millis(); }
     }

    if (vev < nvev - DELTAT * 2) { digitalWrite (F2, HIGH); }
     else {digitalWrite (F2, LOW);}
    if (vev < nvev - DELTAT * 3) { digitalWrite (F3, HIGH); }
     else {digitalWrite (F3, LOW);}
  }

  //cerpadlo
  if ((millis() - psepk) > PROSTOJKOMINA) {
    if (kom > nkom ) { digitalWrite (F1, HIGH); psepk=millis(); }
    else {digitalWrite (F1, LOW); psepk=millis();}
  } 
}

void loop() {
  //zmer teploty
  

  pozapinejCoTreba();
  vypisTeplot();

  
  //prenastaveni 
  long ts = millis();
  int tnvev = 0;
  int tnkom = 100; 
  
  while (digitalRead(CUDL) == HIGH) {
    pozapinejCoTreba();
    tnvev = round(analogRead(PTC1) / AKOEF);
    tnkom = round(analogRead(PTC2) / AKOEF);
    char buf[15] = "               " ;
    sprintf(buf, "Vevnitr: %d",tnvev);
    lcdprint(0,buf);
    buf[15] = "               " ;
    sprintf(buf,"K: %d",tnkom);
    lcdprint(1,buf);
    delay ( 10 );
  }

  if (millis()- ts > 1000){
    nvev = tnvev;
    nkom = tnkom;
  }
  
    
  delay(10); 
  
}
