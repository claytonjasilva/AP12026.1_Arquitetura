/*
202504164065 - Antonio Damasceno - TA
202507345321 - Luiz Fernando - TA
202504236708 - Giovanna Sales - TA
202308675435 - Guilherme Reis- TA
202308675427 - Matheus Reis- TA
*/

#include <Keypad.h>

// ==========================================
// 1. ELEMENTOS ARQUITETURAIS (OBRIGATÓRIOS)
// ==========================================
int MEM[16];           
int PC = 0;            
byte IR = 0;           
int ACC = 0;           
bool FLAG_Z = false;   
bool EXECUTANDO = false; 

String programa[16];   
int ponteiroCarga = 0; 
bool modoLoad = false;

// ==========================================
// 2. PINAGEM OBRIGATÓRIA (SEÇÃO 3.3)
// ==========================================
const byte pinDisplay[] = {22, 23, 24, 25, 26, 27, 28}; 
const byte pinTeclado[] = {30, 31, 32, 33, 34, 35, 36, 37}; 
#define PIN_TRIG 40
#define PIN_ECHO 41
#define PIN_LED1 42
#define PIN_LED2 43
#define PIN_LED3 44
#define PIN_BUZZER 45

char keys[4][4] = {{'1','2','3','A'},{'4','5','6','B'},{'7','8','9','C'},{'*','0','#','D'}};
byte rowPins[4] = {30, 31, 32, 33}; 
byte colPins[4] = {34, 35, 36, 37}; 
Keypad teclado = Keypad(makeKeymap(keys), rowPins, colPins, 4, 4);

// Matriz do Display (ANODO COMUM)
byte numMap[13][7] = {
  {0,0,0,0,0,0,1}, // 0
  {1,0,0,1,1,1,1}, // 1
  {0,0,1,0,0,1,0}, // 2
  {0,0,0,0,1,1,0}, // 3
  {1,0,0,1,1,0,0}, // 4
  {0,1,0,0,1,0,0}, // 5
  {0,1,0,0,0,0,0}, // 6
  {0,0,0,1,1,1,1}, // 7
  {0,0,0,0,0,0,0}, // 8
  {0,0,0,0,1,0,0}, // 9
  {0,1,1,0,0,0,0}, // E (Overflow)
  {1,1,1,1,1,1,0}, // - (Negativo)
  {1,1,1,1,1,1,1}  // APAGADO
};

void setup() {
  Serial.begin(9600);
  for(int i=0; i<7; i++) {
    pinMode(pinDisplay[i], OUTPUT);
    digitalWrite(pinDisplay[i], HIGH); 
  }
  pinMode(PIN_TRIG, OUTPUT); pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_LED1, OUTPUT); pinMode(PIN_LED2, OUTPUT);
  pinMode(PIN_LED3, OUTPUT); pinMode(PIN_BUZZER, OUTPUT);
  Serial.println("SISTEMA PRONTO (ANODO).");
}

// ==========================================
// 3. UNIDADE DE CONTROLE (UC) - CICLO DE INSTRUÇÃO
// ==========================================

void executarCiclo() {
  if (PC >= 16 || programa[PC] == "") {
    Serial.println("FIM DA MEMORIA DE PROGRAMA.");
    EXECUTANDO = false;
    return;
  }

  String instrucaoCompleta = programa[PC];
  int espaco = instrucaoCompleta.indexOf(' ');
  int opcodeDec = (espaco == -1) ? instrucaoCompleta.toInt() : instrucaoCompleta.substring(0, espaco).toInt();
  int operando = (espaco == -1) ? 0 : instrucaoCompleta.substring(espaco + 1).toInt();

  IR = (byte)opcodeDec; 
  processarInstrucao(IR, operando);
  exibirEstados();
  PC++; 
}

