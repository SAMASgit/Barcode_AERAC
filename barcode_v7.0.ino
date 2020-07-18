//è il codice della v7 ma aggiustato e puluto da note e commenti inutili 
//devo ricordarmi che il serial print aggiunge un bel po' di delay 
#include "MotoreStep.h"

char ASCII[]={' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','[',' ',']','^','_','\'','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',' ',' ',' ',' ',' '};

#define clock_pin A0
#define sensor_pin A1
#define led_pin 11

#define motor_timeout 5        //ogni quanto devo ripetere il movimento del motore
#define lettura_timeout 400    //ogni quanto devo ripetere la lettura

//pin per il brushed
//#define motor_pin 10 

//pin per lo stepper
#define STEPPER_PIN_1 4
#define STEPPER_PIN_2 5
#define STEPPER_PIN_3 6
#define STEPPER_PIN_4 7

const bool debug=false;

int sensor_threshold ;  //soglia sensore, se la resistenza indica un valore sotto la soglia vuol dire che ha letto 1
int clock_threshold ;   //soglia clock, se la resistenza indica un valore sotto la soglia vuol dire che ha letto 1

int _bit ;
int _byte[7];
int index;


long last_motor=0;
long last_lettura=0;
bool ero_sul_bianco = true;



void setup() {


//pin per lo stepper
pinMode(STEPPER_PIN_1, OUTPUT);
pinMode(STEPPER_PIN_2, OUTPUT);
pinMode(STEPPER_PIN_3, OUTPUT);
pinMode(STEPPER_PIN_4, OUTPUT);
  
pinMode(led_pin, OUTPUT);
analogWrite(led_pin, 850);         //uso l'analog write così posso tenere acceso il led per tanto tempo senza rischiare di bruciarlo pure senza resistenza
  
pinMode(sensor_pin,INPUT);
pinMode(clock_pin, INPUT);
Serial.begin(9600);

set_thresholds();

}



void loop() {
  run();
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////  METODI PRINCIPALI  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void run(){                                                                                                                    //ciclo principale
  if(debug)Serial.println("sono nel run");
  if(delta_t_motore())scorri();                                                                                                //muovo il motore ogni "delta t motore o qualcosa in più" milllisecondi
  if(delta_t_lettura())leggi_sensori();                                                                                        //leggo i sensori ogni "delta t lettura o qualcosa in più" millisecondi
}


