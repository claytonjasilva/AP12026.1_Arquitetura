#include <Keypad.h>

/*
  APERTE 0 PARA TESTAR O BUZZER E O SENSOR (check)
  APERTE # PARA ENTRAR NO LOAD E NOVAMENTE PARA SAIR (check)
  PARA LIGAR O LED 1 ---> 6 C 1 B #, 6 C 2 B para o segundo ,
   para ligar o restante, pode digitar 6 c 2 b 15 b # e o 3, só trocar o 2 pelo 3 
  FUNÇÃO DO HALT ( Ago esliga tudo ao finalizar)
  Ler sensor e mostrar no displa  # 1 B 10 B 15 B # A * * *
  APERTE * PARA CADA PASSO 
  lIGAR BUZZER ----> 8 b (buzon) e 15 b # () Buzzer desliga no HALT)
  Colocar o item no display ---> ex: 2 c 8 b 10 b 15  b # (Display não testado, possivel problema físico, codigo correto)
*/ 


const bool DISPLAY_ANODO_COMUM = false; // Mudado algumaas vezes

const int LED1 = 42;
const int LED2 = 43;
const int LED3 = 44;

const int BUZZER = 45;

const int TRIG = 40;
const int ECHO = 41;

const int SEG_A  = 22;
const int SEG_B  = 23;
const int SEG_C  = 24;
const int SEG_D  = 25;
const int SEG_E  = 26;
const int SEG_F  = 27;
const int SEG_G  = 28;
const int SEG_DP = 29;

// Teclado 4x4
const byte ROWS = 4;
const byte COLS = 4;

char teclas[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {30, 31, 32, 33};
byte colPins[COLS] = {34, 35, 36, 37};

Keypad keypad = Keypad(makeKeymap(teclas), rowPins, colPins, ROWS, COLS);


int MEM[16];
int PC = 0;
byte IR = 0;
int ACC = 0;

bool FLAG_Z = false;
bool EXECUTANDO = false;


const int MAX_PROG = 32;
String programa[MAX_PROG];
int tamPrograma = 0;


#define OP_NOP    0
#define OP_READ   1
#define OP_LOADK  2
#define OP_ADDK   3
#define OP_SUBK   4
#define OP_CMPK   5
#define OP_LEDON  6
#define OP_LEDOFF 7
#define OP_BUZON  8
#define OP_BUZOFF 9
#define OP_DISP   10
#define OP_ALERT  11
#define OP_BINC   12
#define OP_STORE  13
#define OP_LOADM  14
#define OP_HALT   15


bool modoLOAD = false;
bool modoRUN = false;
String bufferEntrada = "";

bool sensorContinuo = false;
unsigned long ultimaLeituraSensor = 0;
const unsigned long intervaloSensor = 500;


void inicializarHardware();
void atualizarSensorSerial();
void mostrarDisplay(int valor);
void apagarDisplay();
void displayLetraE();
void displayMenos();
void escreverSegmento(int pino, bool ligado);
void ligarLED(int num);
void desligarLED(int num);
void ligarBuzzer();
void desligarBuzzer();
int lerDistancia();
byte mnemonicoParaOpcode(String mnemonico);
String decodificarLinha(String linha);
void processarEntradaLOAD(String entrada);
void iniciarRUN();
void executarInstrucao(String instrucao);
void imprimirEstado(int pcExec, String instrExec);
void imprimirAjuda();
void printLinha(char c = '-', int n = 50);
void printSecao(const __FlashStringHelper* titulo);
void printOK(const __FlashStringHelper* msg);
void printErro(const __FlashStringHelper* msg);
void printInfo(const __FlashStringHelper* msg);
void imprimeBinario(byte val);
void desligarSaidas();

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(9600);
  delay(1000);

  inicializarHardware();

  for (int i = 0; i < 16; i++) {
    MEM[i] = 0;
  }

  printLinha('=', 52);
  Serial.println(F(" ARQ DE COMPUTADORES - AP1"));
  printLinha('=', 52);

  imprimirAjuda();
}

