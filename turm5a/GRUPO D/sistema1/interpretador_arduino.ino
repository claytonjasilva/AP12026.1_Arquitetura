/*
  ============================================================
  PROJETO: Sistema Interpretador de Instruções com Sensor de
           Distância e Atuadores — Arduino Mega 2560
  ============================================================

  Identificação do grupo:
  00001 Vitor M Freire - TA
  00002 Vitor Luiz Z - TA
  00003 Gabriel Almeida - TA
  00004 Filipe - TA

  Disciplina: Arquitetura de Computadores
  ============================================================

  ELEMENTOS ARQUITETURAIS IMPLEMENTADOS:
  - MEM[16]    → Memória de dados simulada
  - PC         → Program Counter (Contador de Programa)
  - IR         → Instruction Register (Registrador de Instrução)
  - ACC        → Acumulador
  - FLAG_Z     → Flag de comparação (zero flag)
  - EXECUTANDO → Controle de estado do ciclo de execução
  - programa[] → Memória de programa (armazena instruções no modo LOAD)
  - UC         → Unidade de Controle (funções: buscarInstrucao, decodificar, executar)
  - ULA        → Unidade Lógica e Aritmética (função: operacaoULA)
  ============================================================
*/

#include <Keypad.h>

// ============================================================
// PINAGEM — conforme especificação obrigatória do projeto
// ============================================================

// Display de 7 segmentos (anodo comum ou catodo comum — ajuste SEG_LIGADO se necessário)
#define PIN_SEG_A 22
#define PIN_SEG_B 23
#define PIN_SEG_C 24
#define PIN_SEG_D 25
#define PIN_SEG_E 26
#define PIN_SEG_F 27
#define PIN_SEG_G 28
#define PIN_SEG_DP 29 // ponto decimal (opcional)

// Teclado matricial 4x4
#define PIN_LINHA1 30
#define PIN_LINHA2 31
#define PIN_LINHA3 32
#define PIN_LINHA4 33
#define PIN_COL1 34
#define PIN_COL2 35
#define PIN_COL3 36
#define PIN_COL4 37

// Sensor ultrassônico HC-SR04
#define PIN_TRIG 40
#define PIN_ECHO 41

// LEDs
#define PIN_LED1 42
#define PIN_LED2 43
#define PIN_LED3 44

// Buzzer
#define PIN_BUZZER 45

// ============================================================
// CONFIGURAÇÃO DO TECLADO MATRICIAL
// ============================================================

const byte LINHAS = 4;
const byte COLUNAS = 4;

