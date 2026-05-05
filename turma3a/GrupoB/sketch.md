```cpp
/* Victor Alvarenga Hwang - 202208766005 - TA
Arthur Calebe Carvalho Da Silva - 202508449013 - TA
Raí Lamper de Avila - 202402627279 - TA
Matheus Cocenzo e Silva - 202107291052 - TA

Projeto: Sistema Interpretador de Instruções com Sensor de Distância e Atuadores
Placa: Arduino Mega 2560
*/

#include <Keypad.h>

const int segmentPins[7] = {22, 23, 24, 25, 27, 29, 31};

const int digitSegments[10][7] = {
  {1, 1, 1, 1, 1, 1, 0},
  {0, 1, 1, 0, 0, 0, 0},
  {1, 1, 0, 1, 1, 0, 1},
  {1, 1, 1, 1, 0, 0, 1},
  {0, 1, 1, 0, 0, 1, 1},
  {1, 0, 1, 1, 0, 1, 1},
  {1, 0, 1, 1, 1, 1, 1},
  {1, 1, 1, 0, 0, 0, 0},
  {1, 1, 1, 1, 1, 1, 1},
  {1, 1, 1, 1, 0, 1, 1}
};

int MEM[16];
int PC = 0;
byte IR = 0;
int ACC = 0;
bool FLAG_Z = false;
bool EXECUTANDO = true;

String PROGRAMA[16];
int TAM_PROGRAMA = 0;
int PONTEIRO_CARGA = 0;

bool MODO_LOAD = false;
bool MODO_RUN = false;

String bufferInstrucao = "";

const int PIN_TRIG = 41;
const int PIN_ECHO = 42;

const int LED1 = 2;
const int LED2 = 3;
const int LED3 = 4;

const int BUZZER = 45;

const byte LINHAS = 4;
const byte COLUNAS = 4;

char teclas[LINHAS][COLUNAS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pinosLinhas[LINHAS] = {26, 28, 30, 32};
byte pinosColunas[COLUNAS] = {34, 36, 38, 40};

Keypad teclado = Keypad(makeKeymap(teclas), pinosLinhas, pinosColunas, LINHAS, COLUNAS);

struct InstrucaoDecodificada {
  int opcode;
  int operando;
  bool temOperando;
  String textoOriginal;
};

void inicializarHardware();
void inicializarSistema();
void limparPrograma();
void limparMemoriaDados();
void processarTecla(char tecla);
void entrarModoLoad();
void sairModoLoad();
void iniciarRun();
bool armazenarInstrucao(String texto);
InstrucaoDecodificada decodificarInstrucao(String texto);
String nomeMneMonico(int opcode);
String opcodeBinario4Bits(int opcode);
bool opcodeExigeOperando(int opcode);
void executarProximaInstrucao();
void executarInstrucao(InstrucaoDecodificada inst);
void mostrarEstadoSerial(String instrucaoExecutada);
void mostrarProgramaSerial();
long lerDistanciaCM();
void ligarLED(int numero);
void desligarLED(int numero);
void desligarTodosLEDs();
void ligarBuzzer();
void desligarBuzzer();
void tratarInstrucaoALERT();
void exibirOpcodeAtualBinario();
void mostrarNumeroDisplay(int numero);
void mostrarErroDisplay();
void mostrarNegativoDisplay();
void apagarDisplay();
void mostrarACCNoDisplay();

void setup() {
  Serial.begin(9600);
  inicializarHardware();
  inicializarSistema();

  Serial.println(F("=============================================="));
  Serial.println(F("Sistema Interpretador de Instrucoes - Arduino"));
  Serial.println(F("Versao com display de 7 segmentos"));
  Serial.println(F("----------------------------------------------"));
  Serial.println(F("Teclas de controle:"));
  Serial.println(F("# -> entra/sai do modo LOAD"));
  Serial.println(F("A -> confirma e armazena a instrucao"));
  Serial.println(F("B -> RUN"));
  Serial.println(F("* -> executa 1 instrucao"));
  Serial.println(F("C -> apaga ultimo caractere"));
  Serial.println(F("D -> separador entre opcode e operando"));
  Serial.println(F("----------------------------------------------"));
  Serial.println(F("Exemplos de digitacao no teclado:"));
  Serial.println(F("LOADK 5 -> 2 D 5 A"));
  Serial.println(F("ADDK 3 -> 3 D 3 A"));
  Serial.println(F("DISP -> 1 0 A"));
  Serial.println(F("HALT -> 1 5 A"));
  Serial.println(F("=============================================="));
}

void loop() {
  char tecla = teclado.getKey();

  if (tecla) {
    processarTecla(tecla);
  }
}

void inicializarHardware() {
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  pinMode(BUZZER, OUTPUT);

  for (int i = 0; i < 7; i++) {
    pinMode(segmentPins[i], OUTPUT);
  }

  desligarTodosLEDs();
  desligarBuzzer();
  apagarDisplay();
}

void inicializarSistema() {
  limparMemoriaDados();
  limparPrograma();

  PC = 0;
  IR = 0;
  ACC = 0;
  FLAG_Z = false;
  EXECUTANDO = true;

  MODO_LOAD = false;
  MODO_RUN = false;

  bufferInstrucao = "";
}

void limparMemoriaDados() {
  for (int i = 0; i < 16; i++) {
    MEM[i] = 0;
  }
}

void limparPrograma() {
  for (int i = 0; i < 16; i++) {
    PROGRAMA[i] = "";
  }

  TAM_PROGRAMA = 0;
  PONTEIRO_CARGA = 0;
}

void processarTecla(char tecla) {
  Serial.print(F("Tecla: "));
  Serial.println(tecla);

  if (tecla == '#') {
    if (!MODO_LOAD) {
      entrarModoLoad();
    } else {
      sairModoLoad();
    }
    return;
  }

  if (tecla == 'B') {
    iniciarRun();
    return;
  }

  if (tecla == '*') {
    if (MODO_RUN) {
      executarProximaInstrucao();
    } else {
      Serial.println(F("Nao esta em modo RUN. Pressione B para iniciar."));
    }
    return;
  }

  if (!MODO_LOAD) {
    Serial.println(F("Tecla ignorada. Entre em LOAD com # ou use B/*."));
    return;
  }

  if (tecla == 'A') {
    if (bufferInstrucao.length() == 0) {
      Serial.println(F("Nenhuma instrucao digitada."));
      return;
    }

    if (armazenarInstrucao(bufferInstrucao)) {
      bufferInstrucao = "";
    }
    return;
  }

  if (tecla == 'C') {
    if (bufferInstrucao.length() > 0) {
      bufferInstrucao.remove(bufferInstrucao.length() - 1);
      Serial.print(F("Buffer atual: "));
      Serial.println(bufferInstrucao);
    }
    return;
  }

  if (tecla == 'D') {
    bufferInstrucao += ' ';
    Serial.print(F("Buffer atual: "));
    Serial.println(bufferInstrucao);
    return;
  }

  if (tecla >= '0' && tecla <= '9') {
    bufferInstrucao += tecla;
    Serial.print(F("Buffer atual: "));
    Serial.println(bufferInstrucao);
    return;
  }

  Serial.println(F("Tecla nao utilizada neste modo."));
}

void entrarModoLoad() {
  MODO_LOAD = true;
  MODO_RUN = false;
  EXECUTANDO = true;
  bufferInstrucao = "";

  limparPrograma();

  PC = 0;

  Serial.println(F("===== MODO LOAD ATIVADO ====="));
  Serial.println(F("Programa limpo."));
  Serial.println(F("Digite instrucoes em decimal."));
  Serial.println(F("Use D como separador e A para confirmar."));
}

void sairModoLoad() {
  MODO_LOAD = false;
  bufferInstrucao = "";

  Serial.println(F("===== MODO LOAD ENCERRADO ====="));
  Serial.print(F("Total de instrucoes armazenadas: "));
  Serial.println(TAM_PROGRAMA);

  mostrarProgramaSerial();
}

void iniciarRun() {
  if (TAM_PROGRAMA == 0) {
    Serial.println(F("Nao ha programa carregado. Use # para entrar em LOAD."));
    return;
  }

  MODO_RUN = true;
  MODO_LOAD = false;
  PC = 0;
  EXECUTANDO = true;

  Serial.println(F("===== MODO RUN INICIADO ====="));
  Serial.println(F("Pressione * para executar uma instrucao por vez."));
}

bool armazenarInstrucao(String texto) {
  if (PONTEIRO_CARGA >= 16) {
    Serial.println(F("Erro: memoria de programa cheia (max 16 instrucoes)."));
    return false;
  }

  texto.trim();

  InstrucaoDecodificada inst = decodificarInstrucao(texto);

  if (inst.opcode < 0 || inst.opcode > 15) {
    Serial.println(F("Erro: opcode invalido."));
    return false;
  }

  if (opcodeExigeOperando(inst.opcode) && !inst.temOperando) {
    Serial.println(F("Erro: esta instrucao exige operando."));
    return false;
  }

  if (!opcodeExigeOperando(inst.opcode) && inst.temOperando) {
    Serial.println(F("Aviso: operando sera ignorado para esta instrucao."));
  }

  PROGRAMA[PONTEIRO_CARGA] = texto;
  TAM_PROGRAMA++;
  PONTEIRO_CARGA++;

  Serial.print(F("Instrucao armazenada em ["));
  Serial.print(PONTEIRO_CARGA - 1);
  Serial.print(F("]: "));
  Serial.print(nomeMneMonico(inst.opcode));

  if (inst.temOperando && opcodeExigeOperando(inst.opcode)) {
    Serial.print(F(" "));
    Serial.print(inst.operando);
  }

  Serial.print(F(" | opcode binario: "));
  Serial.println(opcodeBinario4Bits(inst.opcode));

  return true;
}

InstrucaoDecodificada decodificarInstrucao(String texto) {
  InstrucaoDecodificada inst;

  inst.opcode = -1;
  inst.operando = 0;
  inst.temOperando = false;
  inst.textoOriginal = texto;

  texto.trim();

  int posEspaco = texto.indexOf(' ');

  if (posEspaco == -1) {
    inst.opcode = texto.toInt();
    inst.temOperando = false;
  } else {
    String parteOpcode = texto.substring(0, posEspaco);
    String parteOperando = texto.substring(posEspaco + 1);

    parteOperando.trim();

    inst.opcode = parteOpcode.toInt();
    inst.operando = parteOperando.toInt();
    inst.temOperando = true;
  }

  return inst;
}

String nomeMneMonico(int opcode) {
  switch (opcode) {
    case 0: return "NOP";
    case 1: return "READ";
    case 2: return "LOADK";
    case 3: return "ADDK";
    case 4: return "SUBK";
    case 5: return "CMPK";
    case 6: return "LEDON";
    case 7: return "LEDOFF";
    case 8: return "BUZON";
    case 9: return "BUZOFF";
    case 10: return "DISP";
    case 11: return "ALERT";
    case 12: return "BINC";
    case 13: return "STORE";
    case 14: return "LOADM";
    case 15: return "HALT";
    default: return "INVALIDA";
  }
}

String opcodeBinario4Bits(int opcode) {
  String s = "";

  for (int i = 3; i >= 0; i--) {
    s += ((opcode >> i) & 1) ? '1' : '0';
  }

  return s;
}

bool opcodeExigeOperando(int opcode) {
  switch (opcode) {
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 13:
    case 14:
      return true;
    default:
      return false;
  }
}

void executarProximaInstrucao() {
  if (!EXECUTANDO) {
    Serial.println(F("Programa encerrado por HALT. Pressione B para RUN novamente."));
    return;
  }

  if (PC < 0 || PC >= TAM_PROGRAMA) {
    Serial.println(F("PC fora do limite do programa. Execucao encerrada."));
    EXECUTANDO = false;
    MODO_RUN = false;
    return;
  }

  String instrucaoTexto = PROGRAMA[PC];

  InstrucaoDecodificada inst = decodificarInstrucao(instrucaoTexto);

  IR = (byte)inst.opcode;

  executarInstrucao(inst);

  String nomeInstrucao = nomeMneMonico(inst.opcode);

  if (inst.temOperando && opcodeExigeOperando(inst.opcode)) {
    nomeInstrucao += " " + String(inst.operando);
  }

  mostrarEstadoSerial(nomeInstrucao);

  if (EXECUTANDO) {
    PC = PC + 1;
  }
}

void executarInstrucao(InstrucaoDecodificada inst) {
  switch (inst.opcode) {
    case 0:
      break;

    case 1:
      ACC = (int)lerDistanciaCM();
      break;

    case 2:
      ACC = inst.operando;
      break;

    case 3:
      ACC = ACC + inst.operando;
      break;

    case 4:
      ACC = ACC - inst.operando;
      break;

    case 5:
      FLAG_Z = (ACC == inst.operando);
      break;

    case 6:
      ligarLED(inst.operando);
      break;

    case 7:
      desligarLED(inst.operando);
      break;

    case 8:
      ligarBuzzer();
      break;

    case 9:
      desligarBuzzer();
      break;

    case 10:
      Serial.print(F("DISP -> ACC = "));
      Serial.println(ACC);
      mostrarACCNoDisplay();
      break;

    case 11:
      tratarInstrucaoALERT();
      break;

    case 12:
      exibirOpcodeAtualBinario();
      break;

    case 13:
      if (inst.operando >= 0 && inst.operando < 16) {
        MEM[inst.operando] = ACC;

        Serial.print(F("STORE -> MEM["));
        Serial.print(inst.operando);
        Serial.print(F("] = "));
        Serial.println(ACC);
      } else {
        Serial.println(F("Erro: endereco de memoria invalido em STORE."));
      }
      break;

    case 14:
      if (inst.operando >= 0 && inst.operando < 16) {
        ACC = MEM[inst.operando];

        Serial.print(F("LOADM -> ACC recebeu MEM["));
        Serial.print(inst.operando);
        Serial.print(F("] = "));
        Serial.println(ACC);
      } else {
        Serial.println(F("Erro: endereco de memoria invalido em LOADM."));
      }
      break;

    case 15:
      EXECUTANDO = false;
      MODO_RUN = false;
      Serial.println(F("HALT processado. Execucao encerrada."));
      break;

    default:
      Serial.println(F("Erro: opcode invalido."));
      EXECUTANDO = false;
      MODO_RUN = false;
      break;
  }
}

void mostrarEstadoSerial(String instrucaoExecutada) {
  Serial.println(F("----------------------------------------------"));

  Serial.print(F("PC: "));
  Serial.print(PC);

  Serial.print(F(" | IR: "));
  Serial.print(instrucaoExecutada);

  Serial.print(F(" | ACC: "));
  Serial.print(ACC);

  Serial.print(F(" | FLAG_Z: "));
  Serial.println(FLAG_Z ? 1 : 0);

  Serial.print(F("IR opcode binario: "));
  Serial.println(opcodeBinario4Bits(IR));

  Serial.print(F("MEM: "));

  for (int i = 0; i < 16; i++) {
    Serial.print(F("["));
    Serial.print(i);
    Serial.print(F("]="));
    Serial.print(MEM[i]);

    if (i < 15) {
      Serial.print(F(" "));
    }
  }

  Serial.println();
  Serial.println(F("----------------------------------------------"));
}

void mostrarProgramaSerial() {
  Serial.println(F("Programa armazenado:"));

  for (int i = 0; i < TAM_PROGRAMA; i++) {
    InstrucaoDecodificada inst = decodificarInstrucao(PROGRAMA[i]);

    Serial.print(F("["));
    Serial.print(i);
    Serial.print(F("] "));
    Serial.print(nomeMneMonico(inst.opcode));

    if (inst.temOperando && opcodeExigeOperando(inst.opcode)) {
      Serial.print(F(" "));
      Serial.print(inst.operando);
    }

    Serial.print(F(" | opcode: "));
    Serial.println(opcodeBinario4Bits(inst.opcode));
  }
}

long lerDistanciaCM() {
  long duracao;
  float distancia;

  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(5);

  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);

  digitalWrite(PIN_TRIG, LOW);

  duracao = pulseIn(PIN_ECHO, HIGH, 30000);

  if (duracao == 0) {
    Serial.println(F("READ: sem retorno do sensor."));
    return -1;
  }

  distancia = duracao * 0.0343 / 2.0;

  Serial.print(F("READ -> distancia lida: "));
  Serial.print(distancia);
  Serial.println(F(" cm"));

  return (long)distancia;
}

void ligarLED(int numero) {
  switch (numero) {
    case 1:
      digitalWrite(LED1, HIGH);
      Serial.println(F("LED 1 ligado."));
      break;

    case 2:
      digitalWrite(LED2, HIGH);
      Serial.println(F("LED 2 ligado."));
      break;

    case 3:
      digitalWrite(LED3, HIGH);
      Serial.println(F("LED 3 ligado."));
      break;

    default:
      Serial.println(F("Erro: LED invalido. Use 1, 2 ou 3."));
      break;
  }
}

void desligarLED(int numero) {
  switch (numero) {
    case 1:
      digitalWrite(LED1, LOW);
      Serial.println(F("LED 1 desligado."));
      break;

    case 2:
      digitalWrite(LED2, LOW);
      Serial.println(F("LED 2 desligado."));
      break;

    case 3:
      digitalWrite(LED3, LOW);
      Serial.println(F("LED 3 desligado."));
      break;

    default:
      Serial.println(F("Erro: LED invalido. Use 1, 2 ou 3."));
      break;
  }
}

void desligarTodosLEDs() {
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
}

void ligarBuzzer() {
  digitalWrite(BUZZER, HIGH);
  Serial.println(F("Buzzer ligado."));
}

void desligarBuzzer() {
  digitalWrite(BUZZER, LOW);
  Serial.println(F("Buzzer desligado."));
}

void tratarInstrucaoALERT() {
  long distancia = lerDistanciaCM();

  if (distancia < 0) {
    Serial.println(F("ALERT: leitura invalida do sensor."));
    desligarTodosLEDs();
    desligarBuzzer();
    return;
  }

  if (distancia < 10) {
    Serial.println(F("ALERT: distancia < 10 cm -> LED de alerta + buzzer"));
    ligarLED(1);
    desligarLED(2);
    desligarLED(3);
    ligarBuzzer();
  } else if (distancia < 20) {
    Serial.println(F("ALERT: 10 cm <= distancia < 20 cm -> LED de alerta, sem buzzer"));
    ligarLED(1);
    desligarLED(2);
    desligarLED(3);
    desligarBuzzer();
  } else {
    Serial.println(F("ALERT: distancia >= 20 cm -> alertas desligados"));
    desligarLED(1);
    desligarLED(2);
    desligarLED(3);
    desligarBuzzer();
  }
}

void exibirOpcodeAtualBinario() {
  Serial.print(F("Opcode binario da instrucao atual (IR): "));
  Serial.println(opcodeBinario4Bits(IR));
}

void mostrarNumeroDisplay(int numero) {
  for (int i = 0; i < 7; i++) {
    digitalWrite(segmentPins[i], digitSegments[numero][i]);
  }
}

void mostrarErroDisplay() {
  digitalWrite(segmentPins[0], HIGH);
  digitalWrite(segmentPins[1], LOW);
  digitalWrite(segmentPins[2], LOW);
  digitalWrite(segmentPins[3], HIGH);
  digitalWrite(segmentPins[4], HIGH);
  digitalWrite(segmentPins[5], HIGH);
  digitalWrite(segmentPins[6], HIGH);
}

void mostrarNegativoDisplay() {
  digitalWrite(segmentPins[0], LOW);
  digitalWrite(segmentPins[1], LOW);
  digitalWrite(segmentPins[2], LOW);
  digitalWrite(segmentPins[3], LOW);
  digitalWrite(segmentPins[4], LOW);
  digitalWrite(segmentPins[5], LOW);
  digitalWrite(segmentPins[6], HIGH);
}

void apagarDisplay() {
  for (int i = 0; i < 7; i++) {
    digitalWrite(segmentPins[i], LOW);
  }
}

void mostrarACCNoDisplay() {
  if (ACC > 9) {
    mostrarErroDisplay();
    Serial.println(F("Overflow detectado (valor > 9)."));
  } else if (ACC < 0) {
    mostrarNegativoDisplay();
    Serial.println(F("Valor negativo detectado."));
  } else {
    mostrarNumeroDisplay(ACC);
  }
}