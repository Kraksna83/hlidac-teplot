#include <OneWire.h> 
#include <DallasTemperature.h>
#include <LiquidCrystal.h>
#include <String.h>
//debug

const bool KOMENTATOR = true; 

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
float vevn = 0;
float pla = 0;
float kom = 0; 
bool dbg = false;

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
//const int rs = 12, en = 11, d4 = 2, d5 = 3, d6 = 4, d7 = 5;
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//definice pro dallasy
DeviceAddress D_VEVNITR={ 0x28, 0x3C, 0x60, 0xDA, 0x02, 0x00, 0x00, 0xCA };
DeviceAddress D_PLAST = { 0x28, 0x72, 0x85, 0xDA, 0x02, 0x00, 0x00, 0xA4 };
DeviceAddress D_KOMIN = { 0x28, 0x4F, 0x9B, 0xDA, 0x02, 0x00, 0x00, 0xDD }; 

#define ONE_WIRE_BUS 6
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


void lcdprint(int line, char* txt){
 int delka = strlen(txt);
 if ( delka < 17 ) {
  lcd.setCursor(0,line);
  lcd.print(txt);
//  for ( int i = delka; i < 17; i++  ) {
//   lcd.print(" ");  
//  }
 }
}



void setup() {
  pinMode(F1,OUTPUT);
  pinMode(F2,OUTPUT);
  pinMode(F3,OUTPUT);
  pinMode(CHLAD, OUTPUT);

  pinMode(PTC1,INPUT);
  pinMode(PTC2,INPUT);
  pinMode(CUDL,INPUT);
  Serial.begin(57600);
  if (KOMENTATOR) { Serial.println("Startuju LCD a senzory"); }
  lcd.begin(16, 2);
 // lcd.setCursor(0,0);
 // lcd.print("nasraz");
 // delay(20000);
  lcdprint(0, "Praci cest");
  lcdprint(1, "Nastav hodnoty");
  delay(2000);
  
  sensors.begin();
  if (KOMENTATOR) { Serial.println("Senzory nastartovany"); }
  while (true) {
    nvev = round(analogRead(PTC1) / AKOEF);
    nkom = round(analogRead(PTC2) / AKOEF);
    lcd.clear();
    char buf[16] = "                " ;
    sprintf(buf, "Vevnitr: %d",nvev);
    
    
    if (KOMENTATOR) { Serial.println(buf); }
    lcd.print(buf);
    //lcdprint(0,buf);
    
    
    char buf2[16] = "                " ;
    sprintf(buf2,"K: %d",nkom);
    
    if (KOMENTATOR) { Serial.println(buf2); }
    lcd.setCursor(0,1);
    lcd.print(buf2);
    
    if (digitalRead(CUDL) == HIGH) {
        delay(1000);
         if (digitalRead(CUDL) == HIGH) {break;}
    }
    delay ( 100 );
  }
  if (KOMENTATOR) { Serial.print("Nastaveno : ");Serial.print(nvev);Serial.print("-");Serial.println(nkom); } 
  lcd.clear();
  
  //delay(10000);
}

void vypisTeplot(){
  char cvev[5] = "";
  char cpla[5] = "";
  char ckom[4] = "";
  char buf[16];
  dtostrf(vevn,5,2,cvev);
  dtostrf(pla,5,2,cpla);
  dtostrf(kom,4,1,ckom);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("V:");
  lcd.print(cvev);
  lcd.print(" P:");
  lcd.print(cpla);
  
  lcd.setCursor(0,1);
  lcd.print("K:");
  lcd.print(ckom);

  lcd.print("N:");
  lcd.print(nvev);
  lcd.print("-");
  lcd.print(nkom);
  
  
  Serial.print (buf);
}

void pozapinejCoTreba(){
  //zmer teploty
  sensors.requestTemperatures(); 
  if (KOMENTATOR) { Serial.println("Teploty z cidel ( vevnitr, plast, komin ) : "); }
  vevn = sensors.getTempC(D_VEVNITR);  if (KOMENTATOR) { Serial.println(vevn); }
  delay(10);
  pla = sensors.getTempC(D_PLAST);  if (KOMENTATOR) { Serial.println(pla); }
  delay(10);
  kom = sensors.getTempC(D_KOMIN);   if (KOMENTATOR) { Serial.println(kom); }
  delay(10); 
  //topne faze
  if ((millis() - psepf) > PROSTOJFAZE) {
    if (vevn < nvev) { digitalWrite (F1, HIGH); psepf=millis(); if (KOMENTATOR) { Serial.println("Zapnuta faze 1");} }
     else { 
      if (vevn > nvev) { digitalWrite (F1, LOW); psepf=millis();  if (KOMENTATOR) { Serial.println("Vypnuta faze 1"); }}
     }

    if (vevn < nvev - DELTAT * 2) { digitalWrite (F2, HIGH); if (KOMENTATOR) { Serial.println("Zapnuta faze 2");} }
     else {digitalWrite (F2, LOW);  if (KOMENTATOR) { Serial.println("Vypnuta faze 2");} }
    if (vevn < nvev - DELTAT * 3) { digitalWrite (F3, HIGH); if (KOMENTATOR) { Serial.println("Zapnuta faze 3");}}
     else {digitalWrite (F3, LOW); if (KOMENTATOR) { Serial.println("Vypnuta faze 3");} }
  }

  //cerpadlo
  if ((millis() - psepk) > PROSTOJKOMINA) {
    Serial.println("XXXXXX");Serial.println(kom);Serial.println(nkom);Serial.println("XXXXXX");
    if (kom > nkom ) { digitalWrite (CHLAD, HIGH); psepk=millis(); if (KOMENTATOR) { Serial.println("Zapnuto cerpadlo. ");} }
    else {digitalWrite (F1, LOW); psepk=millis();  if (KOMENTATOR) { Serial.println("Vypnuto cerpadlo. ");}}
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
    if (KOMENTATOR) { Serial.println("Nastavovaci rezim "); } 
    pozapinejCoTreba();
    dbg = true;
    tnvev = round(analogRead(PTC1) / AKOEF);
    tnkom = round(analogRead(PTC2) / AKOEF);
    char buf[16] = "                " ;
    sprintf(buf, "Vevnitr: %d",tnvev);
    lcdprint(0,buf);
    char buf2[16] = "                " ;
    sprintf(buf2,"K: %d",tnkom);
    lcdprint(1,buf2);
    delay ( 100 );
  }

  
  if ( (millis()- ts > 1000) and (dbg) ){
    nvev = tnvev;
    nkom = tnkom;
     if (KOMENTATOR) { Serial.print("Nove hodnoty prijaty : ");Serial.print(nvev);Serial.print("-");Serial.println(nkom); dbg = false; }
  }else{
    if (KOMENTATOR and dbg) { Serial.println("Hodnoty ignorovany, protoze nastavovaci rezim byl prilis kratky."); dbg = false;}
  }
  
  
    
  delay(1000); 
  
}
