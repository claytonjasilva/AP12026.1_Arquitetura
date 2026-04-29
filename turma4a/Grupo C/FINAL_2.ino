/**

   Integrantes do Grupo:

   Kauê Fernandes - 202502070527  (10)
   Iago Viana - 202501001531  (10)
   Gianluca Leonardi - 202501001254 (10)
   Caio Magalhães - 202507153706 (10)
   Leonardo Vaz - 202402626604 (10)
 * ──────────────────────────────────────────────────────────────
 *  CHEATS  (sequência exata de teclas, estilo GTA)
 * ──────────────────────────────────────────────────────────────
 *
 *  BUZOFF  — desliga buzzer
 *    # · 9 B · 1 5 B · # · A · * *
 *
 *  BUZON  — liga buzzer
 *    # · 8 B · 1 5 B · # · A · * *
 *
 *  LEDON 1  — liga LED 1
 *    # · 6 C 1 B · 1 5 B · # · A · * *
 *
 *  LEDOFF 2  — desliga LED 2
 *    # · 7 C 2 B · 1 5 B · # · A · * *
 *
 *  READ + DISP  — lê sensor e mostra no display
 *    # · 1 B · 1 0 B · 1 5 B · # · A · *
 *
 *  LOADK 5 + DISP  — mostra número 5 no display
 *    # · 2 C 5 B · 1 0 B · 1 5 B · # · A · * * *
 *
 *  ALERT  — alarme automático por distância
 *    # · 1 1 B · 1 5 B · # · A · * *
 *
 *  STORE ACC em MEM[3]  — salva valor na memória
 *    # · 2 C 7 B · 1 3 C 3 B · 1 5 B · # · A · * * *
 *
 *  LOADM de MEM[3]  — carrega memória de volta no ACC
 *    # · 1 4 C 3 B · 1 0 B · 1 5 B · # · A · * * *
 *
 *  CMPK — compara ACC com constante (FLAG_Z)
 *    # · 2 C 5 B · 5 C 5 B · 1 5 B · # · A · * * *
 *
 *  ADDK 3  — soma 3 ao ACC
 *    # · 2 C 4 B · 3 C 3 B · 1 0 B · 1 5 B · # · A · * * * *
 *
 *  (cada · é só pra leitura — não é uma tecla real)
 *
 * ╔══════════════════════════════════════════════════════════════╗  <- APENAS COMENTARIO, nao aparece no Serial
 * TRABALHO AP1 2026.1 - Arquitetura de Computadores
 * Sistema Interpretador de Instrucoes com HC-SR04
 * Plataforma: Arduino Mega 2560
 *
 * ATENCAO - PINOS RX0/TX0:
 * Os pinos 0 (SEG_B) e 1 (SEG_A) sao RX0/TX0 do Serial USB.
 * Eles podem piscar no display durante impressoes Serial.
 * Se isso ocorrer, mova SEG_A/SEG_B para pinos 14 e 15.
 *
 * MAPA ARQUITETURAL:
 *   Memoria de programa -> vetor programa[]
 *   Memoria de dados    -> MEM[16]
 *   Program Counter     -> PC
 *   Instruction Register-> IR
 *   Acumulador          -> ACC
 *   Flag Zero           -> FLAG_Z
 *   Unidade de Controle -> executarInstrucao() / iniciarRUN()
 *   ULA                 -> cases ADDK, SUBK, CMPK
 *   Entrada             -> Teclado matricial 4x4
 *   Saida               -> LEDs, buzzer, display 7-seg, Serial
 *
 * LEGENDA DAS TECLAS:
 *   #  = entrar/sair do LOAD    A  = RUN     *  = proximo passo
 *   B  = confirmar (ENTER)      C  = espaco  D  = apagar
 *   sem operando -> so  [op] B
 *   com operando -> [op] C [val] B
 *
 * TABELA RESUMIDA (op -> mnemonico):
 *    0 NOP   1 READ   2 LOADK  3 ADDK   4 SUBK   5 CMPK
 *    6 LEDON 7 LEDOFF 8 BUZON  9 BUZOFF 10 DISP  11 ALERT
 *   12 BINC  13 STORE 14 LOADM 15 HALT
 */

