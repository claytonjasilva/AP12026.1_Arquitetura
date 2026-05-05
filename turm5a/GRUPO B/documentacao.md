# Documentação Técnica: Interpretador de Instruções (Arduino Mega 2560)
**Integrantes:** Christian, Bernardo e Mariana
**Disciplina:** Arquitetura de Computadores
**Plataforma:** Arduino Mega 2560--
## 1. Descrição Geral e Escopo
Este projeto consiste no desenvolvimento de um sistema embarcado que atua como um 
**interpretador de instruções** programável. O objetivo central é a aplicação prática de 
conceitos de **Arquitetura de Computadores**, como o ciclo de instrução e a execução de 
programas armazenados, utilizando um teclado matricial para entrada de comandos 
mnemônicos e diversos atuadores (LEDs, Buzzer, Display) para saída de dados.
**Escopo:**
*   Implementação de uma **Unidade de Controle (UC)** e **Unidade Lógica e Aritmética 
(ULA)** via software.
*   Operação baseada em **Ciclo de Instrução** (Busca, Decodificação e Execução).
*   Uso de sensores (ultrassônico) e atuadores para interação com o ambiente.
*   **Não inclui:** Eletrônica avançada, CIs controladores de display dedicados ou 
comunicação em rede.
## 2. Elementos Arquiteturais
A arquitetura do sistema foi mapeada diretamente no código do Arduino (sketch), utilizando 
variáveis e estruturas de dados para simular os componentes de um processador real:
*   **Unidade de Controle (UC):** Lógica responsável por gerenciar o ciclo de instrução e o 
fluxo de dados entre os componentes.
*   **Unidade Lógica e Aritmética (ULA):** Implementação das operações de soma (ADDK), 
subtração (SUBK) e comparação (CMPK).
*   **Memória (MEM):** Vetor de dados utilizado para armazenamento persistente de valores 
durante a execução.
*   **Registradores Simulados:**
    *   **PC (Program Counter):** Controla o endereço da instrução atual.
    *   **IR (Instruction Register):** Armazena o opcode da instrução em execução.
    *   **ACC (Acumulador):** Registrador principal para operações e resultados.
    *   **FLAG_Z:** Indica se o resultado de uma comparação foi zero (igualdade).
