/*
Lucas Calil Cavalcanti - TA
Marco Antonio - TA
*/

#include <Keypad.h>

// --- 1. Elementos Arquiteturais Obrigatórios ---
int MEM[16];            // Memória de dados simulada
int PC = 0;             // Program Counter (Contador de Programa)
byte IR = 0;            // Instruction Register (Registrador de Instrução)
int ACC = 0;            // Acumulador
bool FLAG_Z = false;    // Flag de comparação (Zero)
bool EXECUTANDO = false; // Controle de execução (Modo RUN)
bool MODO_LOAD = false; // Controle do Modo LOAD

// --- 2. Memória de Programa (Instruções Armazenadas) ---
String programa[16];    // Armazena os mnemônicos digitados
int ponteiroCarga = 0;  // Aponta onde a próxima instrução será gravada

// --- 3. Configuração do Hardware (Pinagem IBMEC) ---
const int PIN_TRIG = 40;
const int PIN_ECHO = 41;
const int PIN_LED1 = 42;
const int PIN_LED2 = 43;
const int PIN_LED3 = 44;
const int PIN_BUZZER = 45;

// Pinos do Display de 7 Segmentos (a-g)
const int displayPins[] = {22, 23, 24, 25, 26, 27, 28};

// Teclado Matricial
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {30, 31, 32, 33}; 
byte colPins[COLS] = {34, 35, 36, 37}; 
Keypad teclado = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(9600);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  pinMode(PIN_LED3, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  for(int i=0; i<7; i++) pinMode(displayPins[i], OUTPUT);
  
  Serial.println("SISTEMA IBMEC INICIADO");
  Serial.println("Pressione # para modo LOAD");
}

void loop() {
  char tecla = teclado.getKey();
  if (tecla) trataEntrada(tecla);
}

// --- 4. Unidade de Controle (UC) ---

void trataEntrada(char t) {
  if (t == '#') { // Alterna modo LOAD
    MODO_LOAD = !MODO_LOAD;
    if (MODO_LOAD) {
      limparSistema();
      Serial.println("MODO LOAD ATIVO: Digite Opcode e Operando (ex: 2 5)");
    } else {
      Serial.println("MODO LOAD ENCERRADO. Digite 'A' (RUN) para iniciar.");
    }
    return;
  }

  if (MODO_LOAD) {
    armazenarInstrucao(t);
  } else if (t == 'A') { // Comando RUN
    PC = 0;
    EXECUTANDO = true;
    Serial.println("MODO RUN ATIVO. Pressione '*' para o ciclo de instrucao.");
  } else if (t == '*' && EXECUTANDO) { // Ciclo de Instrução
    cicloDeInstrucao();
  }
}

void cicloDeInstrucao() {
  if (PC >= 16) { EXECUTANDO = false; return; }

  // 1. Busca e Decodificação
  String instRaw = programa[PC];
  if (instRaw == "") return;

  int opcDecimal = instRaw.substring(0, instRaw.indexOf(' ')).toInt();
  int operando = instRaw.substring(instRaw.indexOf(' ') + 1).toInt();
  
  IR = (byte)opcDecimal; // Carga no Instruction Register

  // 2. Execução (ULA e I/O)
  executarOpcode(IR, operando);

  // 3. Exibição de Estados Internos
  Serial.print("PC: "); Serial.print(PC);
  Serial.print(" | IR: "); Serial.print(IR, BIN);
  Serial.print(" | ACC: "); Serial.print(ACC);
  Serial.print(" | FLAG_Z: "); Serial.println(FLAG_Z);

  PC++; // Atualiza Program Counter
}

// --- 5. Unidade Lógica e Aritmética (ULA) ---

void executarOpcode(byte op, int val) {
  switch(op) {
    case 1:  ACC = lerDistancia(); break;             // READ
    case 2:  ACC = val; break;                         // LOADK
    case 3:  ACC += val; break;                        // ADDK
    case 4:  ACC -= val; break;                        // SUBK
    case 5:  FLAG_Z = (ACC == val); break;             // CMPK
    case 6:  acionarSaida(val, HIGH); break;           // LEDON
    case 7:  acionarSaida(val, LOW); break;            // LEDOFF
    case 8:  digitalWrite(PIN_BUZZER, HIGH); break;    // BUZON
    case 9:  digitalWrite(PIN_BUZZER, LOW); break;     // BUZOFF
    case 10: exibirDisplay(ACC); break;                // DISP
    case 11: executarAlert(); break;                   // ALERT
    case 13: MEM[val % 16] = ACC; break;               // STORE
    case 14: ACC = MEM[val % 16]; break;               // LOADM
    case 15: EXECUTANDO = false; Serial.println("HALT"); break; // HALT
  }
}

// --- 6. Funções de Suporte (Periféricos) ---

int lerDistancia() {
  digitalWrite(PIN_TRIG, LOW); delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  return pulseIn(PIN_ECHO, HIGH) * 0.034 / 2;
}

void acionarSaida(int led, int estado) {
  if(led == 1) digitalWrite(PIN_LED1, estado);
  else if(led == 2) digitalWrite(PIN_LED2, estado);
  else if(led == 3) digitalWrite(PIN_LED3, estado);
}

void executarAlert() {
  int d = lerDistancia();
  if (d < 10) { digitalWrite(PIN_BUZZER, HIGH); digitalWrite(PIN_LED1, HIGH); }
  else if (d < 20) { digitalWrite(PIN_LED1, HIGH); digitalWrite(PIN_BUZZER, LOW); }
  else { digitalWrite(PIN_BUZZER, LOW); digitalWrite(PIN_LED1, LOW); }
}

void exibirDisplay(int valor) {
  if (valor > 9) { Serial.println("OVERFLOW!"); exibirErro('E'); return; } //
  if (valor < 0) { Serial.println("NEGATIVO!"); exibirErro('-'); return; } //
  // Lógica de segmentos simplificada para exemplo
}

void exibirErro(char tipo) {
  // Liga os segmentos correspondentes ao erro no display
}

void armazenarInstrucao(char t) {
  static String buffer = "";
  if (t == 'B') { // Usando 'B' como ENTER para confirmar instrução
    programa[ponteiroCarga++] = buffer;
    Serial.print("Armazenado em MEM_PROG["); Serial.print(ponteiroCarga-1);
    Serial.print("]: "); Serial.println(buffer);
    buffer = "";
  } else {
    buffer += t;
  }
}

void limparSistema() {
  for(int i=0; i<16; i++) { programa[i] = ""; MEM[i] = 0; }
  ponteiroCarga = 0; ACC = 0; PC = 0; FLAG_Z = false;
}