#include <Keypad.h>

// ============================================================
// PINAGEM
// ============================================================

const int LED1   = 11;
const int LED2   = 10;
const int LED3   = 9;
const int BUZZER = 8;
const int TRIG   = 22;   // movido: pino 13 tem LED onboard no Mega (interfere no ECHO)
const int ECHO   = 23;   // movido: pulseIn no pino 13 e distorcido pelo LED/resistor interno

// Display 7 segmentos (catodo comum)
// ATENCAO: SEG_A (pino 1) e SEG_B (pino 0) sao TX0/RX0
const int SEG_A  = 32;
const int SEG_B  = 30;
const int SEG_C  = 7;
const int SEG_D  = 5;
const int SEG_E  = 4;
const int SEG_F  = 2;
const int SEG_G  = 3;
const int SEG_DP = 6;

// Teclado 4x4
const byte ROWS = 4;
const byte COLS = 4;
char teclas[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {33, 35, 37, 39};
byte colPins[COLS] = {41, 43, 45, 47};
Keypad keypad = Keypad(makeKeymap(teclas), rowPins, colPins, ROWS, COLS);

// ============================================================
// REGISTRADORES E MEMORIA
// ============================================================

int  MEM[16];
int  PC      = 0;
byte IR      = 0;
int  ACC     = 0;
bool FLAG_Z     = false;
bool EXECUTANDO = true;

// ============================================================
// MEMORIA DE PROGRAMA
// ============================================================

const int MAX_PROG = 32;
String programa[MAX_PROG];
int    tamPrograma = 0;

// ============================================================
// OPCODES - ISA 4 bits
// ============================================================

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

// ============================================================
// MODOS E BUFFER
// ============================================================

bool   modoLOAD   = false;
bool   modoRUN    = false;
bool   sensorAtivo = false;
String bufferEntrada = "";

// ============================================================
// PROTOTIPOS
// ============================================================

void   inicializarHardware();
void   mostrarDisplay(int valor);
void   apagarDisplay();
void   displayLetraE();
void   displayMenos();
void   ligarLED(int num);
void   desligarLED(int num);
void   ligarBuzzer();
void   desligarBuzzer();
int    lerDistancia();
byte   mnemonicoParaOpcode(String mnemonico);
String decodificarLinha(String linha);
void   processarEntradaLOAD(String entrada);
void   iniciarRUN();
void   executarInstrucao(String instrucao);
void   imprimirEstado(int pcExec, String instrExec);
void   imprimirAjuda();

void   printLinha(char c = '-', int n = 50);
void   printSecao(const __FlashStringHelper* titulo);
void   printOK(const __FlashStringHelper* msg);
void   printErro(const __FlashStringHelper* msg);
void   printInfo(const __FlashStringHelper* msg);

// ============================================================
// SETUP
// ============================================================

void setup() {
  Serial.begin(9600);
  inicializarHardware();
  for (int i = 0; i < 16; i++) MEM[i] = 0;

  printLinha('=', 52);
  Serial.println(F("  INTERPRETADOR DE INSTRUCOES - AP1 2026.1"));
  Serial.println(F("  Arquitetura de Computadores"));
  printLinha('=', 52);
  imprimirAjuda();
}

// ============================================================
// LOOP PRINCIPAL
// ============================================================

void loop() {
  if (sensorAtivo) {
    int d = lerDistancia();
    Serial.println(d);

    int nivel;
    if      (d >= 200) nivel = 9;
    else if (d >= 80) nivel = 8;
    else if (d >= 65) nivel = 7;
    else if (d >=  52) nivel = 6;
    else if (d >=  40) nivel = 5;
    else if (d >=  30) nivel = 4;
    else if (d >=  20) nivel = 3;
    else if (d >=  10) nivel = 2;
    else               nivel = 1;

    mostrarDisplay(nivel);

    if (d < 10) {
      digitalWrite(BUZZER, HIGH);
      digitalWrite(LED1, HIGH); digitalWrite(LED2, HIGH); digitalWrite(LED3, HIGH);
    } else if (d < 30) {
      digitalWrite(BUZZER, LOW);
      digitalWrite(LED1, HIGH); digitalWrite(LED2, HIGH); digitalWrite(LED3, LOW);
    } else if (d < 60) {
      digitalWrite(BUZZER, LOW);
      digitalWrite(LED1, HIGH); digitalWrite(LED2, LOW); digitalWrite(LED3, LOW);
    } else {
      digitalWrite(BUZZER, LOW);
      digitalWrite(LED1, LOW); digitalWrite(LED2, LOW); digitalWrite(LED3, LOW);
    }
  }

  delay(200);

  char tecla = keypad.getKey();
  if (!tecla) return;

  // -- 0 -- Liga / desliga sensor ---------------------------
  if (tecla == '0' && !modoLOAD) {
    sensorAtivo = !sensorAtivo;
    if (sensorAtivo) {
      printInfo(F("Sensor LIGADO."));
    } else {
      apagarDisplay();
      desligarBuzzer();
      desligarLED(1); desligarLED(2); desligarLED(3);
      printInfo(F("Sensor DESLIGADO."));
    }
    return;
  }

  // -- # -- Entra / sai do modo LOAD -----------------------
  if (tecla == '#') {
    if (!modoLOAD) {
      modoLOAD    = true;
      modoRUN     = false;
      tamPrograma = 0;
      bufferEntrada = "";
      PC = 0;
      for (int i = 0; i < MAX_PROG; i++) programa[i] = "";

      printLinha('-', 52);
      printSecao(F("MODO LOAD ATIVADO"));
      Serial.println(F("  Teclas disponiveis:"));
      Serial.println(F("    0-9 -> digito   C -> espaco"));
      Serial.println(F("    B   -> confirmar  D -> apagar"));
      Serial.println(F("    #   -> sair do LOAD"));
      printLinha('-', 52);
    } else {
      modoLOAD = false;
      bufferEntrada = "";

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
        char bin[5];
        bin[0] = ((op>>3)&1)?'1':'0';
        bin[1] = ((op>>2)&1)?'1':'0';
        bin[2] = ((op>>1)&1)?'1':'0';
        bin[3] = (op&1)    ?'1':'0';
        bin[4] = '\0';

        Serial.print(F("  ["));
        if (i < 10) Serial.print('0');
        Serial.print(i);
        Serial.print(F("]  "));
        Serial.print(instr);
        Serial.print(F("  ->  "));
        Serial.println(bin);
      }
      printLinha('-', 52);
      Serial.println(F("  > Pressione  A  para executar."));
      printLinha('-', 52);
    }
    return;
  }

  // -- A -- RUN --------------------------------------------
  if (tecla == 'A') {
    if (modoLOAD) {
      printErro(F("Saia do LOAD antes de RUN  (pressione #)"));
    } else {
      iniciarRUN();
    }
    return;
  }

  // -- * -- Single-step ------------------------------------
  if (tecla == '*') {
    if (modoLOAD) {
      printInfo(F("Tecla * ignorada no modo LOAD."));
      return;
    }
    if (!modoRUN) {
      printInfo(F("Sistema nao esta em RUN. Pressione A."));
      return;
    }
    if (!EXECUTANDO) {
      printInfo(F("[HALT] Execucao encerrada. Pressione A."));
      return;
    }
    if (PC >= tamPrograma) {
      printErro(F("Fim da memoria sem HALT."));
      EXECUTANDO = false;
      return;
    }
    String instrucaoAtual = programa[PC];   // FETCH
    executarInstrucao(instrucaoAtual);
    return;
  }

  // -- Digitos e letras B/C/D no modo LOAD ----------------
  if (modoLOAD) {
    if (tecla == 'B') {
      Serial.println();
      if (bufferEntrada.length() > 0) {
        processarEntradaLOAD(bufferEntrada);
        bufferEntrada = "";
      }
    } else if (tecla == 'C') {
      bufferEntrada += ' ';
      Serial.print(F(" "));
    } else if (tecla == 'D') {
      if (bufferEntrada.length() > 0) {
        bufferEntrada.remove(bufferEntrada.length() - 1);
        Serial.print(F("\b \b"));
      }
    } else if (tecla >= '0' && tecla <= '9') {
      bufferEntrada += tecla;
      Serial.print(tecla);
    }
  }
}

