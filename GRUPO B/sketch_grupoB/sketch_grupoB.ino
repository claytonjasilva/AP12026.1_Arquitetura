/*
202402697706 - João Gabriel (TA)
202407095917 - Enzo Zambrotti (TA)
202408356404 - Guilherme Viana (TA)
Bernardo Araujo (NT)
202503076936 - Pedro Carvalho (TA)
*/



#include <Keypad.h>



int MEM[16];

int PC = 0;

byte IR = 0;

int ACC = 0;

bool FLAG_Z = false;

bool EXECUTANDO = true;



String memoriaPrograma[32];

int ponteiroCarga = 0;



enum ModoOperacao { ESPERA, LOAD, RUN };

ModoOperacao modoAtual = ESPERA;



String bufferEntrada = "";

int operandoAtual = 0;



const byte LINHAS = 4;

const byte COLUNAS = 4;

char teclas[LINHAS][COLUNAS] = {

  {'1','2','3','A'},

  {'4','5','6','B'},

  {'7','8','9','C'},

  {'*','0','#','D'}

};

byte pinosLinhas[LINHAS] = {22, 24, 26, 28};

byte pinosColunas[COLUNAS] = {30, 32, 34, 36};

Keypad teclado = Keypad(makeKeymap(teclas), pinosLinhas, pinosColunas, LINHAS, COLUNAS);

const int pinosDisplay[8] = {48, 49, 52, 50, 51, 47, 46, 53}; 

const int trigPin = 6;

const int echoPin = 7;

const int leds[3] = {38, 39, 40};

const int buzzerPin = 41;

const char* mnemonicos[] = {

  "NOP", "READ", "LOADK", "ADDK", "SUBK", "CMPK", "LEDON", 

  "LEDOFF", "BUZON", "BUZOFF", "DISP", "ALERT", "BINC", 

  "STORE", "LOADM", "HALT"

};

void setup() {

  Serial.begin(9600);

  for (int i = 0; i < 8; i++) {

    pinMode(pinosDisplay[i], OUTPUT);

    digitalWrite(pinosDisplay[i], HIGH);

  }

  for (int i = 0; i < 3; i++) {

    pinMode(leds[i], OUTPUT);

  }

  

  pinMode(trigPin, OUTPUT);

  pinMode(echoPin, INPUT);

  pinMode(buzzerPin, OUTPUT);



  Serial.println("Sistema Iniciado.");

}



void loop() {

  char tecla = teclado.getKey();

  

  if (tecla) {

    processarEntrada(tecla);

  }

}



void processarEntrada(char tecla) {

  if (tecla == '#') {

    if (modoAtual == LOAD) {

      modoAtual = ESPERA;

      Serial.println("Modo LOAD finalizado.");

    } else {

      modoAtual = LOAD;

      limparMemoriaPrograma();

      Serial.println("Modo LOAD iniciado. Digite as instrucoes.");

    }

    return;

  }



  if (tecla == 'A' && modoAtual != LOAD) {

    modoAtual = RUN;

    PC = 0;

    EXECUTANDO = true;

    Serial.println("Modo RUN iniciado. Pressione '*' para executar passos.");

    return;

  }



  if (modoAtual == LOAD) {

    if (tecla >= '0' && tecla <= '9') {

      bufferEntrada += tecla;

      Serial.print(tecla);

    } else if (tecla == 'C') {

      bufferEntrada += " ";

      Serial.print(" ");

    } else if (tecla == 'D') {

      memoriaPrograma[ponteiroCarga] = bufferEntrada;

      Serial.println();

      Serial.print("Instrucao gravada na posicao ");

      Serial.println(ponteiroCarga);

      ponteiroCarga++;

      bufferEntrada = "";

    }

  }



  if (modoAtual == RUN && tecla == '*') {

    if (EXECUTANDO) {

      cicloDeInstrucao();

    } else {

      Serial.println("Programa finalizado. Envie 'A' para rodar novamente.");

    }

  }

}



