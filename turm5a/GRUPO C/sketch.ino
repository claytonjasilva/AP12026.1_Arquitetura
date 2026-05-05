/*
202407106773 - Lucas Kronemberger - TA
202501000771 - Gabriel Militão Vilas Boas - TA
202501001483 - Matheus de Andrade Alvarenga - TA
202407155375 - João Vitor Rodrigues - TA
*/

#include <Keypad.h>

// ==============================================================================
// 1. ELEMENTOS ARQUITETURAIS OBRIGATÓRIOS
// ==============================================================================
int MEM[16];            // memória de dados simulada
int PC = 0;             // program counter (controla a instrução atual/próxima)
byte IR = 0;            // instruction register (armazena o opcode binário)
int ACC = 0;            // acumulador (armazena operandos e resultados)
bool FLAG_Z = false;    // flag de comparação
bool EXECUTANDO = true; // controle de execução do programa

// ==============================================================================
// 2. ESTRUTURAS DE MEMÓRIA DE PROGRAMA E ESTADOS
// ==============================================================================
struct Instrucao {
  byte opcode;
  int operando;
};

Instrucao memoriaPrograma[32]; // Armazena as instruções do usuário
int ponteiroCarga = 0;         // Endereço para carregar nova instrução

// Modos de operação do sistema
enum ModoOperacao { MODO_OCIOSO, MODO_LOAD, MODO_RUN };
ModoOperacao modoAtual = MODO_OCIOSO;

// Controle de digitação (Buffer)
String bufferEntrada = "";
bool aguardandoOperando = false;
byte opcodeTemp = 0;

// ==============================================================================
// 3. PINAGEM OBRIGATÓRIA E HARDWARE
// ==============================================================================
const int pinoA = 22, pinoB = 23, pinoC = 24, pinoD = 25;
const int pinoE = 26, pinoF = 27, pinoG = 28;

const byte LINHAS = 4;
const byte COLUNAS = 4;
char teclas[LINHAS][COLUNAS] = {
  {'1','2','3','A'}, // A = ENTER
  {'4','5','6','B'}, // B = RUN
  {'7','8','9','C'}, // C = CLEAR BUFFER
  {'*','0','#','D'}  // * = NEXT STEP, # = MODO LOAD
};
byte pinosLinhas[LINHAS] = {30, 31, 32, 33};
byte pinosColunas[COLUNAS] = {34, 35, 36, 37};
Keypad teclado = Keypad(makeKeymap(teclas), pinosLinhas, pinosColunas, LINHAS, COLUNAS);

const int pinoTrig = 40;
const int pinoEcho = 41;
const int pinoLed1 = 42; // LED de Alerta principal
const int pinoLed2 = 43;
const int pinoLed3 = 44;
const int pinoBuzzer = 45;

// ==============================================================================
// 4. TABELA DE MNEMÔNICOS E REGRAS
// ==============================================================================
String obterMnemonico(byte opcode) {
  switch(opcode) {
    case 0: return "NOP"; case 1: return "READ"; case 2: return "LOADK";
    case 3: return "ADDK"; case 4: return "SUBK"; case 5: return "CMPK";
    case 6: return "LEDON"; case 7: return "LEDOFF"; case 8: return "BUZON";
    case 9: return "BUZOFF"; case 10: return "DISP"; case 11: return "ALERT";
    case 12: return "BINC"; case 13: return "STORE"; case 14: return "LOADM";
    case 15: return "HALT"; default: return "???";
  }
}

bool precisaOperando(byte opcode) {
  // Retorna true se a instrução exige um segundo número (operando)
  return (opcode == 2 || opcode == 3 || opcode == 4 || opcode == 5 || 
          opcode == 6 || opcode == 7 || opcode == 13 || opcode == 14);
}

// ==============================================================================
// SETUP INICIAL
// ==============================================================================
void setup() {
  Serial.begin(9600);
  
  pinMode(pinoA, OUTPUT); pinMode(pinoB, OUTPUT); pinMode(pinoC, OUTPUT);
  pinMode(pinoD, OUTPUT); pinMode(pinoE, OUTPUT); pinMode(pinoF, OUTPUT);
  pinMode(pinoG, OUTPUT); 
  
  pinMode(pinoLed1, OUTPUT); pinMode(pinoLed2, OUTPUT); pinMode(pinoLed3, OUTPUT);
  pinMode(pinoBuzzer, OUTPUT);
  pinMode(pinoTrig, OUTPUT); pinMode(pinoEcho, INPUT);

  limparDisplay();
  
  Serial.println("=========================================");
  Serial.println("  SISTEMA INTERPRETADOR INICIALIZADO     ");
  Serial.println("=========================================");
  Serial.println("Aperte '#' para entrar no MODO LOAD.");
  Serial.println("Aperte 'B' ou digite 'RUN' para MODO RUN.");
}