// ============================================================
// HARDWARE
// ============================================================

void inicializarHardware() {
  pinMode(LED1,   OUTPUT);  digitalWrite(LED1,   LOW);
  pinMode(LED2,   OUTPUT);  digitalWrite(LED2,   LOW);
  pinMode(LED3,   OUTPUT);  digitalWrite(LED3,   LOW);
  pinMode(BUZZER, OUTPUT);  digitalWrite(BUZZER, LOW);
  pinMode(TRIG,   OUTPUT);  digitalWrite(TRIG,   LOW);
  pinMode(ECHO,   INPUT);
  pinMode(SEG_A,  OUTPUT);
  pinMode(SEG_B,  OUTPUT);
  pinMode(SEG_C,  OUTPUT);
  pinMode(SEG_D,  OUTPUT);
  pinMode(SEG_E,  OUTPUT);
  pinMode(SEG_F,  OUTPUT);
  pinMode(SEG_G,  OUTPUT);
  pinMode(SEG_DP, OUTPUT);
  apagarDisplay();
}

// ============================================================
// DISPLAY 7 SEGMENTOS
// ============================================================

void mostrarDisplay(int valor) {
  const byte digitos[10][7] = {
    {1,1,1,1,1,1,0}, // 0
    {0,1,1,0,0,0,0}, // 1
    {1,1,0,1,1,0,1}, // 2
    {1,1,1,1,0,0,1}, // 3
    {0,1,1,0,0,1,1}, // 4
    {1,0,1,1,0,1,1}, // 5
    {1,0,1,1,1,1,1}, // 6
    {1,1,1,0,0,0,0}, // 7
    {1,1,1,1,1,1,1}, // 8
    {1,1,1,1,0,1,1}  // 9
  };

  if (valor > 9) { displayLetraE(); return; }
  if (valor < 0) { displayMenos(); return; }

  const int pinos[7] = {SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G};
  for (int i = 0; i < 7; i++)
    digitalWrite(pinos[i], digitos[valor][i] ? LOW : HIGH);  // anodo comum: LOW acende
  digitalWrite(SEG_DP, HIGH);
}