void limparMemoriaPrograma() {

  for (int i = 0; i < 32; i++) {

    memoriaPrograma[i] = "";

  }

  ponteiroCarga = 0;

  bufferEntrada = "";

}



void cicloDeInstrucao() {

  String instrucao = memoriaPrograma[PC];

  if (instrucao == "") return;



  int espacoIndex = instrucao.indexOf(' ');

  if (espacoIndex != -1) {

    IR = instrucao.substring(0, espacoIndex).toInt();

    operandoAtual = instrucao.substring(espacoIndex + 1).toInt();

  } else {

    IR = instrucao.toInt();

    operandoAtual = 0;

  }



  executarULAeSaidas();

  imprimirEstado();

  PC++;

}



void executarULAeSaidas() {

  switch (IR) {

    case 0: break;

    case 1: ACC = lerSensor(); break;

    case 2: ACC = operandoAtual; break;

    case 3: ACC += operandoAtual; break;

    case 4: ACC -= operandoAtual; break;

    case 5: FLAG_Z = (ACC == operandoAtual); break;

    case 6: 

      if (operandoAtual >= 1 && operandoAtual <= 3) digitalWrite(leds[operandoAtual - 1], HIGH); 

      break;

    case 7: 

      if (operandoAtual >= 1 && operandoAtual <= 3) digitalWrite(leds[operandoAtual - 1], LOW); 

      break;

    case 8: digitalWrite(buzzerPin, HIGH); break;

    case 9: digitalWrite(buzzerPin, LOW); break;

    case 10: exibirDisplay(ACC); break;

    case 11: executarAlerta(); break;

    case 12: 

      Serial.print("Opcode Binario: "); 

      Serial.println(IR, BIN); 

      break;

    case 13: if (operandoAtual < 16) MEM[operandoAtual] = ACC; break;

    case 14: if (operandoAtual < 16) ACC = MEM[operandoAtual]; break;

    case 15: EXECUTANDO = false; break;

  }

}



int lerSensor() {

  digitalWrite(trigPin, LOW);

  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);

  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  long duracao = pulseIn(echoPin, HIGH);

  return duracao * 0.034 / 2;

}



void executarAlerta() {

  int dist = lerSensor();

  if (dist < 10) {

    digitalWrite(buzzerPin, HIGH);

    digitalWrite(leds[0], HIGH);

  } else if (dist >= 10 && dist < 20) {

    digitalWrite(buzzerPin, LOW);

    digitalWrite(leds[0], HIGH);

  } else {

    digitalWrite(buzzerPin, LOW);

    digitalWrite(leds[0], LOW);

  }

}

void exibirDisplay(int valor) {

  const byte digitos[10] = {

    0b11000000, 0b11111001, 0b10100100, 0b10110000, 0b10011001,

    0b10010010, 0b10000010, 0b11111000, 0b10000000, 0b10010000

  };

  

  byte padrao;

  

  if (valor < 0) {

    Serial.println("Aviso: Valor negativo detectado.");

    padrao = 0b10111111;

  } else if (valor > 9) {

    Serial.println("Aviso: Overflow detectado (Valor > 9).");

    padrao = 0b10000110;

  } else {

    padrao = digitos[valor];

  }



  for (int i = 0; i < 7; i++) {

    digitalWrite(pinosDisplay[i], bitRead(padrao, i));

  }

  

  digitalWrite(pinosDisplay[7], HIGH); 

}



void imprimirEstado() {
  Serial.print("PC: ");
  Serial.print(PC);
  Serial.print(" | IR: ");
  Serial.print(mnemonicos[IR]);
  
  Serial.print(" (");
  for (int i = 3; i >= 0; i--) {
    Serial.print(bitRead(IR, i));
  }
  Serial.print(")");

  if (operandoAtual > 0 || IR == 2 || IR == 3 || IR == 4 || IR == 5) {
    Serial.print(" ");
    Serial.print(operandoAtual);
  }
  Serial.print(" | ACC: ");
  Serial.print(ACC);
  Serial.print(" | FLAG_Z: ");
  Serial.println(FLAG_Z);
}