char mapasTeclas[LINHAS][COLUNAS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

byte pinosLinhas[LINHAS] = {PIN_LINHA1, PIN_LINHA2, PIN_LINHA3, PIN_LINHA4};
byte pinosColunas[COLUNAS] = {PIN_COL1, PIN_COL2, PIN_COL3, PIN_COL4};

Keypad teclado = Keypad(makeKeymap(mapasTeclas), pinosLinhas, pinosColunas, LINHAS, COLUNAS);

// ============================================================
// REGISTRADORES E MEMÓRIA SIMULADA (elementos arquiteturais)
// ============================================================

int MEM[16];             // Memória de dados simulada (F09)
int PC = 0;              // Program Counter — aponta para a próxima instrução a executar
byte IR = 0;             // Instruction Register — armazena o opcode da instrução corrente
byte IR_ANTERIOR = 0;    // Registrador para armazenar o opcode da instrução anterior (para comparação em BINC)
int ACC = 0;             // Acumulador — operandos e resultados de operações
bool FLAG_Z = false;     // Flag de zero — resultado de comparação (CMPK)
bool EXECUTANDO = false; // Controle: true = modo RUN ativo, false = parado

// ============================================================
// MEMÓRIA DE PROGRAMA
// ============================================================

#define MAX_INSTRUCOES 32
String programa[MAX_INSTRUCOES]; // Memória de programa: armazena instruções no formato "opcode operando"
int totalInstrucoes = 0;         // Quantidade de instruções carregadas

// ============================================================
// MODOS DE OPERAÇÃO
// ============================================================

bool modoLOAD = false; // true quando o sistema está em modo de entrada de programa

// ============================================================
// TABELA DE OPCODES — ISA do projeto
// ============================================================
// Decimal | Opcode | Mnemônico
//   0     | 0000   | NOP
//   1     | 0001   | READ
//   2     | 0010   | LOADK  <k>
//   3     | 0011   | ADDK   <k>
//   4     | 0100   | SUBK   <k>
//   5     | 0101   | CMPK   <k>
//   6     | 0110   | LEDON  <led>
//   7     | 0111   | LEDOFF <led>
//   8     | 1000   | BUZON
//   9     | 1001   | BUZOFF
//  10     | 1010   | DISP
//  11     | 1011   | ALERT
//  12     | 1100   | BINC
//  13     | 1101   | STORE  <addr>
//  14     | 1110   | LOADM  <addr>
//  15     | 1111   | HALT

const char *MNEMONICOS[] = {
    "NOP", "READ", "LOADK", "ADDK", "SUBK", "CMPK",
    "LEDON", "LEDOFF", "BUZON", "BUZOFF", "DISP",
    "ALERT", "BINC", "STORE", "LOADM", "HALT"};

// Padrões de segmento para display de 7 segmentos (catodo comum)
// Ordem dos bits: a b c d e f g  (pinos 22–28)
// 1 = segmento ligado, 0 = desligado
const byte SEG_LIGADO = HIGH;
const byte SEG_APAGADO = LOW;

// Padrões para dígitos 0–9, letra E (erro/overflow) e traço - (negativo)
//                            a  b  c  d  e  f  g
const byte PADROES[13][7] = {
    {1, 1, 1, 1, 1, 1, 0}, // 0
    {0, 1, 1, 0, 0, 0, 0}, // 1
    {1, 1, 0, 1, 1, 0, 1}, // 2
    {1, 1, 1, 1, 0, 0, 1}, // 3
    {0, 1, 1, 0, 0, 1, 1}, // 4
    {1, 0, 1, 1, 0, 1, 1}, // 5
    {1, 0, 1, 1, 1, 1, 1}, // 6
    {1, 1, 1, 0, 0, 0, 0}, // 7
    {1, 1, 1, 1, 1, 1, 1}, // 8
    {1, 1, 1, 1, 0, 1, 1}, // 9
    {1, 0, 0, 1, 1, 1, 1}, // E  (índice 10 — overflow)
    {0, 0, 0, 0, 0, 0, 1}, // -  (índice 11 — negativo)
    {0, 0, 0, 0, 0, 0, 0}, // apagado (índice 12)
};

// Pinos dos segmentos em ordem a–g
const byte PINOS_SEG[7] = {
    PIN_SEG_A, PIN_SEG_B, PIN_SEG_C,
    PIN_SEG_D, PIN_SEG_E, PIN_SEG_F, PIN_SEG_G};

// Buffer de entrada do teclado
String bufferEntrada = "";

// ============================================================
// SETUP
// ============================================================

void setup()
{
  Serial.begin(9600);

  // Pinos do display
  for (int i = 0; i < 7; i++)
  {
    pinMode(PINOS_SEG[i], OUTPUT);
  }
  pinMode(PIN_SEG_DP, OUTPUT);
  digitalWrite(PIN_SEG_DP, SEG_APAGADO);

  // Pinos dos LEDs
  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  pinMode(PIN_LED3, OUTPUT);

  // Pino do buzzer
  pinMode(PIN_BUZZER, OUTPUT);

  // Pinos do sensor ultrassônico
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  // Estado inicial de saídas
  desligarTodasSaidas();
  exibirDisplay(12); // display apagado

  Serial.println(F("=============================================="));
  Serial.println(F("  Interpretador de Instrucoes - Arduino Mega"));
  Serial.println(F("=============================================="));
  Serial.println(F("Comandos disponiveis:"));
  Serial.println(F("  # -> Entra/sai do modo LOAD"));
  Serial.println(F("  No modo LOAD: digite 'opcode [operando]' + A para confirmar"));
  Serial.println(F("  RUN -> Inicia modo de execucao (via Serial)"));
  Serial.println(F("  '*' -> Inicia modo de execucao (via Teclado)"));
  Serial.println(F("  '*' -> Executa proxima instrucao (modo RUN)"));
  Serial.println(F("==============================================\n"));
}

// ============================================================
// LOOP PRINCIPAL
// ============================================================

void loop()
{
  // Leitura do teclado matricial
  char tecla = teclado.getKey();
  if (tecla)
  {
    processarTecla(tecla);
  }

  // Leitura do Serial Monitor (para comando RUN digitado como texto)
  if (Serial.available())
  {
    String cmdSerial = Serial.readStringUntil('\n');
    cmdSerial.trim();
    cmdSerial.toUpperCase();
    if (cmdSerial == "RUN")
    {
      iniciarExecucao();
    }
  }
}

// ============================================================
// F01 — ENTRADA DE INSTRUÇÕES (processamento do teclado)
// ============================================================

void processarTecla(char tecla)
{
  // Tecla '#' alterna o modo LOAD
  if (tecla == '#')
  {
    if (!modoLOAD)
    {
      entrarModoLOAD();
    }
    else
    {
      sairModoLOAD();
    }
    return;
  }

  // Tecla '*' no modo RUN executa uma instrução
  if (tecla == '*')
  {
    if (!EXECUTANDO && !modoLOAD)
    {
      iniciarExecucao();
    }
    else if (EXECUTANDO)
    {
      executarProximaInstrucao();
    }
    else
    {
      Serial.println(F("[AVISO] Sistema nao esta em modo RUN. Pressione '*' no teclado ou digite RUN no Serial Monitor."));
    }
    return;
  }

  // No modo LOAD: acumular dígitos e confirmar com 'A'
  if (modoLOAD)
  {
    if (tecla == 'A')
    {
      // Confirmar instrução
      confirmarInstrucao();
    }
    else if (tecla == 'B')
    {
      // 'B' funciona como backspace
      if (bufferEntrada.length() > 0)
      {
        bufferEntrada.remove(bufferEntrada.length() - 1);
        Serial.print(F("Buffer: ["));
        Serial.print(bufferEntrada);
        Serial.println(F("]"));
      }
    }
    else if (tecla == 'C')
    {
      // 'C' limpa o buffer atual
      bufferEntrada = "";
      Serial.println(F("Buffer limpo."));
    }
    else if (tecla == 'D')
    {
      // 'D' é usado como separador de opcode/operando (espaço lógico)
      bufferEntrada += " ";
      Serial.print(F("Buffer: ["));
      Serial.print(bufferEntrada);
      Serial.println(F("]"));
    }
    else
    {
      // Dígitos e outros caracteres
      bufferEntrada += tecla;
      Serial.print(F("Buffer: ["));
      Serial.print(bufferEntrada);
      Serial.println(F("] (A=confirmar, B=backspace, C=limpar, D=espaco)"));
    }
  }
}

// ============================================================
// MODO LOAD
// ============================================================

void entrarModoLOAD()
{
  modoLOAD = true;
  EXECUTANDO = false;

  // Limpar memória de programa
  totalInstrucoes = 0;
  for (int i = 0; i < MAX_INSTRUCOES; i++)
  {
    programa[i] = "";
  }
  bufferEntrada = "";

  Serial.println(F("\n--- MODO LOAD ATIVADO ---"));
  Serial.println(F("Digite instrucoes no formato: <opcode_decimal> [operando]"));
  Serial.println(F("Use 'D' como espaco, 'A' para confirmar, '#' para sair do LOAD"));
  Serial.println(F("Exemplo: LOADK 5 → tecle '2', 'D', '5', 'A'"));
  Serial.println(F("Opcodes: 0=NOP 1=READ 2=LOADK 3=ADDK 4=SUBK 5=CMPK 6=LEDON"));
  Serial.println(F("         7=LEDOFF 8=BUZON 9=BUZOFF 10=DISP 11=ALERT"));
  Serial.println(F("         12=BINC 13=STORE 14=LOADM 15=HALT"));
}

void sairModoLOAD()
{
  modoLOAD = false;
  bufferEntrada = "";
  Serial.println(F("\n--- MODO LOAD ENCERRADO ---"));
  Serial.print(F("Total de instrucoes carregadas: "));
  Serial.println(totalInstrucoes);
  Serial.println(F("Programa armazenado em memoria:"));
  imprimirProgramaArmazenado();
  Serial.println(F("Pressione '*' no teclado ou digite RUN no Serial Monitor para iniciar execucao."));
}

void confirmarInstrucao()
{
  bufferEntrada.trim();

  if (bufferEntrada.length() == 0)
  {
    Serial.println(F("[AVISO] Buffer vazio. Nenhuma instrucao armazenada."));
    return;
  }

  if (totalInstrucoes >= MAX_INSTRUCOES)
  {
    Serial.println(F("[ERRO] Memoria de programa cheia!"));
    return;
  }

  // Validar opcode
  int opcodeDecimal = -1;
  int spaceIdx = bufferEntrada.indexOf(' ');
  String parteOpcode = (spaceIdx >= 0) ? bufferEntrada.substring(0, spaceIdx) : bufferEntrada;
  parteOpcode.trim();
  opcodeDecimal = parteOpcode.toInt();

  if (opcodeDecimal < 0 || opcodeDecimal > 15)
  {
    Serial.println(F("[ERRO] Opcode invalido (deve ser 0-15)."));
    bufferEntrada = "";
    return;
  }

  // Armazenar na memória de programa
  programa[totalInstrucoes] = bufferEntrada;

  // Exibir confirmação em formato mnemônico
  String mnemonico = obterMnemonico(bufferEntrada);
  Serial.print(F("  ["));
  Serial.print(totalInstrucoes);
  Serial.print(F("] Armazenado: "));
  Serial.print(bufferEntrada);
  Serial.print(F("  →  "));
  Serial.println(mnemonico);

  totalInstrucoes++;
  bufferEntrada = "";
}

// ============================================================
// F02 — CODIFICAÇÃO MNEMÔNICO → OPCODE
// Converte a string "opcode_decimal [operando]" para opcode binário
// e retorna o mnemônico legível
// ============================================================

byte codificarParaOpcode(String instrucao)
{
  instrucao.trim();
  int spaceIdx = instrucao.indexOf(' ');
  String parteOpcode = (spaceIdx >= 0) ? instrucao.substring(0, spaceIdx) : instrucao;
  parteOpcode.trim();
  int opDecimal = parteOpcode.toInt();
  if (opDecimal < 0)
    opDecimal = 0;
  if (opDecimal > 15)
    opDecimal = 15;
  return (byte)opDecimal; // 4 bits: 0000–1111
}

int extrairOperando(String instrucao)
{
  int spaceIdx = instrucao.indexOf(' ');
  if (spaceIdx < 0)
    return 0;
  String parteOp = instrucao.substring(spaceIdx + 1);
  parteOp.trim();
  return parteOp.toInt();
}

String obterMnemonico(String instrucao)
{
  byte opcode = codificarParaOpcode(instrucao);
  int operando = extrairOperando(instrucao);
  String result = String(MNEMONICOS[opcode]);
  if (opcode == 2 || opcode == 3 || opcode == 4 || opcode == 5 ||
      opcode == 6 || opcode == 7 || opcode == 13 || opcode == 14)
  {
    result += " ";
    result += operando;
  }
  return result;
}

String opcodeParaBinario(byte opcode)
{
  String bin = "";
  for (int i = 3; i >= 0; i--)
  {
    bin += ((opcode >> i) & 1) ? "1" : "0";
  }
  return bin;
}

// ============================================================
// INICIALIZAR EXECUÇÃO (comando RUN ou *)
// ============================================================

void iniciarExecucao()
{
  if (totalInstrucoes == 0)
  {
    Serial.println(F("[AVISO] Nenhum programa carregado. Use # para entrar no modo LOAD."));
    return;
  }

  // Inicializar registradores — início do ciclo de instrução
  PC = 0;
  ACC = 0;
  FLAG_Z = false;
  EXECUTANDO = true;

  Serial.println(F("\n--- MODO RUN INICIADO ---"));
  Serial.print(F("Total de instrucoes: "));
  Serial.println(totalInstrucoes);
  Serial.println(F("Pressione '*' no teclado para executar instrucao por instrucao."));
  Serial.println(F("--------------------------------------------------"));
}

// ============================================================
// F03 — UNIDADE DE CONTROLE (UC): Ciclo de instrução completo
// Busca → Codifica → Carrega IR → Decodifica → Executa → Atualiza PC
// ============================================================

void executarProximaInstrucao()
{
  if (!EXECUTANDO)
  {
    Serial.println(F("[INFO] Execucao encerrada. Pressione '*' para reiniciar."));
    return;
  }

  if (PC >= totalInstrucoes)
  {
    Serial.println(F("[AVISO] PC alem do limite do programa. Execucao encerrada."));
    EXECUTANDO = false;
    return;
  }

  // --- BUSCA: carregar instrução da memória de programa ---
  String instrucaoAtual = programa[PC];

  // --- CODIFICAÇÃO: converter para opcode binário (F02) ---
  IR_ANTERIOR = IR;
  IR = codificarParaOpcode(instrucaoAtual); // IR recebe o opcode (Registrador de Instrução)
  int operando = extrairOperando(instrucaoAtual);

  // --- DECODIFICAÇÃO E EXECUÇÃO ---
  decodificarEExecutar(IR, operando);

  // --- APRESENTAÇÃO DOS ESTADOS INTERNOS (2.2.4) ---
  Serial.print(F("PC: "));
  Serial.print(PC);
  Serial.print(F(" | IR: "));
  Serial.print(opcodeParaBinario(IR));
  Serial.print(F(" ("));
  Serial.print(MNEMONICOS[IR]);
  Serial.print(F(") | ACC: "));
  Serial.print(ACC);
  Serial.print(F(" | FLAG_Z: "));
  Serial.println(FLAG_Z ? "1" : "0");

  // --- ATUALIZAR PC ---
  PC++;
}

// ============================================================
// DECODIFICAÇÃO E EXECUÇÃO das instruções
// UC despacha para as funções correspondentes (F03–F11)
// ============================================================

void decodificarEExecutar(byte opcode, int operando)
{
  switch (opcode)
  {

  case 0: // NOP — não faz nada
    Serial.println(F("[NOP] Nenhuma operacao."));
    break;

  case 1: // READ — lê sensor de distância → ACC (F05)
    ACC = lerDistancia();
    Serial.print(F("[READ] Distancia lida: "));
    Serial.print(ACC);
    Serial.println(F(" cm → ACC"));
    break;

  case 2: // LOADK k — carrega constante em ACC
    ACC = operando;
    Serial.print(F("[LOADK] ACC = "));
    Serial.println(ACC);
    break;

  case 3: // ADDK k — ULA: soma constante a ACC (F04)
    ACC = operacaoULA(ACC, operando, 'A');
    Serial.print(F("[ADDK] ACC = "));
    Serial.println(ACC);
    break;

  case 4: // SUBK k — ULA: subtrai constante de ACC (F04)
    ACC = operacaoULA(ACC, operando, 'S');
    Serial.print(F("[SUBK] ACC = "));
    Serial.println(ACC);
    break;

  case 5: // CMPK k — ULA: compara ACC com constante, atualiza FLAG_Z (F04)
    operacaoULA(ACC, operando, 'C');
    Serial.print(F("[CMPK] ACC == "));
    Serial.print(operando);
    Serial.print(F(" ? FLAG_Z = "));
    Serial.println(FLAG_Z ? "1 (igual)" : "0 (diferente)");
    break;

  case 6: // LEDON led — liga LED especificado por operando (F06)
    controlarLED(operando, true);
    Serial.print(F("[LEDON] LED "));
    Serial.print(operando);
    Serial.println(F(" ligado."));
    break;

  case 7: // LEDOFF led — desliga LED especificado por operando (F06)
    controlarLED(operando, false);
    Serial.print(F("[LEDOFF] LED "));
    Serial.print(operando);
    Serial.println(F(" desligado."));
    break;

  case 8: // BUZON — liga buzzer (F07)
    digitalWrite(PIN_BUZZER, HIGH);
    Serial.println(F("[BUZON] Buzzer ligado."));
    break;

  case 9: // BUZOFF — desliga buzzer (F07)
    digitalWrite(PIN_BUZZER, LOW);
    Serial.println(F("[BUZOFF] Buzzer desligado."));
    break;

  case 10: // DISP — exibe ACC no display de 7 segmentos (F08)
    exibirValorDisplay(ACC);
    Serial.print(F("[DISP] Exibindo ACC = "));
    Serial.println(ACC);
    break;

  case 11: // ALERT — resposta automática por faixa de distância (F10)
    executarALERT();
    break;

  case 12: // BINC — exibe opcode binário da instrução atual no Serial Monitor
    Serial.print(F("[BINC] Opcode binario de IR ("));
    Serial.print(MNEMONICOS[IR_ANTERIOR]);
    Serial.print(F("): "));
    Serial.println(opcodeParaBinario(IR_ANTERIOR));
    break;

  case 13: // STORE addr — armazena ACC em MEM[addr] (F09)
    if (operando >= 0 && operando < 16)
    {
      MEM[operando] = ACC;
      Serial.print(F("[STORE] MEM["));
      Serial.print(operando);
      Serial.print(F("] = "));
      Serial.println(ACC);
    }
    else
    {
      Serial.println(F("[STORE] Endereco invalido (0-15)."));
    }
    break;

  case 14: // LOADM addr — carrega MEM[addr] em ACC (F09)
    if (operando >= 0 && operando < 16)
    {
      ACC = MEM[operando];
      Serial.print(F("[LOADM] ACC = MEM["));
      Serial.print(operando);
      Serial.print(F("] = "));
      Serial.println(ACC);
    }
    else
    {
      Serial.println(F("[LOADM] Endereco invalido (0-15)."));
    }
    break;

  case 15: // HALT — encerra execução (F11)
    EXECUTANDO = false;
    Serial.println(F("[HALT] Execucao encerrada."));
    exibirDisplay(12); // apaga display
    break;

  default:
    Serial.println(F("[ERRO] Opcode desconhecido."));
    break;
  }
}

// ============================================================
// F04 — ULA: Operações aritméticas e de comparação
// Parâmetro op: 'A' = ADD, 'S' = SUB, 'C' = CMP
// ============================================================

int operacaoULA(int valorACC, int operando, char op)
{
  int resultado = valorACC;
  switch (op)
  {
  case 'A':
    resultado = valorACC + operando;
    break;
  case 'S':
    resultado = valorACC - operando;
    break;
  case 'C':
    // Comparação: atualiza FLAG_Z, não altera ACC
    FLAG_Z = (valorACC == operando);
    return valorACC; // ACC inalterado em CMPK
  }
  return resultado;
}

// ============================================================
// F05 — Leitura do sensor ultrassônico HC-SR04
// ============================================================

int lerDistancia()
{
  // Pulso de trigger
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  // Medir duração do echo
  long duracao = pulseIn(PIN_ECHO, HIGH, 30000); // timeout 30ms
  int distanciaCm = (int)(duracao * 0.034 / 2);

  // Limitar a faixas razoáveis
  if (distanciaCm <= 0 || distanciaCm > 400)
    distanciaCm = 0;

  return distanciaCm;
}

// ============================================================
// F06 — Controle de LEDs
// led: 1, 2 ou 3
// ============================================================

void controlarLED(int led, bool ligar)
{
  int pino;
  switch (led)
  {
  case 1:
    pino = PIN_LED1;
    break;
  case 2:
    pino = PIN_LED2;
    break;
  case 3:
    pino = PIN_LED3;
    break;
  default:
    Serial.println(F("[LED] Numero de LED invalido (1-3)."));
    return;
  }
  digitalWrite(pino, ligar ? HIGH : LOW);
}

// ============================================================
// F07 — Controle do buzzer (já tratado inline em decodificarEExecutar)
// ============================================================

// ============================================================
// F08 — Display de 7 segmentos
// indice: 0-9 = dígito, 10 = E (overflow), 11 = - (negativo), 12 = apagado
// ============================================================

void exibirDisplay(int indice)
{
  if (indice < 0 || indice > 12)
    indice = 12;
  for (int i = 0; i < 7; i++)
  {
    digitalWrite(PINOS_SEG[i], PADROES[indice][i] ? SEG_LIGADO : SEG_APAGADO);
  }
}

void exibirValorDisplay(int valor)
{
  // Tratamento de overflow (> 9) — seção 2.5.1
  if (valor > 9)
  {
    exibirDisplay(10); // 'E'
    Serial.print(F("[DISPLAY] OVERFLOW: valor "));
    Serial.print(valor);
    Serial.println(F(" nao representavel em 1 digito decimal. Exibindo 'E'."));
    return;
  }
  // Tratamento de resultado negativo — seção 2.5.2
  if (valor < 0)
  {
    exibirDisplay(11); // '-'
    Serial.print(F("[DISPLAY] NEGATIVO: valor "));
    Serial.print(valor);
    Serial.println(F(" nao representavel. Exibindo '-'."));
    return;
  }
  // Valor válido (0–9)
  exibirDisplay(valor);
}

// ============================================================
// F10 — Instrução ALERT: resposta automática por faixa de distância
// ============================================================

void executarALERT()
{
  int distancia = lerDistancia();
  ACC = distancia;
  Serial.print(F("[ALERT] Distancia medida: "));
  Serial.print(distancia);
  Serial.println(F(" cm"));

  if (distancia < 10)
  {
    // Faixa crítica: buzzer + LED de alerta (LED1)
    digitalWrite(PIN_BUZZER, HIGH);
    controlarLED(1, true);
    controlarLED(2, false);
    Serial.println(F("[ALERT] < 10cm: BUZZER LIGADO + LED1 LIGADO (alerta critico)"));
  }
  else if (distancia < 20)
  {
    // Faixa de atenção: LED de alerta, sem buzzer
    digitalWrite(PIN_BUZZER, LOW);
    controlarLED(1, true);
    controlarLED(2, false);
    Serial.println(F("[ALERT] 10-19cm: LED1 LIGADO (alerta), buzzer desligado"));
  }
  else
  {
    // Faixa segura: tudo desligado
    digitalWrite(PIN_BUZZER, LOW);
    controlarLED(1, false);
    controlarLED(2, false);
    Serial.println(F("[ALERT] >= 20cm: saidas de alerta DESLIGADAS (situacao segura)"));
  }
}

// ============================================================
// UTILITÁRIOS
// ============================================================

void desligarTodasSaidas()
{
  digitalWrite(PIN_LED1, LOW);
  digitalWrite(PIN_LED2, LOW);
  digitalWrite(PIN_LED3, LOW);
  digitalWrite(PIN_BUZZER, LOW);
}

void imprimirProgramaArmazenado()
{
  if (totalInstrucoes == 0)
  {
    Serial.println(F("  (vazio)"));
    return;
  }
  for (int i = 0; i < totalInstrucoes; i++)
  {
    Serial.print(F("  ["));
    Serial.print(i);
    Serial.print(F("] "));
    Serial.print(programa[i]);
    Serial.print(F("  →  "));
    Serial.println(obterMnemonico(programa[i]));
  }
}