void apagarDisplay() {
  const int pinos[8] = {SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G, SEG_DP};
  for (int i = 0; i < 8; i++) digitalWrite(pinos[i], HIGH);  // anodo comum: HIGH apaga
}

void displayLetraE() {
  digitalWrite(SEG_A, LOW);  digitalWrite(SEG_B, HIGH);
  digitalWrite(SEG_C, HIGH); digitalWrite(SEG_D, LOW);
  digitalWrite(SEG_E, LOW);  digitalWrite(SEG_F, LOW);
  digitalWrite(SEG_G, LOW);  digitalWrite(SEG_DP, HIGH);
}

void displayMenos() {
  const int pinos[7] = {SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F};
  for (int i = 0; i < 6; i++) digitalWrite(pinos[i], HIGH);  // anodo comum: HIGH apaga
  digitalWrite(SEG_G, LOW);   // só o traço do meio acende
  digitalWrite(SEG_DP, HIGH);
}

// ============================================================
// LEDs E BUZZER
// ============================================================

void ligarLED(int num) {
  switch (num) {
    case 1: digitalWrite(LED1, HIGH); break;
    case 2: digitalWrite(LED2, HIGH); break;
    case 3: digitalWrite(LED3, HIGH); break;
    default: printErro(F("LED invalido (use 1, 2 ou 3)")); break;
  }
}