// ==============================================================================
// LOOP PRINCIPAL
// ==============================================================================
void loop() {
  verificarComandosSerial();
  verificarTeclado();
}

// ==============================================================================
// FUNÇÕES DE ENTRADA (F01)
// ==============================================================================
void verificarComandosSerial() {
  if (Serial.available() > 0) {
    String comando = Serial.readStringUntil('\n');
    comando.trim();
    comando.toUpperCase();
    
    if (comando == "RUN" && modoAtual != MODO_LOAD) {
      iniciarModoRun();
    }
  }
}

void verificarTeclado() {
  char tecla = teclado.getKey();
  if (!tecla) return;

  if (tecla == '#') {
    alternarModoLoad();
    return;
  }

  if (tecla == 'B' && modoAtual != MODO_LOAD) {
    iniciarModoRun();
    return;
  }

  if (tecla == '*' && modoAtual == MODO_RUN && EXECUTANDO) {
    executarCicloDeInstrucao(); // Aciona o avanço do passo
    return;
  }

  if (modoAtual == MODO_LOAD) {
    processarDigitacaoLoad(tecla);
  }
}

void alternarModoLoad() {
  if (modoAtual == MODO_LOAD) {
    modoAtual = MODO_OCIOSO;
    Serial.println("\n[SISTEMA] Saiu do MODO LOAD. Programa armazenado.");
  } else {
    modoAtual = MODO_LOAD;
    ponteiroCarga = 0; // Limpa ponteiro de carga
    for (int i=0; i<32; i++) { memoriaPrograma[i].opcode = 0; memoriaPrograma[i].operando = 0; }
    bufferEntrada = "";
    aguardandoOperando = false;
    Serial.println("\n[SISTEMA] MODO LOAD INICIADO.");
    Serial.println("Digite o OPCODE, aperte 'A' (Enter). Digite OPERANDO (se houver), aperte 'A'.");
  }
}

void iniciarModoRun() {
  modoAtual = MODO_RUN;
  PC = 0;
  EXECUTANDO = true;
  Serial.println("\n[SISTEMA] MODO RUN INICIADO. PC = 0. Aguardando '*' para executar passo.");
}

void processarDigitacaoLoad(char tecla) {
  if (tecla >= '0' && tecla <= '9') {
    bufferEntrada += tecla;
    Serial.print(tecla);
  } 
  else if (tecla == 'C') {
    bufferEntrada = "";
    Serial.println("\n[LOAD] Buffer limpo.");
  }
  else if (tecla == 'A') { // ENTER
    if (bufferEntrada.length() == 0) return;
    
    int valorDigitado = bufferEntrada.toInt();
    bufferEntrada = "";
    Serial.println(); // Quebra linha
    
    if (!aguardandoOperando) {
      opcodeTemp = (byte)valorDigitado;
      if (precisaOperando(opcodeTemp)) {
        aguardandoOperando = true;
        Serial.print("[LOAD] Mnemônico: " + obterMnemonico(opcodeTemp) + " -> Aguardando Operando: ");
      } else {
        salvarInstrucaoMemoria(opcodeTemp, 0);
      }
    } else {
      salvarInstrucaoMemoria(opcodeTemp, valorDigitado);
      aguardandoOperando = false;
    }
  }
}

void salvarInstrucaoMemoria(byte op, int operando) {
  memoriaPrograma[ponteiroCarga].opcode = op;
  memoriaPrograma[ponteiroCarga].operando = operando;
  
  Serial.print("[LOAD] Salvo em P" + String(ponteiroCarga) + ": ");
  Serial.print(obterMnemonico(op));
  if (precisaOperando(op)) Serial.print(" " + String(operando));
  Serial.println();
  
  ponteiroCarga++;
}