void leggi_sensori(){
   if(debug)Serial.println("sono in leggi_sensori");
   if(read_clock_pin()==0)ero_sul_bianco=true;                                                                                 //appena non vedo più traccia sul clock vuol dire che sono tornato sulla parte di nastro bianca, quindi rimetto s true la variabile visto che al prossimo ciclo è vero che "ero sul bianco"
   if((read_clock_pin()==1) && ero_sul_bianco){                                                                                //se trovo il segnale del clock e al ciclo prima ero sul bianco eseguo il codice principale, "ero_sul_bianco" parte da true e viene messo su false ogni volta che viene trovata ed elaborata una traccia del clock
      if(debug)Serial.println("clock trovato");
      _bit = read_sensor_pin();                                                                                                //leggo la reistenza del sensore per vedere se il bit che ho trovato è uno 0 o un 1
      _byte[index]=_bit;                                                                                                       //dopo aver capito se è 0 o 1 lo scrivo nella cella del vettore index-esima  
      if(debug)Serial.println(_byte[index]);                                                                                   //se sono in debug stampo sulla seriale il valore che ho appena scritto nel vettore 
      index++;                                                                                                                 //aumento l'indice
      ero_sul_bianco=false;                                                                                                    //avendo appena letto il clock al prossimo ciclo non è vero che "ero sul bianco", quindi metto la variabile su falso   
      if(index==7){                                                                                                            //se l'indice vale 7 vuol dire che le celle 0123456 del vettore sono piene 
      Serial.print(ASCII[convert(_byte)]);                                                                                     //una volta che ho caricato il vettore _byte, converto il vettore in un intero e stampo su seriale la lettera corrispondente di ASCII
      if(debug) output_debug();                                                                                                //se sono in debug stampo in modo più visibile la lettera che ho trovato
      index=0;      
      }                                                                                                
   }
   last_lettura=millis();                                                                                                      //visto che ho appena fatto la lettura prendo il tempo in cui l'ho fatta per calcolare i successivi delta_t                  
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




void scorri(){                                                                                                                 //metodo per muovere di uno step il motore stepper e aggiornare il tempo dell'ultimo movimento
  if(debug)Serial.println("sono in scorri");
  OneStep(true, STEPPER_PIN_1, STEPPER_PIN_2, STEPPER_PIN_3, STEPPER_PIN_4);                                                   //l'argomento di one step è la direzione di rotazione, per il resto del codice del motore controllare il file MotoreStep.h
  last_motor=millis();                                                                                                         //aggiorno il momento dell'ultimo movimento del motore
}



void set_thresholds(){                                                                                                         //questa versione del set_thresholds è pensata per avere unico loop ovvero il while true da poter usare anche per altro volendo
  bool stampato_c0=false;
  bool stampato_c1=false;
  bool stampato_s0=false;
  bool stampato_s1=false;
  bool attesa=true;
  int letture=0;
  int c0=0;
  int c1=0;
  int s0=0;
  int s1=0;
  bool finito_c0=false;
  bool finito_c1=false;
  bool finito_clock=false;
  bool finito_s0=false;
  bool finito_s1=false;
  bool finito_sensore=false;
  bool finito=false;
  while(!finito){    //in questo caso non è necessario avere il while(true) tanto l'unica condizione per rimanere nel loop è che non devo aver ancora finito
    setta_soglie(attesa, letture, stampato_c0, c0, finito_c0, stampato_c1, c1, finito_c1, stampato_s0, s0, finito_s0, stampato_s1, s1, finito_s1, finito_clock, finito_sensore, finito);    
    if(delta_t_motore()){scorri();}
  }
  if(debug)Serial.println("agg finit tutt cumba mocc a mammt");
  clock_threshold=((c0+c1)/2);
  sensor_threshold=((s0+s1)/2);
  if(debug)Serial.print("come soglia del clock ho impostato: ");
  if(debug)Serial.println(clock_threshold);
  if(debug)Serial.print("come soglia del sensore ho impostato: ");
  if(debug)Serial.println(sensor_threshold);
  wait_ready();
}



//setta_soglie è il cuore del metodo set_thresholds, set threshold serve solo a creare le condizioni per eseguire setta_soglie nel modo giusto, questo metodo è pensato per essere eseguito ciclicamente "attivando" e "disattivando" pezzi di codice a ogni ciclo
void setta_soglie(bool& attesa, int& letture, bool& stampato_c0, int& c0, bool& finito_c0, bool& stampato_c1, int& c1, bool& finito_c1, bool& stampato_s0, int& s0, bool& finito_s0,  bool& stampato_s1, int& s1, bool& finito_s1, bool& finito_clock, bool& finito_sensore, bool& finito){
    set_c0(attesa, stampato_c0, letture, c0, finito_c0);
    if(finito_c0)set_c1(attesa, stampato_c1, letture, c1, finito_c1);
    finito_clock=finito_c0 && finito_c1;
    if(finito_clock)set_s0(attesa, stampato_s0, letture, s0, finito_s0);
    if(finito_clock && finito_s0)set_s1(attesa, stampato_s1, letture, s1, finito_s1);
    finito_sensore=finito_s0 && finito_s1;
    finito = finito_clock && finito_sensore;
}



void set_c0(bool& attesa, bool& stampato_c0, int& letture, int& c0, bool& finito_c0){
  if(letture == 10)letture=0; //questo mi serve perchè se ho appena finito di leggere un sensore mi troverò letture a 10 quindi lo devo riportare prima a 0 (oppure in alternativa dovrei partire da 10 e fare letture fino a 20 e così via) 
  if(attesa && !stampato_c0){
    Serial.println("posizionare il nastro su una parte di nastro c0");
    stampato_c0=true;
  }
  if(Serial.available()!=0)attesa=false;
  if(!attesa && !finito_c0){
    c0+=analogRead(clock_pin);
    letture++;
    if (letture==10){
      if(debug)Serial.println("ho fatto la lettura we ");
      serialFlush();
      finito_c0=true;
      attesa=true;
      c0=c0/10;
      if(debug)Serial.print("come c0 ho settato: ");
      if(debug)Serial.println(c0);
    }
  }
}



void set_c1(bool& attesa, bool& stampato_c1, int& letture, int& c1, bool& finito_c1){
  if(letture == 10)letture=0; //questo mi serve perchè se ho appena finito di leggere un sensore mi troverò letture a 10 quindi lo devo riportare prima a 0 (oppure in alternativa dovrei partire da 10 e fare letture fino a 20 e così via)  
  if(attesa && !stampato_c1){
    Serial.println("posizionare il nastro su una parte di nastro c1");
    stampato_c1=true;
  }
  if(Serial.available()!=0)attesa=false;
  if(!attesa && !finito_c1){
    c1+=analogRead(clock_pin);
    letture++;
    if (letture==10){
      if(debug)Serial.println("ho fatto la lettura we ");
      serialFlush();
      finito_c1=true;
      attesa=true;
      c1=c1/10;
      if(debug)Serial.print("come c1 ho settato: ");
      if(debug)Serial.println(c1);
    }
  }
}



void set_s0(bool& attesa, bool& stampato_s0, int& letture, int& s0, bool& finito_s0){
  if(letture == 10)letture=0; //questo mi serve perchè se ho appena finito di leggere un sensore mi troverò letture a 10 quindi lo devo riportare prima a 0 (oppure in alternativa dovrei partire da 10 e fare letture fino a 20 e così via)  
  if(attesa && !stampato_s0){
    Serial.println("posizionare il nastro su una parte di nastro s0");
    stampato_s0=true;
  }
  if(Serial.available()!=0)attesa=false;
  if(!attesa && !finito_s0){
    s0+=analogRead(sensor_pin);
    letture++;
    if (letture==10){
      if(debug)Serial.println("ho fatto la lettura we ");
      serialFlush();
      finito_s0=true;
      attesa=true;
      s0=s0/10;
      if(debug)Serial.print("come s1 ho settato: ");
      if(debug)Serial.println(s0);
    }
  }
}



void set_s1(bool& attesa, bool& stampato_s1, int& letture, int& s1, bool& finito_s1){
  if(letture == 10)letture=0; //questo mi serve perchè se ho appena finito di leggere un sensore mi troverò letture a 10 quindi lo devo riportare prima a 0 (oppure in alternativa dovrei partire da 10 e fare letture fino a 20 e così via)  
  if(attesa && !stampato_s1){
    Serial.println("posizionare il nastro su una parte di nastro s1");
    stampato_s1=true;
  }
  if(Serial.available()!=0)attesa=false;
  if(!attesa && !finito_s1){
    s1+=analogRead(sensor_pin);
    letture++;
    if (letture==10){
      if(debug)Serial.println("ho fatto la lettura we ");
      serialFlush();
      finito_s1=true;
      attesa=true;
      s1=s1/10;
      if(debug)Serial.print("come s1 ho settato: ");
      if(debug)Serial.println(s1);
    }
  }
}



bool delta_t_motore(){                  //metodo che restituisce true quando passano "motor_timeout" o più millisecondi 
  long t=millis();
  if(debug)Serial.print("sono in delta t motore e deltat vale: ");
  if(debug)Serial.println(t-last_motor);
  if ((t-last_motor) >= motor_timeout)return true;
  else if((t-last_motor)<motor_timeout) return false; 
}



bool delta_t_lettura(){                 //metodo che restituisce true quando passano "lettura_timeout" o più millisecondi 
  long t=millis();
  if(debug)Serial.print("sono in delta t lettura e deltat vale: ");
  if(debug)Serial.println(t-last_lettura);
  if ((t-last_lettura) >= lettura_timeout)return true;
  else if ((t-last_lettura)<lettura_timeout) return false; 
}



void output_debug(){            //questo metodo serve solo stampare in modo più visibile su seriale il vettore di bit che ho trovato e il risultato della conversione, con un delay alla fine per leggerli meglio senza che scorrano subito fuori dalla schermata
  Serial.println(" ");
  print_byte();

  Serial.println(" ");
  Serial.print("la conversione del vettore vale: ");
  Serial.println(convert(_byte));
  
  
  Serial.println("------ carattere trovato ----");
  Serial.println(ASCII[convert(_byte)]);
  Serial.println("---------------------------");
  delay(5000); 
}



void print_byte(){                           //stampa il vettore _byte[] in seriale
  for(int k=0;k<7;k++){
    Serial.print(_byte[k]);
  }
}



int convert(int v[]){                        //prende in input un vettore tipo 1100011 che rappresenta un numero binario e lo converte in un intero normale
  int n=0;
  for (int k=0;k<7;k++){
    n+=power_2(k)*v[6-k];
  }
  return n;
}



int read_sensor_pin(){                       //legge il valore del sensore ma restituisce solo 1 o 0 confrontandolo con la sua soglia
  int a=analogRead(sensor_pin);
  if(debug)Serial.print("dal read sensor ho letto ");
  if(debug)Serial.println(a);
  if(a<sensor_threshold)return 1;
  else if(a>=sensor_threshold) return 0;
}



int read_clock_pin(){                       //legge il valore del clock ma restituisce solo 1 o 0 confrontandolo con la sua soglia
  int a=analogRead(clock_pin);
  if(debug)Serial.print("dal read clock ho letto ");
  if(debug)Serial.println(a);
  if(a<clock_threshold)return 1;
  else if(a>=clock_threshold) return 0;
}



void wait_ready(){                            //questo metodo chiede solo di rimettersi all'inizio del nastro e rimane in attesa dell'invio via seriale, uso un altro ciclo così posso mettere in pausa il motore tanto le letture le ho già fatte
  serialFlush();
  Serial.println("posizionare il nastro all'inizio poi premere invio");
  while(Serial.available()==0){
    delay(5);
  }
}



void serialFlush(){                          //metodo che serve per ripulire il buffer della seriale
  while(Serial.available() > 0) {
    char t = Serial.read();
  }
}



int power_2(int esponente){                  //metodo che calcola 2^(esponente), uso questo al posto di pow(2, esponente) perchè la funione pow(a, b) mi restituisce valori sbagliati
  int n=1;
  if(esponente == 0) return 1;
  else if(esponente != 0) {
    for (int i=0; i<esponente; i++){
      n=2*n;
    }
    return n;
  }
}
