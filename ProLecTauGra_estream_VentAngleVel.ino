
//ProLecTauGra_estream_VentAngleVel.ide
// programa que llegeix les linies d'un fitxer de log de B&G (NMEA) i extreu les següents dades:
// TWS de les linies WIMWV (CAMP 3 si 2 es T)
// TWA de les linies WIMWV (CAMP 1 si 2 es R)
// SOG de les linies GPRMC (CAMP 7 si 2 es igual a A)
// i les grava en en una taula, les files son el vent TWS( de 4 a 20 nusos) i les columnes son
// l'ange de vent aparent TWA( de 30 a 330) Les dades de la taula son la freqüencia freq, la mitjana m, la valiança S, la desv est sigma i l'error er 

/*
 Lectura d'un fitxer
Connexions de la SD
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 3 (for MKRZero SD: SDCARD_SS_PIN)
*/

#include <SD.h>

File myFile;
String nomFitch="Mo150426.txt"; // el nom del fitxer sols pot tenir 8 lletres


//variables del missatge NMEA
String linia="";// linia llegida
String men="";//Codi del missatge NMEA

//variables per la selecció dels camps
int coma[15];   //posició de les comes de separació de les dades
String Cam [15]; // camps de dades del missatge NMEA

//Variables de sortida
float SOG;
float SOGant;
float TWS;
float TWA;
int frequencia;
float media;
int mediaInt;
float delta;
float variancia ;
int varianciaInt;
float error;
int velInt;
float vel;
float sigma;



 // variables de la taula
int freq[496]; //taula de freqüencies
int m[496];
int S[496]; // taula de variancia
int sig[496];// taula de desviació estàndard


void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("Initializing SD card...");

  if (!SD.begin(3)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  if (!SD.exists(nomFitch)){
     Serial.println("El fitxer no existeix");
     while (1);
  }
}


void loop() {
// obre el fitxer per llegir
myFile = SD.open(nomFitch,FILE_READ);

// read from the file until there's nothing else in it:
while (myFile.available()) {
  linia = myFile.readStringUntil('\n');
  men = linia.substring(1,6);
// Serial.println(men);

if ((linia.length() > 20 ) && ( men=="GPRMC")){
//    Serial.println(linia);
    separacamps();
     if (Cam[2] == "A"){;
      SOGant = SOG;
      SOG = Cam[7].toFloat();
 //     Serial.println(SOG);
      }
  }   
// LECTURA DE TWS
  if ((linia.length() > 20 ) && ( men=="WIMWV")){  // separació dels camps amb les comes
//    Serial.println(linia); // imprimeix la linia
    separacamps();
    if (Cam[2] == "R"){;
      TWA = Cam[1].toFloat(); 
//      Serial.println(TWA);
      }
    if (Cam[2] == "T"){;
      TWS = Cam[3].toFloat(); 
//     Serial.println(TWS);

      // filtres de velocitat
      if ( 2 > SOG < 11 ) {
        if ((SOG - SOGant) < 1 ){;
//          resventangle ();
          gravaTaula ();}
      }
      }      
  }    
}   

    // close the file:
    myFile.close();
    BolcaTaula();
    while(1);
}

// funció de separar els valors de la linia llegida

void separacamps () {
// lectura dels cams de la linia
  int ultimaComa = linia.lastIndexOf(',');
  int j = 0;
  for (int i=1; i < 15; i++) {
    coma[i] = linia.indexOf(',',coma[i-1]+1);
    j++;
    if (coma[i]+1 > ultimaComa){
      break;
    }
  } 
  for (int n=1; n < j; n++) {
    Cam[n] = linia.substring(coma[n]+1,coma[n+1]);
  }
}
 
 // funció d'imprimir resultat vent - angle

/*
void resventangle () {
// Imprimeix resultats per un vent - angle
Serial.println("x");
Serial.println("x");
Serial.println("x");
Serial.print( " xxxxx ");
Serial.print("TWS ");
Serial.print(TWS);
Serial.print(" TWA ");
Serial.print(TWA);
Serial.print("  SOG ");
Serial.print(SOG),
Serial.println ( " xxxxx ");
Serial.println("x");
Serial.println("x");
Serial.println("x");

}
*/

void gravaTaula() {

int psv;  // part sencera mes pròxima del vent
float margev = 0.2 ;
float margeInf;
float margeSup;
int index1; //primer element del index de la taula fila
int Dve; // Desena del vent
int index2; 
int index;
// int velInt;
// float vel;
// int frequencia;

psv = TWS + 0.5 ;
margeSup = psv + margev;
margeInf = psv - margev;

// limits i marges del vent
if (TWS >= 4 && TWS <= 20){
  if ((TWS <= margeSup) && (TWS >= margeInf )){
    index1 = psv - 4;
    Serial.println("index1 -------------------");
    Serial.println(index1);
    if ((TWA > 30) && (TWA <= 330)){
     Dve = TWA/10;
     if ((TWA >= Dve * 10) && (TWA <= ((Dve + 1) * 10))){
      index2 = Dve - 2;
      Serial.println("index2 -------------------");
      Serial.println(index2);
     }
    }
index = index1 * 31 + index2;

// VALORS ANTERIORS
frequencia = freq[index];
mediaInt = m[index] ;
media = mediaInt/ 100;
delta = SOG - media ;
varianciaInt = S[index] ;
variancia = varianciaInt / 100;

//CALCULA I GRABA ELS NOUS VALORS
freq[index] += 1;
frequencia = freq[index];

m[index] = 100 * (media + delta / freq[index]) ;
vel = m[index]/100;

S[index] = 100 * ((variancia + (delta * delta)));
variancia = (S[index]/100)/(freq[index]);

sig[index] = 100 * (sqrt((variancia/(freq[index]-1))));
sigma = sig[index]/100;

/*
Serial.println("valors: vel frequencia variancia sigma");
Serial.println(vel);
Serial.println(frequencia);
Serial.println(variancia);
Serial.println(sigma);
Serial.println("===============================================");
*/
  }
  }
 }

// *******************************************************
void BolcaTaula(){
  Serial.println(" ************* grava els valors**************************");

//float vel;
//int velInt;

//IMPRIMIR LA TAULA
  Serial.println(" Llista VENT ANGLE VELOCITAT ");
//  Serial.println(" vent    30       40       50       60       70       80       90       100      110      120      130      140      150      160      170      180      190    200     210     220     230     240     250     260     270     280     290     300     310     320     330");
  int k=0;
 for (int j=4 ; j<21 ;j++){
    // Serial.print("   "); 
    // Serial.print(j);
    // Serial.print("    "); 
      for (int i=k; i < k+31; i++) { 
         error = (sig[i]/sqrt(freq[i]))/100;
        if ( (error > 0) && ( error < 0.05)) {
          TWS = j;
          Serial.print( TWS );
          Serial.print(", ");
          TWA = (i-(j-4)*31)*10 + 30 ;
          Serial.print(TWA);
          Serial.print(", ");
          SOG = m[i];
          Serial.print(SOG/100);
          // Serial.print(", ");
          // Serial.print(error);
          Serial.println("");
        }
      }
      k += 31;
        Serial.println("");
   
      }
Serial.println("\n\n");



}







