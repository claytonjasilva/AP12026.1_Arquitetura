#include <Keypad.h>

/*
12345 Maria Luiza Vicente Sinésio - TA
23456 Henrique Lucena Mundy  - TA
34567 Lucas De Carvalho Vilela - TA 
*/


int segPins[7] = {22, 23, 24, 25, 26, 27, 28}; //definição de pinos

#define TRIG   40
#define ECHO   41
#define LED1   42
#define LED2   43 //cada um de form especifica
#define LED3   44
#define BUZZER 45


int PC = 0;
byte IR = 0;
int ACC = 0;
bool FLAG_Z = false;
bool EXECUTANDO = false;

int MEM[16]; //seguindo o que foi pedido no trabalho do mem 16

struct Instrucao {
  byte opcode;
  int operando;
};

Instrucao programa[16];
int numInstrucoes = 0;

const char* MNEMONICOS[] = {
  "NOP","READ","LOADK","ADDK","SUBK","CMPK",
  "LEDON","LEDOFF","BUZON","BUZOFF","DISP",
  "NOP2","NOP3","STORE","LOADM","HALT"
};

const bool TEM_OPERANDO[] = {
  false,false,true,true,true,true,
  true,true,false,false,false,
  false,false,true,true,false
};


byte SEG_DIGITOS[10][7] = { //teclado
  {1,1,1,1,1,1,0},
  {0,1,1,0,0,0,0},
  {1,1,0,1,1,0,1},
  {1,1,1,1,0,0,1},
  {0,1,1,0,0,1,1},
  {1,0,1,1,0,1,1},
  {1,0,1,1,1,1,1},
  {1,1,1,0,0,0,0},
  {1,1,1,1,1,1,1},
  {1,1,1,1,0,1,1}
};

byte SEG_VAZIO[7] = {0,0,0,0,0,0,0};

void escreverSegmentos(byte p[7]) {
  for (int i = 0; i < 7; i++)
    digitalWrite(segPins[i], p[i]);
}

void mostrarNoDisplay(int valor) {
  if (valor >= 0 && valor <= 9)
    escreverSegmentos(SEG_DIGITOS[valor]);
}


void setup() {
  Serial.begin(9600);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  for (int i = 0; i < 7; i++)
    pinMode(segPins[i], OUTPUT);

  inicializarSistema();

  Serial.println("===============");
  Serial.println("Formato: opcode operando"); //SERIAL
  Serial.println("Ex: 2 5 (LOADK 5)");
  Serial.println("Digite RUN para executar");
}


void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "RUN") {
      EXECUTANDO = true;
      PC = 0;
      Serial.println(">>> EXECUTANDO");
    } else if (cmd == "*") {
      executarCiclo();
    } else {
      armazenarInstrucao(cmd);
    }
  }
}


void armazenarInstrucao(String cmd) {
  if (numInstrucoes >= 16) {
    Serial.println("Memoria cheia");
    return;
  }

  int esp = cmd.indexOf(' ');
  byte op = (esp == -1) ? cmd.toInt() : cmd.substring(0, esp).toInt();
  int oper = (esp == -1) ? 0 : cmd.substring(esp + 1).toInt();

  programa[numInstrucoes++] = {op, oper};

  Serial.print("[OK] ");
  Serial.println(instrucaoParaMnemonico(op, oper));
}


void executarCiclo() {
  if (!EXECUTANDO) return;

  if (PC >= numInstrucoes) {
    Serial.println("[FIM]");
    EXECUTANDO = false;
    return;
  }

  IR = programa[PC].opcode;
  int oper = programa[PC].operando;

  
  switch (IR) { //IR - Registrador de instrução

    case 0: break;

    case 1: // READ (via Serial)
      Serial.println("Digite valor:");
      while (!Serial.available());
      ACC = Serial.parseInt();
      break;

    case 2: ACC = oper; break;
    case 3: ACC += oper; break;
    case 4: ACC -= oper; break;
    case 5: FLAG_Z = (ACC == oper); break;

    case 6: ligarLED(oper); break;
    case 7: desligarLED(oper); break;

    case 8: tone(BUZZER, 1000); break;
    case 9: noTone(BUZZER); break;

    case 10: mostrarNoDisplay(ACC); break;

    case 13: MEM[oper] = ACC; break;
    case 14: ACC = MEM[oper]; break;

    case 15:
      EXECUTANDO = false;
      Serial.println("HALT");
      break;
  }

  Serial.print("PC: "); Serial.print(PC);
  Serial.print(" ACC: "); Serial.println(ACC);

  if (IR != 15) PC++;
}


void ligarLED(int n) {
  if (n == 1) digitalWrite(LED1, HIGH);
  if (n == 2) digitalWrite(LED2, HIGH);
  if (n == 3) digitalWrite(LED3, HIGH);
}

void desligarLED(int n) {
  if (n == 1) digitalWrite(LED1, LOW);
  if (n == 2) digitalWrite(LED2, LOW);
  if (n == 3) digitalWrite(LED3, LOW);
}


void inicializarSistema() {
  PC = 0; ACC = 0; FLAG_Z = false;
  EXECUTANDO = false; numInstrucoes = 0;

  for (int i = 0; i < 16; i++) {
    MEM[i] = 0;
    programa[i] = {0,0};
  }

  escreverSegmentos(SEG_VAZIO);
}

String instrucaoParaMnemonico(byte op, int operando) {
  String s = MNEMONICOS[op];
  if (TEM_OPERANDO[op]) {
    s += " ";
    s += operando;
  }
  return s;
}