// ============================================================
// LOOP PRINCIPAL
// ============================================================
void loop() {
  if (sensorContinuo) {
    atualizarSensorSerial();
  }

  char tecla = keypad.getKey();
  if (!tecla) return;

  Serial.print(F("Tecla pressionada: "));
  Serial.println(tecla);

  // Tecla 0: liga/desliga sensor contínuo
  if (tecla == '0' && !modoLOAD) {
    sensorContinuo = !sensorContinuo;
    if (sensorContinuo) {
      printInfo(F("Sensor continuo LIGADO"));
      printInfo(F("Buzzer dispara quando distancia < 10 cm."));
    } else {
      desligarBuzzer();
      printInfo(F("Sensor continuo DESLIGADO."));
    }
    return;
  }

  // Tecla #
  if (tecla == '#') {
    if (!modoLOAD) {
      modoLOAD = true; modoRUN = false; EXECUTANDO = false;
      sensorContinuo = false; 
      

      desligarSaidas(); 
      apagarDisplay();

      tamPrograma = 0; bufferEntrada = ""; PC = 0;

      for (int i = 0; i < MAX_PROG; i++) programa[i] = "";

      printLinha('-', 52);
      printSecao(F("MODO LOAD ATIVADO"));
      Serial.println(F("  0-9 -> digito\n  C   -> espaco\n  B   -> confirmar instrucao\n  D   -> apagar\n  #   -> sair do LOAD"));
      printLinha('-', 52);
    } 
    else {
      modoLOAD = false; bufferEntrada = "";

      printLinha('-', 52);
      printSecao(F("MODO LOAD ENCERRADO"));
      Serial.print(F("  Instrucoes carregadas: "));
      Serial.println(tamPrograma);
      printLinha('.', 52);

      for (int i = 0; i < tamPrograma; i++) {
        String instr = programa[i];
        int esp = instr.indexOf(' ');
        String mnem = (esp == -1) ? instr : instr.substring(0, esp);
        byte op = mnemonicoParaOpcode(mnem);

        Serial.print(F("  ["));
        if (i < 10) Serial.print('0');
        Serial.print(i);
        Serial.print(F("] "));
        Serial.print(instr);
        Serial.print(F(" -> "));
        imprimeBinario(op);
        Serial.println();
      }

      printLinha('-', 52);
      Serial.println(F("  Pressione A para executar."));
      printLinha('-', 52);
    }
    return;
  }

  // Tecla A
  if (tecla == 'A') {
    if (modoLOAD) printErro(F("Saia do LOAD antes de RUN. Pressione #."));
    else iniciarRUN();
    return;
  }

  // Tecla *
  if (tecla == '*') {
    if (modoLOAD) { printInfo(F("Tecla * ignorada no modo LOAD.")); return; }
    if (!modoRUN) { printInfo(F("Sistema nao esta em RUN. Pressione A.")); return; }
    if (!EXECUTANDO) { printInfo(F("Execucao encerrada. Pressione A para reiniciar.")); return; }

    if (PC >= tamPrograma) {
      printErro(F("Fim da memoria sem HALT."));
      EXECUTANDO = false; modoRUN = false;
      
      // Garante que buzzer e LEDs desliguem se o programa acabar à força
      desligarSaidas(); 
      return;
    }
    executarInstrucao(programa[PC]);
    return;
  }

  // Teclas do modo LOAD
  if (modoLOAD) {
    if (tecla == 'B') {
      Serial.println();
      if (bufferEntrada.length() > 0) {
        processarEntradaLOAD(bufferEntrada);
        bufferEntrada = "";
      }
    } 
    else if (tecla == 'C') {
      bufferEntrada += ' '; Serial.print(F(" "));
    } 
    else if (tecla == 'D') {
      if (bufferEntrada.length() > 0) {
        bufferEntrada.remove(bufferEntrada.length() - 1); Serial.print(F("\b \b"));
      }
    } 
    else if (tecla >= '0' && tecla <= '9') {
      bufferEntrada += tecla; Serial.print(tecla);
    }
  }
}