void desligarLED(int num) {
  switch (num) {
    case 1: digitalWrite(LED1, LOW); break;
    case 2: digitalWrite(LED2, LOW); break;
    case 3: digitalWrite(LED3, LOW); break;
    default: printErro(F("LED invalido (use 1, 2 ou 3)")); break;
  }
}

void ligarBuzzer()    { digitalWrite(BUZZER, HIGH); }
void desligarBuzzer() { digitalWrite(BUZZER, LOW);  }

// ============================================================
// SENSOR ULTRASSONICO HC-SR04
// ============================================================

int lerDistancia() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long dur = pulseIn(ECHO, HIGH, 30000UL);
  if (dur == 0) return 400;
  int d = (int)(dur / 58);
  return (d <= 0 || d > 400) ? 400 : d;
}

// ============================================================
// CODIFICACAO / DECODIFICACAO
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
  int opDec, oper = -1;

  if (espaco == -1) {
    opDec = linha.toInt();
  } else {
    opDec = linha.substring(0, espaco).toInt();
    oper  = linha.substring(espaco + 1).toInt();
  }

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
// CARGA DE INSTRUCOES - MODO LOAD
// ============================================================

void processarEntradaLOAD(String entrada) {
  entrada.trim();
  if (entrada.length() == 0) return;

  if (tamPrograma >= MAX_PROG) {
    printErro(F("Memoria cheia! Maximo: 32 instrucoes."));
    return;
  }

  String instrucao = decodificarLinha(entrada);

  if (instrucao == "???") {
    Serial.print(F("  [ERR] Instrucao invalida: '"));
    Serial.print(entrada);
    Serial.println(F("' - verifique o opcode."));
    return;
  }

  programa[tamPrograma] = instrucao;
  Serial.print(F("  [OK] ["));
  if (tamPrograma < 10) Serial.print('0');
  Serial.print(tamPrograma);
  Serial.print(F("]  "));
  Serial.println(instrucao);
  tamPrograma++;
}

// ============================================================
// INICIAR RUN
// ============================================================

void iniciarRUN() {
  if (tamPrograma == 0) {
    printInfo(F("Nenhum programa carregado. Use # para LOAD."));
    return;
  }
  PC         = 0;
  EXECUTANDO = true;
  modoRUN    = true;

  printLinha('=', 52);
  Serial.print(F("  > MODO RUN  |  "));
  Serial.print(tamPrograma);
  Serial.println(F(" instrucoes  |  PC = 0"));
  printLinha('=', 52);
  Serial.println(F("  Pressione  *  para cada passo."));
  printLinha('-', 52);
}

// ============================================================
// CICLO DE INSTRUCAO - Fetch -> Encode -> Inc PC -> Decode -> Execute
// ============================================================

