1. Elementos Arquiteturais (Seção 2.1)
 - Código ->

    int MEM[16];            // memória de dados simulada
    int PC = 0;             // program counter (controla a instrução atual/próxima)
    byte IR = 0;            // instruction register (armazena o opcode binário)
    int ACC = 0;            // acumulador (armazena operandos e resultados)
    bool FLAG_Z = false;    // flag de comparação
    bool EXECUTANDO = true; // controle de execução do programa

 - Exige que todas as variaveis estajam com seus respectivos nomes e dados sketch indentificáveis, também exige que cada cotenha comentários explicando sua função.

2.  Estruturas de memória de programa e operação (Seção 2.2, 2.2.1)
 - Código ->
    
        struct Instrucao {
    byte opcode;
    int operando;
    };

    Instrucao memoriaPrograma[32]; // Armazena as instruções do usuário
    int ponteiroCarga = 0;         // Endereço para carregar nova instrução

    enum ModoOperacao { MODO_OCIOSO, MODO_LOAD, MODO_RUN };
    ModoOperacao modoAtual = MODO_OCIOSO;

    String bufferEntrada = "";
    bool aguardandoOperando = false;
    byte opcodeTemp = 0;

 - Exigiu 2 modelos distintos de funções: Load e Run. Suas instrições são armazenadas na memória, de forma que sua execução não seja automatica, utilizamos: *enum* *memoriaPrograma* *ponteiroCarga* pra fazer isso(modo Run). Também exige que uma memoria de instruções que opere de forma sequencial e ponteiro de carga, utilizamos: *memoriaPrograma* *ponteiroCarga* (modo Load).

3. Pinagem e Hardware (Seção 2.2.1, F01, F05, F06, F07, F08)
 - Código ->
   
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

 - Exigiu um mecanismo de confirmação e de limpeza do buffer, foi definido Tecla *A* : Enter | Tecla *C* : Clear | Tecla *✳* : Próxima    Instrução | Tecla *#* : Modo Load. Possui entrada via teclado matricial com mapeamento de teclas.
   Exigiu entrada via teclado matricial com mapeamento de teclas. Matriz de teclas documentada com comentários para cada função.
   Tambem solicitou saídas de sensor, LED's, buzzer e display, estão todos conectados declarados como constantes e com seus respectivos nomes.

4. Tabela de Mnemonicos e ISA (Seção 2.2.1, Seção 2.3, F02)
 - Código ->

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
        //retorna true se a instrução exige um segunso número
        return (opcode == 2 || opcode == 3 || opcode == 4 || opcode == 5 || 
              opcode == 6 || opcode == 7 || opcode == 13 || opcode == 14);
      }

  - Exige uma rotina que associe mnemonicos a upcodes, armazenados numa IR. A função *obterMnemonico(byte opcode)* recebe o opcode e associa com o mnemonico. o O *precisaOperando(byte code)* garante que o sistema saiba exatamente quantos valores esperar para cada instrução antes de armazená-la. Exige que cada instrução tenha mnemônico, opcode de 4 bits e descrição funcional.

5. Setup Inicial(Seção 2.2, Seção 2.2.4, F05, F06, F07, F08 ) 
 - Código ->

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

 - Exige que o sistema esteja em um estado inicial antes de aceitar comandos. O *setup()* inicializa todos os pinos, limpa o display e informa o usuário sobre os comandos disponíveis. Tambem pede que LEDs, buzzer e display estejam configurados antes do uso, fizemos por meio do *pinMode()*. Exige que o (sensor) HC-SR04 esteja corretamente configurado com Trig como saída e Echo como entrada, implementado com direções distintas. Pede que o Serial Monitor seja utilizado para comunicação com o usuário, inicializa com *Serial.begin(9600)* e exibe mensagem de orientação.

6. Loop Principal e Função de Entrada (Seção 2.2.1, Seção 2.2.2, Seção 2.2.3, Seção 2.2.7, F01)
  - Código ->

      void loop() {
    verificarComandosSerial();
    verificarTeclado();
  }

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
     executarCicloDeInstrucao();
     return;
    }

    if (modoAtual == MODO_LOAD) {
      processarDigitacaoLoad(tecla);
    }
  }

  - Esse é o centro de controle de entrada do sistema. Exige entrada via teclado matricial feito com *verificarTeclado()* roteia cada tecla para sua função correspondente. Exige que # ative o modo de carregamento feito com *alternarModoLoad()*. Exige comando explícito RUN para iniciar execução e *✳* para proxima instrução.Condição de parada é que após HALT novos comandos *✳* sejam ignorados feito pelo *executarCicloDeInstrucao()*. Exige que instruções não sejam executadas imediatamente, só ocorre no MODO RUN mediante comando *✳*