// ============================================================
// FUNÇÃO DE DESLIGAMENTO DE SEGURANÇA
// ============================================================
void desligarSaidas() {
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(BUZZER, LOW);
}

// ============================================================
// FUNÇÃO ENCURTADORA: IMPRIME BINÁRIO
// ============================================================
void imprimeBinario(byte val) {
  for (int i = 3; i >= 0; i--) {
    Serial.print(((val >> i) & 1) ? '1' : '0');
  }
}

// ============================================================
// SENSOR CONTÍNUO
// ============================================================
void atualizarSensorSerial() {
  unsigned long agora = millis();
  if (agora - ultimaLeituraSensor >= intervaloSensor) {
    ultimaLeituraSensor = agora;
    int distancia = lerDistancia();

    Serial.print(F("[SENSOR CONTINUO] Distancia atual: "));
    Serial.print(distancia);
    Serial.print(F(" cm | Buzzer: "));

    if (distancia < 10) {
      ligarBuzzer();
      Serial.println(F("LIGADO"));
    } else {
      desligarBuzzer();
      Serial.println(F("desligado"));
    }
  }
}

// ============================================================
// HARDWARE
// ============================================================
void inicializarHardware() {
  pinMode(LED1, OUTPUT); pinMode(LED2, OUTPUT); pinMode(LED3, OUTPUT); pinMode(BUZZER, OUTPUT);
  digitalWrite(LED1, LOW); digitalWrite(LED2, LOW); digitalWrite(LED3, LOW); digitalWrite(BUZZER, LOW);

  pinMode(TRIG, OUTPUT); pinMode(ECHO, INPUT); digitalWrite(TRIG, LOW);

  pinMode(SEG_A, OUTPUT); pinMode(SEG_B, OUTPUT); pinMode(SEG_C, OUTPUT); pinMode(SEG_D, OUTPUT);
  pinMode(SEG_E, OUTPUT); pinMode(SEG_F, OUTPUT); pinMode(SEG_G, OUTPUT); pinMode(SEG_DP, OUTPUT);

  apagarDisplay();
}

void escreverSegmento(int pino, bool ligado) {
  if (DISPLAY_ANODO_COMUM) digitalWrite(pino, ligado ? LOW : HIGH);
  else digitalWrite(pino, ligado ? HIGH : LOW);
}

void mostrarDisplay(int valor) {
  const byte digitos[10][7] = {
    {1,1,1,1,1,1,0}, {0,1,1,0,0,0,0}, {1,1,0,1,1,0,1}, {1,1,1,1,0,0,1}, {0,1,1,0,0,1,1},
    {1,0,1,1,0,1,1}, {1,0,1,1,1,1,1}, {1,1,1,0,0,0,0}, {1,1,1,1,1,1,1}, {1,1,1,1,0,1,1} 
  };

  if (valor > 9) { displayLetraE(); return; }
  if (valor < 0) { displayMenos(); return; }

  const int pinos[7] = { SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G };
  for (int i = 0; i < 7; i++) escreverSegmento(pinos[i], digitos[valor][i] == 1);
  escreverSegmento(SEG_DP, false);
}

void apagarDisplay() {
  const int pinos[8] = { SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G, SEG_DP };
  for(int i=0; i<8; i++) escreverSegmento(pinos[i], false);
}

void displayLetraE() {
  escreverSegmento(SEG_A, true); escreverSegmento(SEG_B, false); escreverSegmento(SEG_C, false);
  escreverSegmento(SEG_D, true); escreverSegmento(SEG_E, true); escreverSegmento(SEG_F, true);
  escreverSegmento(SEG_G, true); escreverSegmento(SEG_DP, false);
}

void displayMenos() {
  escreverSegmento(SEG_A, false); escreverSegmento(SEG_B, false); escreverSegmento(SEG_C, false);
  escreverSegmento(SEG_D, false); escreverSegmento(SEG_E, false); escreverSegmento(SEG_F, false);
  escreverSegmento(SEG_G, true); escreverSegmento(SEG_DP, false);
}