void executarInstrucao(String instrucao) {
  instrucao.trim();

  int    espaco   = instrucao.indexOf(' ');
  String mnem;
  int    operando = -1;

  if (espaco == -1) {
    mnem = instrucao;
  } else {
    mnem     = instrucao.substring(0, espaco);
    operando = instrucao.substring(espaco + 1).toInt();
  }

  // 1. CODIFICACAO -> IR
  IR = mnemonicoParaOpcode(mnem);

  // 2. Salva PC atual e incrementa
  int pcAtual = PC++;

  // 3. DECODIFICACAO + EXECUCAO
  switch (IR) {

    case OP_NOP:
      printInfo(F("NOP - nenhuma operacao."));
      break;

    case OP_READ: {
      int d = lerDistancia();
      ACC = d;
      sensorAtivo = true;
      Serial.print(F("  [READ]  -> "));
      Serial.print(d);
      Serial.println(F(" cm  -> ACC"));
      break;
    }

    case OP_LOADK:
      if (operando < 0) { printErro(F("LOADK requer operando.")); break; }
      ACC = operando;
      break;

    case OP_ADDK:
      if (operando < 0) { printErro(F("ADDK requer operando.")); break; }
      ACC += operando;
      break;

    case OP_SUBK:
      if (operando < 0) { printErro(F("SUBK requer operando.")); break; }
      ACC -= operando;
      break;

    case OP_CMPK:
      if (operando < 0) { printErro(F("CMPK requer operando.")); break; }
      FLAG_Z = (ACC == operando);
      Serial.print(F("  [CMPK]  ACC("));
      Serial.print(ACC); Serial.print(F(") == "));
      Serial.print(operando); Serial.print(F("  ->  FLAG_Z = "));
      Serial.println(FLAG_Z ? 1 : 0);
      break;

    case OP_LEDON:
      if (operando < 1 || operando > 3) {
        printErro(F("LEDON: operando deve ser 1, 2 ou 3.")); break;
      }
      ligarLED(operando);
      Serial.print(F("  [LEDON]  -> LED ")); Serial.print(operando);
      Serial.println(F("  LIGADO"));
      break;

    case OP_LEDOFF:
      if (operando < 1 || operando > 3) {
        printErro(F("LEDOFF: operando deve ser 1, 2 ou 3.")); break;
      }
      desligarLED(operando);
      Serial.print(F("  [LEDOFF] -> LED ")); Serial.print(operando);
      Serial.println(F("  DESLIGADO"));
      break;

    case OP_BUZON:
      ligarBuzzer();
      printOK(F("BUZON  ->  Buzzer LIGADO"));
      break;

    case OP_BUZOFF:
      desligarBuzzer();
      printOK(F("BUZOFF ->  Buzzer desligado"));
      break;

    case OP_DISP:
      if (operando >= 0) ACC = operando;
      mostrarDisplay(ACC);
      Serial.print(F("  [DISP]   ->  ACC = "));
      Serial.print(ACC);
      if      (ACC > 9) Serial.println(F("  [!] OVERFLOW -> exibe 'E'"));
      else if (ACC < 0) Serial.println(F("  [!] NEGATIVO  -> exibe '-'"));
      else              Serial.println();
      break;

    case OP_ALERT: {
      int d = lerDistancia();
      Serial.print(F("  [ALERT]  ->  "));
      Serial.print(d); Serial.print(F(" cm  -- "));
      if (d < 10) {
        ligarBuzzer();
        ligarLED(1); ligarLED(2); ligarLED(3);
        Serial.println(F("PERIGO!   Buzzer + LED 1/2/3 LIGADOS"));
      } else if (d < 20) {
        desligarBuzzer();
        ligarLED(1); ligarLED(2); ligarLED(3);
        Serial.println(F("ALERTA!   LED 1/2/3 LIGADOS"));
      } else {
        desligarBuzzer();
        desligarLED(1); desligarLED(2); desligarLED(3);
        Serial.println(F("SEGURO.   Todas as saidas desligadas"));
      }
      break;
    }

    case OP_BINC: {
      char bin[5];
      bin[0] = ((IR>>3)&1)?'1':'0';
      bin[1] = ((IR>>2)&1)?'1':'0';
      bin[2] = ((IR>>1)&1)?'1':'0';
      bin[3] = (IR&1)     ?'1':'0';
      bin[4] = '\0';
      Serial.print(F("  [BINC]   ->  IR = "));
      Serial.print(IR); Serial.print(F("  ("));
      Serial.print(bin); Serial.println(F(")"));
      break;
    }

    case OP_STORE:
      if (operando < 0 || operando > 15) {
        printErro(F("STORE: endereco invalido (0-15).")); break;
      }
      MEM[operando] = ACC;
      Serial.print(F("  [STORE]  ->  MEM[")); Serial.print(operando);
      Serial.print(F("] = ")); Serial.println(ACC);
      break;

    case OP_LOADM:
      if (operando < 0 || operando > 15) {
        printErro(F("LOADM: endereco invalido (0-15).")); break;
      }
      ACC = MEM[operando];
      Serial.print(F("  [LOADM]  ->  ACC = MEM[")); Serial.print(operando);
      Serial.print(F("] = ")); Serial.println(ACC);
      break;

    case OP_HALT:
      EXECUTANDO = false;
      modoRUN    = false;
      printLinha('=', 52);
      Serial.println(F("  [HALT] Execucao encerrada."));
      Serial.println(F("         Pressione  A  para reiniciar."));
      printLinha('=', 52);
      break;

    default:
      Serial.print(F("  [ERR] Instrucao desconhecida: "));
      Serial.println(instrucao);
      break;
  }

  // 4. EXIBICAO DO ESTADO DOS REGISTRADORES
  imprimirEstado(pcAtual, instrucao);

  // 5. Exibe memoria para STORE/LOADM
  if (IR == OP_STORE || IR == OP_LOADM) {
    Serial.print(F("     MEM[")); Serial.print(operando);
    Serial.print(F("] = ")); Serial.println(MEM[operando]);
  }

  printLinha('.', 52);
}