void processarInstrucao(byte opcode, int k) {
  switch(opcode) {
    case 0:  Serial.println("NOP"); break;
    case 1:  ACC = lerDistancia(); Serial.println("READ"); break;
    case 2:  ACC = k; Serial.println("LOADK"); break;
    case 3:  ACC += k; Serial.println("ADDK"); break; 
    case 4:  ACC -= k; Serial.println("SUBK"); break; 
    case 5:  FLAG_Z = (ACC == k); Serial.println("CMPK"); break; 
    case 6:  acionarLED(k, HIGH); break; 
    case 7:  acionarLED(k, LOW); break;
    case 8:  digitalWrite(PIN_BUZZER, HIGH); Serial.println("BUZZ ON"); break; 
    case 9:  digitalWrite(PIN_BUZZER, LOW); Serial.println("BUZZ OFF"); break;
    case 10: exibirNoDisplay(ACC); break; 
    case 11: executarAlert(); break; 
    case 12: Serial.print("BIN: "); Serial.println(opcode, BIN); break;
    case 13: if(k < 16) MEM[k] = ACC; break; 
    case 14: if(k < 16) ACC = MEM[k]; break; 
    case 15: EXECUTANDO = false; Serial.println("HALT"); break; 
    case 16: desligarDisplay(); Serial.println("DISPLAY OFF"); break;
  }
}

// ==========================================
// 4. FUNÇÕES DE APOIO
// ==========================================

void loop() {
  char tecla = teclado.getKey();
  if (!tecla) return;

  if (tecla == '#') {
    modoLoad = !modoLoad;
    if (modoLoad) {
      ponteiroCarga = 0; PC = 0; EXECUTANDO = false;
      for(int i=0; i<16; i++) programa[i] = "";
      Serial.println(">>> MODO LOAD ATIVADO.");
    } else {
      Serial.println(">>> MODO LOAD ENCERRADO.");
    }
  } 
  else if (tecla == 'A' && !modoLoad) { 
    PC = 0; EXECUTANDO = true;
    Serial.println(">>> MODO RUN INICIADO. '*' PARA STEP.");
  }
  else if (tecla == '*' && EXECUTANDO) {
    executarCiclo();
  }
  else if (modoLoad) {
    receberInstrucaoTeclado(tecla);
  }
}

void receberInstrucaoTeclado(char t) {
  static String buffer = "";
  if (t >= '0' && t <= '9' || t == 'B') { 
    if (t == 'B') buffer += " ";
    else buffer += t;
    Serial.print(t);
  } 
  else if (t == 'C') { 
    programa[ponteiroCarga] = buffer;
    Serial.println(" -> Salvo em MEM[" + String(ponteiroCarga) + "]");
    ponteiroCarga++;
    buffer = "";
  }
}

void exibirNoDisplay(int v) {
  int idx = v;
  if (v > 9) { idx = 10; } // E
  else if (v < 0) { idx = 11; } // -
  for(int i=0; i<7; i++) digitalWrite(pinDisplay[i], numMap[idx][i]);
}

void desligarDisplay() {
  for(int i=0; i<7; i++) digitalWrite(pinDisplay[i], numMap[12][i]);
}

int lerDistancia() {
  digitalWrite(PIN_TRIG, LOW); delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  long duracao = pulseIn(PIN_ECHO, HIGH);
  int d = duracao * 0.034 / 2;
  return (d > 0) ? d : 999; // Evita que erro de leitura vire 0
}

void acionarLED(int l, int st) {
  if(l == 1) digitalWrite(PIN_LED1, st);
  if(l == 2) digitalWrite(PIN_LED2, st);
  if(l == 3) digitalWrite(PIN_LED3, st);
}

void executarAlert() {
  // Agora usa o valor estabilizado no ACC
  if (ACC <= 10) { 
    digitalWrite(PIN_BUZZER, HIGH); 
    digitalWrite(PIN_LED1, HIGH); 
    Serial.println("ALERTA: < 10cm");
  }
  else if (ACC < 20) { 
    digitalWrite(PIN_BUZZER, LOW); 
    digitalWrite(PIN_LED1, HIGH); 
    Serial.println("ALERTA: 10-20cm");
  }
  else { 
    digitalWrite(PIN_BUZZER, LOW); 
    digitalWrite(PIN_LED1, LOW); 
  }
}

void exibirEstados() {
  Serial.print("PC: "); Serial.print(PC);
  Serial.print(" | IR: "); Serial.print(IR, BIN);
  Serial.print(" | ACC: "); Serial.print(ACC);
  Serial.print(" | FLAG_Z: "); Serial.println(FLAG_Z);
}