void ligarLED(int num) {
  switch (num) {
    case 1: digitalWrite(LED1, HIGH); break;
    case 2: digitalWrite(LED2, HIGH); break;
    case 3: digitalWrite(LED3, HIGH); break;
    default: printErro(F("LED invalido. Use 1, 2 ou 3.")); break;
  }
}

void desligarLED(int num) {
  switch (num) {
    case 1: digitalWrite(LED1, LOW); break;
    case 2: digitalWrite(LED2, LOW); break;
    case 3: digitalWrite(LED3, LOW); break;
    default: printErro(F("LED invalido. Use 1, 2 ou 3.")); break;
  }
}

void ligarBuzzer() { digitalWrite(BUZZER, HIGH); }
void desligarBuzzer() { digitalWrite(BUZZER, LOW); }

int lerDistancia() {
  digitalWrite(TRIG, LOW); delayMicroseconds(2);
  digitalWrite(TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long duracao = pulseIn(ECHO, HIGH, 30000UL);
  if (duracao == 0) return 400;
  int distancia = (int)(duracao / 58);
  if (distancia <= 0 || distancia > 400) return 400;
  return distancia;
}

// ============================================================
// CODIFICAÇÃO / DECODIFICAÇÃO
// ============================================================
byte mnemonicoParaOpcode(String m) {
  m.toUpperCase(); m.trim();
  if (m == "NOP")    return OP_NOP;
  if (m == "READ")   return OP_READ;
  if (m == "LOADK")  return OP_LOADK;
  if (m == "ADDK")   return OP_ADDK;
  if (m == "SUBK")   return OP_SUBK;
  if (m == "CMPK")   return OP_CMPK;
  if (m == "LEDON")  return OP_LEDON;
  if (m == "LEDOFF") return OP_LEDOFF;
  if (m == "BUZON")  return OP_BUZON;
  if (m == "BUZOFF") return OP_BUZOFF;
  if (m == "DISP")   return OP_DISP;
  if (m == "ALERT")  return OP_ALERT;
  if (m == "BINC")   return OP_BINC;
  if (m == "STORE")  return OP_STORE;
  if (m == "LOADM")  return OP_LOADM;
  if (m == "HALT")   return OP_HALT;
  return 0xFF;
}

String decodificarLinha(String linha) {
  linha.trim();
  int espaco = linha.indexOf(' ');
  int opDec = (espaco == -1) ? linha.toInt() : linha.substring(0, espaco).toInt();
  int oper = (espaco == -1) ? -1 : linha.substring(espaco + 1).toInt();
  String mnem;

  switch (opDec) {
    case OP_NOP:    mnem = "NOP";    break;
    case OP_READ:   mnem = "READ";   break;
    case OP_LOADK:  mnem = "LOADK";  break;
    case OP_ADDK:   mnem = "ADDK";   break;
    case OP_SUBK:   mnem = "SUBK";   break;
    case OP_CMPK:   mnem = "CMPK";   break;
    case OP_LEDON:  mnem = "LEDON";  break;
    case OP_LEDOFF: mnem = "LEDOFF"; break;
    case OP_BUZON:  mnem = "BUZON";  break;
    case OP_BUZOFF: mnem = "BUZOFF"; break;
    case OP_DISP:   mnem = "DISP";   break;
    case OP_ALERT:  mnem = "ALERT";  break;
    case OP_BINC:   mnem = "BINC";   break;
    case OP_STORE:  mnem = "STORE";  break;
    case OP_LOADM:  mnem = "LOADM";  break;
    case OP_HALT:   mnem = "HALT";   break;
    default:        mnem = "???";    break;
  }
  return (oper >= 0) ? mnem + " " + String(oper) : mnem;
}

// ============================================================
// LOAD E RUN
// ============================================================
void processarEntradaLOAD(String entrada) {
  entrada.trim();
  if (entrada.length() == 0) return;
  if (tamPrograma >= MAX_PROG) { printErro(F("Memoria cheia. Maximo: 32 instrucoes.")); return; }

  String instrucao = decodificarLinha(entrada);
  if (instrucao == "???") {
    Serial.print(F("  [ERR] Instrucao invalida: ")); Serial.println(entrada); return;
  }

  programa[tamPrograma] = instrucao;
  Serial.print(F("  [OK] ["));
  if (tamPrograma < 10) Serial.print('0');
  Serial.print(tamPrograma); Serial.print(F("] ")); Serial.println(instrucao);
  tamPrograma++;
}

void iniciarRUN() {
  if (tamPrograma == 0) { printInfo(F("Nenhum programa carregado. Use #.")); return; }
  PC = 0; EXECUTANDO = true; modoRUN = true;
  printLinha('=', 52);
  Serial.print(F("  MODO RUN | ")); Serial.print(tamPrograma); Serial.println(F(" instrucoes | PC = 0"));
  printLinha('=', 52); Serial.println(F("  Pressione * para cada passo.")); printLinha('-', 52);
}

// ============================================================
// CICLO DE INSTRUÇÃO
// ============================================================
void executarInstrucao(String instrucao) {
  instrucao.trim();
  int espaco = instrucao.indexOf(' ');
  String mnem = (espaco == -1) ? instrucao : instrucao.substring(0, espaco);
  int operando = (espaco == -1) ? -1 : instrucao.substring(espaco + 1).toInt();

  IR = mnemonicoParaOpcode(mnem);
  int pcAtual = PC;
  PC++;

  switch (IR) {
    case OP_NOP: printInfo(F("NOP - nenhuma operacao.")); break;
    case OP_READ: {
      ACC = lerDistancia();
      Serial.print(F("  [READ] -> ")); Serial.print(ACC); Serial.println(F(" cm -> ACC"));
      break;
    }
    case OP_LOADK:
      if (operando < 0) printErro(F("LOADK requer operando."));
      else ACC = operando;
      break;
    case OP_ADDK:
      if (operando < 0) printErro(F("ADDK requer operando."));
      else ACC += operando;
      break;
    case OP_SUBK:
      if (operando < 0) printErro(F("SUBK requer operando."));
      else ACC -= operando;
      break;
    case OP_CMPK:
      if (operando < 0) { printErro(F("CMPK requer operando.")); break; }
      FLAG_Z = (ACC == operando);
      Serial.print(F("  [CMPK] ACC(")); Serial.print(ACC); Serial.print(F(") == "));
      Serial.print(operando); Serial.print(F(" -> FLAG_Z = ")); Serial.println(FLAG_Z ? 1 : 0);
      break;
    case OP_LEDON:
      if (operando < 1 || operando > 3) printErro(F("LEDON: use 1, 2 ou 3."));
      else { ligarLED(operando); Serial.print(F("  [LEDON] -> LED ")); Serial.print(operando); Serial.println(F(" LIGADO")); }
      break;
    case OP_LEDOFF:
      if (operando < 1 || operando > 3) printErro(F("LEDOFF: use 1, 2 ou 3."));
      else { desligarLED(operando); Serial.print(F("  [LEDOFF] -> LED ")); Serial.print(operando); Serial.println(F(" DESLIGADO")); }
      break;
    case OP_BUZON: ligarBuzzer(); printOK(F("BUZON -> Buzzer LIGADO")); break;
    case OP_BUZOFF: desligarBuzzer(); printOK(F("BUZOFF -> Buzzer DESLIGADO")); break;
    case OP_DISP:
      if (operando >= 0) ACC = operando;
      mostrarDisplay(ACC);
      Serial.print(F("  [DISP] -> ACC = ")); Serial.print(ACC);
      if (ACC > 9) Serial.println(F(" | OVERFLOW: mostra E"));
      else if (ACC < 0) Serial.println(F(" | NEGATIVO: mostra -"));
      else Serial.println();
      break;
    case OP_ALERT: {
      int d = lerDistancia();
      Serial.print(F("  [ALERT] -> ")); Serial.print(d); Serial.print(F(" cm | "));
      if (d < 10) {
        ligarBuzzer(); ligarLED(1); ligarLED(2); ligarLED(3);
        Serial.println(F("PERIGO: buzzer + LEDs ligados"));
      } else if (d < 20) {
        desligarBuzzer(); ligarLED(1); ligarLED(2); ligarLED(3);
        Serial.println(F("ALERTA: LEDs ligados"));
      } else {
        desligarBuzzer(); desligarLED(1); desligarLED(2); desligarLED(3);
        Serial.println(F("SEGURO: saidas desligadas"));
      }
      break;
    }
    case OP_BINC:
      Serial.print(F("  [BINC] -> IR = ")); Serial.print(IR); Serial.print(F(" ("));
      imprimeBinario(IR); 
      Serial.println(F(")"));
      break;
    case OP_STORE:
      if (operando < 0 || operando > 15) printErro(F("STORE: endereco invalido. Use 0 a 15."));
      else {
        MEM[operando] = ACC;
        Serial.print(F("  [STORE] -> MEM[")); Serial.print(operando); Serial.print(F("] = ")); Serial.println(ACC);
      }
      break;
    case OP_LOADM:
      if (operando < 0 || operando > 15) printErro(F("LOADM: endereco invalido. Use 0 a 15."));
      else {
        ACC = MEM[operando];
        Serial.print(F("  [LOADM] -> ACC = MEM[")); Serial.print(operando); Serial.print(F("] = ")); Serial.println(ACC);
      }
      break;
    case OP_HALT:
      EXECUTANDO = false; modoRUN = false;
      
      // HALT agora desliga os LEDs e o Buzzer
      desligarSaidas(); 
      
      printLinha('=', 52); Serial.println(F("  [HALT] Execucao encerrada.\n  Pressione A para reiniciar.")); printLinha('=', 52);
      break;
    default:
      Serial.print(F("  [ERR] Instrucao desconhecida: ")); Serial.println(instrucao);
      break;
  }

  imprimirEstado(pcAtual, instrucao);
  if (IR == OP_STORE || IR == OP_LOADM) {
    Serial.print(F("  MEM[")); Serial.print(operando); Serial.print(F("] = ")); Serial.println(MEM[operando]);
  }
  printLinha('.', 52);
}

void imprimirEstado(int pcExec, String instrExec) {
  Serial.print(F("  PC: ")); Serial.print(pcExec);
  Serial.print(F(" | IR: ")); Serial.print(instrExec);
  Serial.print(F(" | OPCODE: "));
  imprimeBinario(IR); 
  Serial.print(F(" | ACC: ")); Serial.print(ACC);
  Serial.print(F(" | FLAG_Z: ")); Serial.print(FLAG_Z ? 1 : 0);
  Serial.print(F(" | MEM[0]: ")); Serial.print(MEM[0]);
  Serial.print(F(" | MEM[1]: ")); Serial.println(MEM[1]);
}

void printLinha(char c, int n) {
  Serial.print(F("  "));
  for (int i = 0; i < n; i++) Serial.print(c);
  Serial.println();
}

void printSecao(const __FlashStringHelper* titulo) { Serial.print(F("  >> ")); Serial.println(titulo); }
void printOK(const __FlashStringHelper* msg) { Serial.print(F("  [OK] ")); Serial.println(msg); }
void printErro(const __FlashStringHelper* msg) { Serial.print(F("  [ERR] ")); Serial.println(msg); }
void printInfo(const __FlashStringHelper* msg) { Serial.print(F("  [INFO] ")); Serial.println(msg); }

void imprimirAjuda() {
  Serial.println(F("\n  TECLAS\n "));
  Serial.println(F("   #  - Entra / sai do modo LOAD\n   A   - RUN\n   *   Executa uma instrucao por vez\n  - B   ENTER - confirma instrucao no LOAD"));
  Serial.println(F("   C   ESPACO - separa opcode do operando\n   D  - BACKSPACE - apaga ultimo digito\n   0  vai ligar/desliga sensor continuo + buzzer automatico"));
  printLinha('=', 52); Serial.println(F("  Pressione # para comecar o LOAD.\n  Pressione 0 para ligar/desligar o sensor continuo.")); printLinha('=', 52);
}