// ============================================================
// EXIBICAO DO ESTADO DOS REGISTRADORES
// ============================================================

void imprimirEstado(int pcExec, String instrExec) {
  char bin[5];
  bin[0] = ((IR>>3)&1)?'1':'0';
  bin[1] = ((IR>>2)&1)?'1':'0';
  bin[2] = ((IR>>1)&1)?'1':'0';
  bin[3] = (IR&1)     ?'1':'0';
  bin[4] = '\0';

  Serial.print(F("  PC: "));
  Serial.print(pcExec + 1);
  Serial.print(F(" | IR: "));
  Serial.print(instrExec);
  Serial.print(F(" ("));
  Serial.print(bin);
  Serial.print(F(") | ACC: "));
  Serial.print(ACC);
  Serial.print(F(" | FLAG_Z: "));
  Serial.println(FLAG_Z ? 1 : 0);
}

// ============================================================
// HELPERS DE FORMATACAO DO TERMINAL
// ============================================================

void printLinha(char c, int n) {
  Serial.print(F("  "));
  for (int i = 0; i < n; i++) Serial.print(c);
  Serial.println();
}

void printSecao(const __FlashStringHelper* titulo) {
  Serial.print(F("  >> "));
  Serial.println(titulo);
}

void printOK(const __FlashStringHelper* msg) {
  Serial.print(F("  [OK]   "));
  Serial.println(msg);
}

void printErro(const __FlashStringHelper* msg) {
  Serial.print(F("  [ERR]  "));
  Serial.println(msg);
}

void printInfo(const __FlashStringHelper* msg) {
  Serial.print(F("  [INFO] "));
  Serial.println(msg);
}

// ============================================================
// TELA DE AJUDA
// ============================================================

void imprimirAjuda() {
  Serial.println(F(""));
  Serial.println(F("  TECLAS DE CONTROLE"));
  Serial.println(F("  --------------------------------------------------"));
  Serial.println(F("   #   Entra / sai do modo LOAD"));
  Serial.println(F("   A   RUN - executa o programa carregado"));
  Serial.println(F("   *   Single-step - executa um passo"));
  Serial.println(F("   B   ENTER - confirma instrucao no LOAD"));
  Serial.println(F("   C   ESPACO - separa opcode do operando"));
  Serial.println(F("   D   BACKSPACE - apaga ultimo digito"));
  Serial.println(F(""));
  Serial.println(F("  EXEMPLOS DE ENTRADA (modo LOAD)"));
  Serial.println(F("  --------------------------------------------------"));
  Serial.println(F("   2 C 5 B  ->  LOADK 5   (ACC = 5)"));
  Serial.println(F("   3 C 3 B  ->  ADDK  3   (ACC += 3)"));
  Serial.println(F("   6 C 1 B  ->  LEDON 1   (liga LED 1)"));
  Serial.println(F("   1 B      ->  READ       (le sensor)"));
  Serial.println(F("   10 B     ->  DISP       (exibe ACC)"));
  Serial.println(F("   15 B     ->  HALT       (para)"));
  printLinha('=', 52);
  Serial.println(F("  Pressione  #  para comecar o LOAD."));
  printLinha('=', 52);
}
