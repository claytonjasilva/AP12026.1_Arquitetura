#include <Keypad.h>

/* MARIA LUIZA VICENTE SINESIO 
HENRIQUE MUNDY 
LUCAS DE CARVALHO V.
*/

int segPins[7] = {22, 23, 24, 25, 26, 27, 28};

#define LED1   42
#define LED2   43
#define LED3   44
#define BUZZER 45

// ================== TECLADO ==================
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {32, 33, 34, 35};
byte colPins[COLS] = {36, 37, 38, 39};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ================== CPU ==================
int PC = 0;
byte IR = 0;
int ACC = 0;
bool FLAG_Z = false;
bool EXECUTANDO = false;

int MEM[16];

struct Instrucao {
  byte opcode;
  int operando;
};

Instrucao programa[16];
int numInstrucoes = 0;

String buffer = "";

// ================== MNEMONICOS ==================
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

// ================== DISPLAY ==================
byte SEG_DIGITOS[10][7] = {
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

void escreverSegmentos(byte p[7]) {
  for (int i = 0; i < 7; i++)
    digitalWrite(segPins[i], p[i]);
}

void mostrarNoDisplay(int valor) {
  if (valor >= 0 && valor <= 9)
    escreverSegmentos(SEG_DIGITOS[valor]);
}

// ================== SETUP ==================
void setup() {
  Serial.begin(9600);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  for (int i = 0; i < 7; i++)
    pinMode(segPins[i], OUTPUT);

  Serial.println("=== CPU DIDATICA COM TECLADO ===");
  Serial.println("Digite instrucoes:");
  Serial.println("Ex: 2A5D (LOADK 5)");
  Serial.println("D = confirmar | * = executar | # = reset");
}

// ================== LOOP ==================
void loop() {
  char tecla = keypad.getKey();

  if (tecla) {
    Serial.print("Tecla: ");
    Serial.println(tecla);

    processarEntrada(tecla);
  }
}

// ================== ENTRADA ==================
void processarEntrada(char tecla) {

  if (tecla == '#') {
    limparMemoria();
    Serial.println(">>> RESET");
    return;
  }

  if (tecla == '*') {
    executarCiclo();
    return;
  }

  if (tecla == 'D') {
    buffer.trim();
    if (buffer.length() > 0) {
      armazenarInstrucao(buffer);
      buffer = "";
    }
    return;
  }

  // A = espaço
  if (tecla == 'A') buffer += ' ';
  else buffer += tecla;
}

// ================== LOAD ==================
void armazenarInstrucao(String cmd) {

  if (cmd == "RUN") {
    EXECUTANDO = true;
    PC = 0;
    Serial.println(">>> EXECUTANDO");
    return;
  }

  if (numInstrucoes >= 16) {
    Serial.println("Memoria cheia");
    return;
  }

  int esp = cmd.indexOf(' ');
  byte op = (esp == -1) ? cmd.toInt() : cmd.substring(0, esp).toInt();
  int oper = (esp == -1) ? 0 : cmd.substring(esp + 1).toInt();

  programa[numInstrucoes++] = {op, oper};

  Serial.print("[LOAD] ");
  Serial.println(instrucaoParaMnemonico(op, oper));
}

// ================== EXECUÇÃO ==================
void executarCiclo() {

  if (!EXECUTANDO) {
    Serial.println("Nao esta em RUN");
    return;
  }

  if (PC >= numInstrucoes) {
    Serial.println("[FIM]");
    EXECUTANDO = false;
    return;
  }

  IR = programa[PC].opcode;
  int oper = programa[PC].operando;

  switch (IR) {

    case 0: break;

    case 1:
      Serial.println("Digite valor no Serial:");
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

  Serial.print("PC: ");
  Serial.print(PC);
  Serial.print(" ACC: ");
  Serial.println(ACC);

  if (IR != 15) PC++;
}

// ================== HARDWARE ==================
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

// ================== AUX ==================
void limparMemoria() {
  PC = 0;
  EXECUTANDO = false;
  numInstrucoes = 0;

  for (int i = 0; i < 16; i++) {
    MEM[i] = 0;
    programa[i] = {0,0};
  }
}

String instrucaoParaMnemonico(byte op, int operando) {
  String s = MNEMONICOS[op];
  if (TEM_OPERANDO[op]) {
    s += " ";
    s += operando;
  }
  return s;
}