## 3. Modos de Operação
O sistema opera em dois estados distintos, reforçando o conceito de programa armazenado:
### 3.1 Modo de Entrada (LOAD)
Ativado pelo caractere **#** no teclado matricial.
1.  Limpa a memória de programa e reseta o ponteiro de carga para zero.
2.  O usuário insere instruções mnemônicas sequencialmente via teclado.
3.  As instruções são armazenadas em uma estrutura de memória (vetor de strings) sem 
execução imediata.
4.  O modo é encerrado pressionando **#** novamente.
### 3.2 Modo de Execução (RUN)
Iniciado pelo comando **RUN** no Monitor Serial ou tecla equivalente definida.
1.  O **PC** é inicializado em 0.
2.  A execução é passo a passo: cada comando **\*** executa exatamente um ciclo de 
instrução.
3.  O ciclo compreende: busca do mnemônico, conversão para opcode binário, carga no **IR**, 
decodificação, execução, atualização dos registradores (**ACC, FLAG_Z**) e incremento do 
**PC**.
## 4. Conjunto de Instruções (ISA)
A Instruction Set Architecture (ISA) do projeto define 16 instruções com opcodes de 4 bits:
| Decimal | Opcode | Mnemônico | Função Principal |
| :--- | :--- | :--- | :--- |
| 0 | 0000 | NOP | Nenhuma operação |
| 1 | 0001 | READ | Lê sensor de distância para o ACC |
| 2 | 0010 | LOADK | Carrega constante no ACC |
| 3 | 0011 | ADDK | Soma constante ao ACC |
| 4 | 0100 | SUBK | Subtrai constante do ACC |
| 5 | 0101 | CMPK | Compara ACC com constante (atualiza FLAG_Z) |
| 6 | 0110 | LEDON | Liga LED (via operando) |
| 7 | 0111 | LEDOFF | Desliga LED (via operando) |
| 8 | 1000 | BUZON | Liga o buzzer |
| 9 | 1001 | BUZOFF | Desliga o buzzer |
| 10 | 1010 | DISP | Exibe ACC no display de 7 segmentos |
| 11 | 1011 | ALERT | Resposta automática baseada na distância |
| 12 | 1100 | BINC | Exibe opcode binário no Serial Monitor |
| 13 | 1101 | STORE | Salva ACC na MEM[X] |
| 14 | 1110 | LOADM | Carrega MEM[X] no ACC |
| 15 | 1111 | HALT | Encerra a execução |
## 5. Descrição das Funcionalidades (F01 a F11)
*   **F01 (Entrada):** Captura de mnemônicos via teclado matricial.
*   **F02 (Codificação):** Tradução de texto (mnemônico) para opcode binário de 4 bits.
*   **F03 (Controle):** Gerenciamento do ciclo de instrução pela UC.
*   **F04 (ULA):** Execução de ADDK, SUBK e CMPK.
*   **F05 (Sensor):** Leitura do sensor ultrassônico HC-SR04 em centímetros.
*   **F06/F07 (Atuadores):** Controle digital de LEDs e Buzzer.
*   **F08 (Display):** Interface com display de 7 segmentos para exibição de resultados.
*   **F09 (Memória):** Operações de STORE e LOADM em vetor simulado.
*   **F10 (ALERT):** Lógica automática: <10cm (Buzzer+LED); 10-20cm (LED); >=20cm (Nada).
*   **F11 (Fim):** Interrupção total do ciclo via instrução HALT.
## 6. Tratamento de Erros
Devido à limitação de um único dígito no display, implementamos o seguinte tratamento:
*   **Overflow:** Se o resultado for > 9, o display exibe **"E"** e o Serial Monitor alerta sobre o 
erro de estouro.
*   **Resultados Negativos:** Se uma operação resultar em valor < 0, o display exibe um 
padrão definido (ex: **"-"**) e o erro é reportado no Serial.
## 7. Esquema Elétrico e Pinagem
A montagem segue a pinagem obrigatória definida para o Arduino Mega 2560:
| Componente | Pinos Arduino |
| :--- | :--- |
| **Display (a-g)** | 22, 23, 24, 25, 26, 27, 28 |
| **Teclado (L1-L4, C1-C4)** | 30, 31, 32, 33 / 34, 35, 36, 37 |
| **Sensor (TRIG / ECHO)** | 40 / 41 |
| **LEDs (1, 2, 3)** | 42, 43, 44 |
| **Buzzer** | 45 |
*Nota: Todos os LEDs e segmentos do display utilizam resistores de 220Ω a 330Ω em série.*
## 8. Roteiro de Testes
Para validação do sistema, o grupo (Christian, Bernardo e Mariana) seguirá os seguintes 
passos de teste:
1.  **Carga de Programa:** Iniciar modo LOAD (#), inserir sequência (ex: READ, STORE, DISP, 
HALT) e encerrar (#).
2.  **Execução Passo a Passo:** Enviar RUN e usar "*" para cada instrução.
3.  **Validação de Dados:** Verificar se o valor do sensor lido por READ aparece corretamente 
no ACC e no display via DISP.
4.  **Teste de ULA:** Realizar ADDK e SUBK, verificando se resultados maiores que 9 disparam 
o erro "E" (Overflow).
5.  **Teste de Memória:** Executar STORE seguido de LOADM para confirmar a persistência 
no vetor MEM.
6.  **Teste de Alerta:** Posicionar objeto a <10cm e executar ALERT para verificar acionamento 
do Buzzer.
7.  **Encerramento:** Confirmar que, após HALT, o sistema ignora novos comandos "*" até um 
novo RUN.