7. Função modo Load (Seção 2.2.1, Seção 2.2.2, Seção 2.2.7, F01)
 - Código ->
  
  void alternarModoLoad() {
  if (modoAtual == MODO_LOAD) {
    modoAtual = MODO_OCIOSO;
    Serial.println("\n[SISTEMA] Saiu do MODO LOAD. Programa armazenado.");
  } else {
    modoAtual = MODO_LOAD;
    ponteiroCarga = 0;
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
  else if (tecla == 'A') {
    if (bufferEntrada.length() == 0) return;
    
    int valorDigitado = bufferEntrada.toInt();
    bufferEntrada = "";
    Serial.println();
    
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

- Exige limpeza da memória ao entrar no LOAD, ponteiro iniciado em zero, armazenamento sequencial e confirmação de instrução armazenada.  Todos implementados em *alternarModoLoad()* e *salvarInstrucaoMemoria()*. Inicialização de PC = 0 ao receber comando RUN. Feito em *iniciarModoRun()*. Pede que instruções sejam armazenadas e não executadas imediatamente, *salvarInstrucaoMemoria()* apenas grava na memória sem executa-la. Pede definição do mecanismo de confirmação e indicação ao usuário de instrução armazenada.  Tecla *A* como ENTER, *C* como CLEAR e confirmação via Serial Monitor a cada instrução salva.

8. UC e ciclo de instrução (Seção 2.2.4, Seção 2.2.3, F03, F04, F09)
 - Código ->
  
  void executarCicloDeInstrucao() {
  // 1. Busca da Instrução
  IR = memoriaPrograma[PC].opcode;
  int OPND = memoriaPrograma[PC].operando;

  // 2 e 3. Decodificação e Execução
  switch (IR) {
    case 0: break;                          // NOP
    case 1: ACC = medirDistancia(); break;  // READ
    case 2: ACC = OPND; break;             // LOADK
    case 3: ACC += OPND; break;            // ADDK
    case 4: ACC -= OPND; break;            // SUBK
    case 5: FLAG_Z = (ACC == OPND); break; // CMPK
    case 6: controlarLed(OPND, HIGH); break;  // LEDON
    case 7: controlarLed(OPND, LOW); break;   // LEDOFF
    case 8: digitalWrite(pinoBuzzer, HIGH); break; // BUZON
    case 9: digitalWrite(pinoBuzzer, LOW); break;  // BUZOFF
    case 10: exibirDisplay(ACC); break;    // DISP
    case 11: executarAlert(); break;       // ALERT
    case 12:                               // BINC
      Serial.print("Binário do Opcode Atual: ");
      Serial.println(IR, BIN);
      break;
    case 13:                               // STORE
      if (OPND >= 0 && OPND < 16) MEM[OPND] = ACC;
      break;
    case 14:                               // LOADM
      if (OPND >= 0 && OPND < 16) ACC = MEM[OPND];
      break;
    case 15:                               // HALT
      EXECUTANDO = false;
      Serial.println("[HALT] Fim da Execução.");
      break;
  }

  // Apresentação do Estado
  String repIR = obterMnemonico(IR);
  if (precisaOperando(IR)) repIR += " " + String(OPND);
  
  Serial.println("PC: " + String(PC) + " | IR: " + repIR + 
                 " | ACC: " + String(ACC) + " | FLAG_Z: " + String(FLAG_Z));

  // 4. Atualização do PC
  if (EXECUTANDO) PC++;
}

- Exige funções de busca, codificação, carga em IR, decodificação, execução e controle de fluxo feitas sequencialmente em *executarCicloDeInstrucao()*. Pede ADDK, SUBK e CMPK com resultados em ACC e FLAG_Z. Exige STORE e LOADM com validação de endereço, estão nos Cases 13 e 14 com verificação de bounds. Exige exibição de PC, IR, ACC e FLAG_Z após cada instrução. Pede que HALT encerre a execução e bloqueie novos avanços feito em *EXECUTANDO = false impede* incremento do PC e bloqueia novos comandos *.

9. Processamento e Hardware 
  - Código ->

  int medirDistancia() {
  digitalWrite(pinoTrig, LOW); delayMicroseconds(2);
  digitalWrite(pinoTrig, HIGH); delayMicroseconds(10);
  digitalWrite(pinoTrig, LOW);
  long duracao = pulseIn(pinoEcho, HIGH, 30000);
  if (duracao == 0) return 99;
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

- Representa a camada de interface com o hardware físico, instruções abstratas do programa se traduzem em ações, acionando componentes eletrônicos conectados ao arduino.Pede a leitura do HC-SR04 com resultado em centímetros armazenado no ACC, feito pelço *medirDistancia()*, atribuído ao ACC via case 1 na UC. Exige ligar e desligar LEDs por instruções operando e identificando qual LED, *controlarLed()* mapeia operandos 1, 2 e 3 para os respectivos pinos. Exige comportamento mínimo obrigatório com 3 faixas: <10cm, 10-20cm e ≥20cm implementadas corretamente por *executarAlert()*.


10. Display de 7 Segmentos, Overflow e Valor Negativo (Seção 2.5.1, Seção 2.5.2, F08)
   - Código ->

    void exibirDisplay(int valor) {
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
        {1,0,0,1,1,1,1}, // 10 -> 'E' (Overflow)
        {0,0,0,0,0,0,1}  // 11 -> '-' (Negativo)
      };

       int indiceMapa;

      if (valor > 9) {
         indiceMapa = 10;
         Serial.println(">> ALERTA: OVERFLOW DETECTADO (Valor > 9)");
      } 
    else if (valor < 0) {
      indiceMapa = 11;
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

- seção que fecha o ciclo do sistema após todas as operações de entrada, processamento e controle, o resultado final chega de forma visual no displays. Exige exibição de valores resultantes de instruções respeitando as limitações do dispositivo. Pede uma exibição de *E* no display e alerta no Serial Monitor quando é valor > 9, Implementado com *indiceMapa = 10 e mensagem ">> ALERTA: OVERFLOW DETECTADO"*. Exige exibição de - no display e alerta no Serial Monitor quando valor < 0. Implementado com *indiceMapa = 11 e mensagem ">> ALERTA: VALOR NEGATIVO DETECTADO"*.     