// ==============================================================================
// UNIDADE DE CONTROLE (UC) - F03
// ==============================================================================
void executarCicloDeInstrucao() {
  // 1. Busca da Instrução
  IR = memoriaPrograma[PC].opcode;
  int OPND = memoriaPrograma[PC].operando;

  // 2 e 3. Decodificação e Execução
  switch (IR) {
    case 0: // NOP
      break;
    case 1: // READ (F05)
      ACC = medirDistancia();
      break;
    case 2: // LOADK
      ACC = OPND;
      break;
    case 3: // ADDK (F04 - ULA)
      ACC += OPND;
      break;
    case 4: // SUBK (F04 - ULA)
      ACC -= OPND;
      break;
    case 5: // CMPK (F04 - ULA)
      FLAG_Z = (ACC == OPND);
      break;
    case 6: // LEDON (F06)
      controlarLed(OPND, HIGH);
      break;
    case 7: // LEDOFF (F06)
      controlarLed(OPND, LOW);
      break;
    case 8: // BUZON (F07)
      digitalWrite(pinoBuzzer, HIGH);
      break;
    case 9: // BUZOFF (F07)
      digitalWrite(pinoBuzzer, LOW);
      break;
    case 10: // DISP (F08)
      exibirDisplay(ACC);
      break;
    case 11: // ALERT (F10)
      executarAlert();
      break;
    case 12: // BINC
      Serial.print("Binário do Opcode Atual: ");
      Serial.println(IR, BIN);
      break;
    case 13: // STORE (F09)
      if (OPND >= 0 && OPND < 16) MEM[OPND] = ACC;
      break;
    case 14: // LOADM (F09)
      if (OPND >= 0 && OPND < 16) ACC = MEM[OPND];
      break;
    case 15: // HALT (F11)
      EXECUTANDO = false;
      Serial.println("[HALT] Fim da Execução.");
      break;
  }

  // Apresentação do Estado (Seção 2.2.4)
  String repIR = obterMnemonico(IR);
  if (precisaOperando(IR)) repIR += " " + String(OPND);
  
  Serial.println("PC: " + String(PC) + " | IR: " + repIR + 
                 " | ACC: " + String(ACC) + " | FLAG_Z: " + String(FLAG_Z));

  // 4. Atualização do PC
  if (EXECUTANDO) PC++;
}

// ==============================================================================
// PROCESSAMENTO, HARDWARE E SAÍDAS (F05 a F10)
// ==============================================================================
int medirDistancia() {
  digitalWrite(pinoTrig, LOW); delayMicroseconds(2);
  digitalWrite(pinoTrig, HIGH); delayMicroseconds(10);
  digitalWrite(pinoTrig, LOW);
  long duracao = pulseIn(pinoEcho, HIGH, 30000); // timeout para evitar travar
  if (duracao == 0) return 99; // Retorno padrão caso não leia nada
  return duracao * 0.034 / 2;
}

void executarAlert() {
  int dist = medirDistancia();
  if (dist < 10) {
    digitalWrite(pinoBuzzer, HIGH);
    digitalWrite(pinoLed1, HIGH);
  } else if (dist >= 10 && dist < 20) {
    digitalWrite(pinoBuzzer, LOW);
    digitalWrite(pinoLed1, HIGH);
  } else {
    digitalWrite(pinoBuzzer, LOW);
    digitalWrite(pinoLed1, LOW);
  }
}

void controlarLed(int numLed, int estado) {
  if (numLed == 1) digitalWrite(pinoLed1, estado);
  else if (numLed == 2) digitalWrite(pinoLed2, estado);
  else if (numLed == 3) digitalWrite(pinoLed3, estado);
}

// Controle do Display 7 Segmentos e Tratamento de Erros (Seção 2.5)
void exibirDisplay(int valor) {
  // Matriz de segmentos: A, B, C, D, E, F, G
  const byte mapaDisplay[12][7] = {
    {1,1,1,1,1,1,0}, // 0
    {0,1,1,0,0,0,0}, // 1
    {1,1,0,1,1,0,1}, // 2
    {1,1,1,1,0,0,1}, // 3
    {0,1,1,0,0,1,1}, // 4
    {1,0,1,1,0,1,1}, // 5
    {1,0,1,1,1,1,1}, // 6
    {1,1,1,0,0,0,0}, // 7
    {1,1,1,1,1,1,1}, // 8
    {1,1,1,1,0,1,1}, // 9
    {1,0,0,1,1,1,1}, // 10 -> 'E' (Erro / Overflow)
    {0,0,0,0,0,0,1}  // 11 -> '-' (Negativo)
  };

  int indiceMapa;

  if (valor > 9) {
    indiceMapa = 10; // 'E'
    Serial.println(">> ALERTA: OVERFLOW DETECTADO (Valor > 9)");
  } 
  else if (valor < 0) {
    indiceMapa = 11; // '-'
    Serial.println(">> ALERTA: VALOR NEGATIVO DETECTADO (< 0)");
  } 
  else {
    indiceMapa = valor;
  }

  digitalWrite(pinoA, mapaDisplay[indiceMapa][0]);
  digitalWrite(pinoB, mapaDisplay[indiceMapa][1]);
  digitalWrite(pinoC, mapaDisplay[indiceMapa][2]);
  digitalWrite(pinoD, mapaDisplay[indiceMapa][3]);
  digitalWrite(pinoE, mapaDisplay[indiceMapa][4]);
  digitalWrite(pinoF, mapaDisplay[indiceMapa][5]);
  digitalWrite(pinoG, mapaDisplay[indiceMapa][6]);
}

void limparDisplay() {
  digitalWrite(pinoA, 0); digitalWrite(pinoB, 0); digitalWrite(pinoC, 0);
  digitalWrite(pinoD, 0); digitalWrite(pinoE, 0); digitalWrite(pinoF, 0);
  digitalWrite(pinoG, 0);
}