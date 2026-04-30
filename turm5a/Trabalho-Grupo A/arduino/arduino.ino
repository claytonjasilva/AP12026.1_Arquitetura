#include <Keypad.h>

/* MARIA LUIZA VICENTE SINESIO 
HENRIQUE MUNDY 
LUCAS DE CARVALHO V.
*/

// ===== TECLADO =====
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {30,31,32,33};
byte colPins[COLS] = {34,35,36,37};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

#define TRIG 40
#define ECHO 41
#define LED1 42
#define LED2 43
#define LED3 44
#define BUZZER 45

int PC = 0;
int ACC = 0;
bool FLAG_Z = false;
bool EXECUTANDO = false;

struct Instrucao {
  byte op;
  int arg;
};

Instrucao prog[16];
int tam = 0;

int MEM[16];

enum {
  HALT, READ, LOADK, ADDK, SUBK, CMPK,
  LEDON, LEDOFF, BUZON, BUZOFF,
  DISP, ALERT,
  BINC, STORE, LOADM,
  NOP,
  BZ, JMP
};

const char* NOME[] = {
  "HALT","READ","LOADK","ADDK","SUBK","CMPK",
  "LEDON","LEDOFF","BUZON","BUZOFF",
  "DISP","ALERT",
  "BINC","STORE","LOADM",
  "NOP","BZ","JMP"
};


int lerSensor(){
  digitalWrite(TRIG,LOW); delayMicroseconds(2);
  digitalWrite(TRIG,HIGH); delayMicroseconds(10);
  digitalWrite(TRIG,LOW);

  long d = pulseIn(ECHO,HIGH,10000);
  if(d==0) return 15;

  return d*0.034/2;
}


void ciclo(){

  if(!EXECUTANDO) return;

  if(PC >= tam){
    EXECUTANDO = false;
    Serial.println("FIM");
    return;
  }

  byte op = prog[PC].op;
  int arg = prog[PC].arg;

  switch(op){

    case HALT: EXECUTANDO=false; break;

    case READ: ACC=lerSensor(); break;

    case LOADK: ACC=arg; break;

    case ADDK: ACC+=arg; break;

    case SUBK: ACC-=arg; break;

    case CMPK: FLAG_Z=(ACC==arg); break;

    case LEDON:
      if(arg==1) digitalWrite(LED1,HIGH);
      if(arg==2) digitalWrite(LED2,HIGH);
      if(arg==3) digitalWrite(LED3,HIGH);
      break;

    case LEDOFF:
      if(arg==1) digitalWrite(LED1,LOW);
      if(arg==2) digitalWrite(LED2,LOW);
      if(arg==3) digitalWrite(LED3,LOW);
      break;

    case BUZON: tone(BUZZER,1000); break;

    case BUZOFF: noTone(BUZZER); break;

    case DISP:
      Serial.print("ACC=");
      Serial.println(ACC);
      break;

    case ALERT:
      ACC = lerSensor();
      if(ACC < 10) tone(BUZZER,2000);
      break;

    case BINC: if(arg<16) MEM[arg]++; break;

    case STORE: if(arg<16) MEM[arg]=ACC; break;

    case LOADM: if(arg<16) ACC=MEM[arg]; break;

    case BZ:
      if(FLAG_Z){ PC=arg; return; }
      break;

    case JMP:
      PC=arg; return;

    case NOP: break;
  }

  Serial.print("PC:");
  Serial.print(PC);
  Serial.print(" ");
  Serial.print(NOME[op]);
  Serial.print(" ACC=");
  Serial.println(ACC);

  PC++;
}


String buffer = "";
bool modoLOAD = true;

unsigned long lastKeyTime = 0;

void setup(){

  Serial.begin(9600);

  pinMode(TRIG,OUTPUT);
  pinMode(ECHO,INPUT);

  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(LED3,OUTPUT);

  pinMode(BUZZER,OUTPUT);

  for(int i=0;i<16;i++) MEM[i]=0;

  Serial.println("MODO LOAD");
}

// ===== LOOP =====
void loop(){

  char t = keypad.getKey();

  // debounce forte
  if(t && millis() - lastKeyTime < 200) return;
  if(t) lastKeyTime = millis();

  if(!t) return;


  if(t=='C'){
    modoLOAD=true;
    EXECUTANDO=false;
    PC=0;
    tam=0;
    buffer="";
    Serial.println("RESET");
    return;
  }

 
  if(modoLOAD){

    // entrar em RUN
    if(t=='D'){
      if(tam == 0){
        Serial.println("SEM PROGRAMA");
        return;
      }

      modoLOAD=false;
      EXECUTANDO=true;
      PC=0;
      Serial.println("RUN");
      return;
    }

    // confirmar instrução
    if(t=='#'){

      int esp = buffer.indexOf(' ');
      int op = (esp==-1)? buffer.toInt() : buffer.substring(0,esp).toInt();
      int arg = (esp==-1)? 0 : buffer.substring(esp+1).toInt();

      if(op >= 0 && op < 18 && tam < 16){
        prog[tam++] = {(byte)op,arg};
        Serial.println(NOME[op]);
      } else {
        Serial.println("ERRO");
      }

      buffer="";
      return;
    }

    // montar instrução
    if(t=='A') buffer+=' ';
    else buffer+=t;

    Serial.print(t);
  }

  else{

    if(t=='*') ciclo();
  }
}
