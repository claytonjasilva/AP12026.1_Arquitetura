# Documentação Técnica do Projeto

## Sistema Interpretador de Instruções com Sensor de Distância e Atuadores

**Disciplina:** Arquitetura de Computadores
**Avaliação:** AP1 — 2026/1
**Plataforma:** Arduino Mega 2560

### Link para Wokwi

https://wokwi.com/projects/462868896859417601


#### Integrantes do grupo

| Matrícula     | Nome             |
|---------------|------------------|
| 202507356356  | Filipe Andrade   |
| 202507010727  | Vitor Zanconato  |
| 202507012436  | Gabriel de Souza |
| 202507142925  | Vitor Magalhães  |

---

## 1. Visão geral

O projeto consiste em um **interpretador de instruções** implementado em Arduino Mega 2560 que reproduz, em escala didática, os elementos clássicos de uma arquitetura de von Neumann: memória de programa, memória de dados, registradores (PC, IR, ACC), Unidade de Controle (UC) e Unidade Lógica e Aritmética (ULA).

O usuário digita instruções por meio de um teclado matricial 4x4. As instruções são mnemônicos representados por códigos decimais (0–15) que correspondem a **opcodes binários de 4 bits**. O sistema opera segundo o modelo de **programa armazenado**: as instruções primeiro são carregadas em memória (modo LOAD), depois são executadas uma a uma sob comando explícito do usuário (modo RUN), respeitando o **ciclo de instrução**: busca → decodificação → execução.

### 1.1 Mapeamento entre conceitos arquiteturais e implementação

| Conceito arquitetural          | Implementação no sketch                                              |
|--------------------------------|----------------------------------------------------------------------|
| Memória de programa            | Vetor `programa[]` (Strings no formato `"opcode operando"`)          |
| Memória de dados               | Vetor `MEM[16]`                                                      |
| Program Counter (PC)           | Variável `int PC`                                                    |
| Instruction Register (IR)      | Variável `byte IR`                                                   |
| Acumulador (ACC)               | Variável `int ACC`                                                   |
| Flag de zero                   | Variável `bool FLAG_Z`                                               |
| Controle de execução           | Variável `bool EXECUTANDO`                                           |
| Unidade de Controle (UC)       | Funções `executarProximaInstrucao()` e `decodificarEExecutar()` (ciclo de busca/decodificação/execução) |
| Unidade Lógica e Aritmética    | Função `operacaoULA(acc, operando, op)`                              |
| Codificação mnemônico → opcode | Array `MNEMONICOS[]` + funções `codificarParaOpcode()` e `obterMnemonico()` |
| ISA                            | Tabela de opcodes decimais 0…15 (4 bits)                             |
| Entrada                        | Teclado matricial 4x4 + Serial Monitor                               |
| Saída                          | LEDs, buzzer, display 7 segmentos, Serial Monitor                    |
| Sensor de dados                | HC-SR04 (função `lerDistancia()`)                                    |

---

## 2. ISA — Conjunto de Instruções

Cada instrução é representada por um **opcode binário de 4 bits**, conforme a especificação do enunciado.

| Decimal | Opcode | Mnemônico | Operando | Descrição                                            |
|---------|--------|-----------|----------|------------------------------------------------------|
| 0       | 0000   | NOP       | —        | Não realiza operação                                 |
| 1       | 0001   | READ      | —        | Lê o sensor e armazena em `ACC`                      |
| 2       | 0010   | LOADK     | sim      | Carrega constante em `ACC`                           |
| 3       | 0011   | ADDK      | sim      | Soma constante a `ACC`                               |
| 4       | 0100   | SUBK      | sim      | Subtrai constante de `ACC`                           |
| 5       | 0101   | CMPK      | sim      | Compara `ACC` com constante e atualiza `FLAG_Z`      |
| 6       | 0110   | LEDON     | sim      | Liga LED indicado pelo operando (1, 2 ou 3)          |
| 7       | 0111   | LEDOFF    | sim      | Desliga LED indicado pelo operando                   |
| 8       | 1000   | BUZON     | —        | Liga o buzzer                                        |
| 9       | 1001   | BUZOFF    | —        | Desliga o buzzer                                     |
| 10      | 1010   | DISP      | —        | Exibe valor de `ACC` no display de 7 segmentos       |
| 11      | 1011   | ALERT     | —        | Resposta automática conforme distância lida          |
| 12      | 1100   | BINC      | —        | Imprime no Serial Monitor o opcode binário atual     |
| 13      | 1101   | STORE     | sim      | Armazena `ACC` em `MEM[X]`                           |
| 14      | 1110   | LOADM     | sim      | Carrega `MEM[X]` em `ACC`                            |
| 15      | 1111   | HALT      | —        | Encerra a execução do programa                       |

---

## 3. Modelo de operação

O sistema possui três estados internos:

- **MODO_OCIOSO** — estado inicial; aguarda comando.
- **MODO_LOAD** — recebe instruções do teclado e armazena na memória de programa.
- **MODO_RUN** — executa o programa carregado, uma instrução por vez.

```
[OCIOSO] --(#)--> [LOAD] --(#)--> [OCIOSO] --(*ou RUN)--> [RUN]
                                                            |
                                                (* a cada instrução)
                                                            |
                                                         [HALT] --> [OCIOSO]
```

### 3.1 Mapa do teclado matricial

| Tecla    | Função                                                                  |
|----------|-------------------------------------------------------------------------|
| `0`–`9`  | Dígitos decimais (compõem o opcode/operando no buffer)                  |
| `A`      | Confirma a instrução digitada (armazena na memória de programa)         |
| `B`      | Backspace — apaga o último caractere do buffer                          |
| `C`      | Limpa todo o buffer atual                                               |
| `D`      | Separador — insere espaço entre opcode e operando no buffer             |
| `*`      | Inicia execução (modo OCIOSO → RUN) / avança 1 instrução (modo RUN)     |
| `#`      | Entra ou sai do modo LOAD                                               |

> **Nota:** O comando RUN também pode ser enviado pelo Serial Monitor digitando `RUN` seguido de Enter.

### 3.2 Modo LOAD — entrada de programa

Ao pressionar `#`:

1. A memória de programa (`programa[]`) é zerada.
2. O contador `totalInstrucoes` é zerado.
3. O sistema entra em modo LOAD e imprime as instruções disponíveis no Serial Monitor.

Cada instrução é digitada em decimal. A tecla `D` separa opcode do operando (equivalente a um espaço), e `A` confirma o par armazenando-o em `programa[totalInstrucoes]`:

- Para instruções **sem operando** (ex.: `DISP` = 10), basta `1`, `0`, `A`.
- Para instruções **com operando** (ex.: `LOADK 5` = opcode 2, operando 5), digite `2`, `D`, `5`, `A`.

Ao pressionar `#` novamente, o sistema sai do modo LOAD e imprime o programa carregado em mnemônicos no Serial Monitor.

#### Exemplo do enunciado

Programa em mnemônicos:

```
LOADK 5
ADDK 3
DISP
HALT
```

Sequência digitada no teclado:

```
2 D 5 A      (LOADK 5)
3 D 3 A      (ADDK 3)
1 0 A        (DISP)
1 5 A        (HALT)
```

Saída no Serial Monitor após sair do LOAD:

```
--- MODO LOAD ENCERRADO ---
Total de instrucoes carregadas: 4
Programa armazenado em memoria:
  [0] 2 5  →  LOADK 5
  [1] 3 3  →  ADDK 3
  [2] 10   →  DISP
  [3] 15   →  HALT
```

### 3.3 Modo RUN — execução

Pressionando `*` (ou enviando `RUN` pelo Serial Monitor):

- `PC = 0`, `ACC = 0`, `IR = 0`, `FLAG_Z = false`.
- O sistema aguarda `*` para executar cada instrução.

A cada `*`, ocorre exatamente um **ciclo de instrução**:

1. **Busca** — `instrucaoAtual ← programa[PC]`.
2. **Codificação** — `IR ← codificarParaOpcode(instrucaoAtual)`; `operando ← extrairOperando(instrucaoAtual)`.
3. **Execução** — `decodificarEExecutar(IR, operando)` seleciona a operação (ULA, sensor, saída, memória).
4. **Atualização de estado** — `ACC`, `MEM`, `FLAG_Z`, dispositivos de saída.
5. **Apresentação dos estados internos** no Serial Monitor (PC, IR binário, mnemônico, ACC, FLAG_Z).
6. **Atualização do PC** — `PC ← PC + 1`.

Exemplo de saída para o programa acima:

```
[LOADK] ACC = 5
PC: 0 | IR: 0010 (LOADK) | ACC: 5 | FLAG_Z: 0
[ADDK] ACC = 8
PC: 1 | IR: 0011 (ADDK) | ACC: 8 | FLAG_Z: 0
[DISP] Exibindo ACC = 8
PC: 2 | IR: 1010 (DISP) | ACC: 8 | FLAG_Z: 0
[HALT] Execucao encerrada.
PC: 3 | IR: 1111 (HALT) | ACC: 8 | FLAG_Z: 0
```

### 3.4 Condição de parada

A execução só termina pela instrução `HALT` (opcode 15), que define `EXECUTANDO = false`. Após `HALT`, novos `*` são ignorados até que outro `*` (ou `RUN` pelo Serial) seja emitido para reiniciar.

---

## 4. Funcionalidades implementadas

### 4.1 F01 — Entrada de instruções por teclado
Implementada em `processarTecla()` e `confirmarInstrucao()`. O usuário digita o opcode em decimal, usa `D` para separar o operando (quando houver) e confirma com `A`. A instrução é armazenada na memória de programa como String (`programa[totalInstrucoes]`) no formato `"opcode operando"`.

### 4.2 F02 — Codificação de mnemônico para opcode
A correspondência decimal/mnemônico é estabelecida pelo array `MNEMONICOS[]`. A função `codificarParaOpcode()` extrai o opcode decimal da String armazenada e retorna o valor como `byte` (4 bits: 0000–1111). A função `opcodeParaBinario()` produz a representação binária de 4 bits, exibida pela instrução `BINC` e em todo log de execução. A função `obterMnemonico()` compõe a representação legível para exibição no Serial Monitor.

### 4.3 F03 — Unidade de Controle (ciclo de instrução)
Implementada em `executarProximaInstrucao()` e `decodificarEExecutar()`. A primeira realiza, em sequência: busca a instrução em `programa[PC]`, codifica o opcode em `IR` via `codificarParaOpcode()`, extrai o operando via `extrairOperando()`, despacha para `decodificarEExecutar()`, imprime o estado interno e incrementa `PC`. A segunda contém o `switch` que seleciona a operação de cada opcode (ULA, periférico, memória). A função trata o término por `HALT`, alterando `EXECUTANDO` para `false`.

### 4.4 F04 — ULA
A função `operacaoULA(valorACC, operando, op)` concentra todas as operações aritméticas e de comparação:

- `LOADK k` → `ACC ← k` (tratado diretamente no `switch`, não passa pela ULA)
- `ADDK k` → `ACC ← operacaoULA(ACC, k, 'A')`
- `SUBK k` → `ACC ← operacaoULA(ACC, k, 'S')`
- `CMPK k` → `FLAG_Z ← (ACC == k)` via `operacaoULA(ACC, k, 'C')` — ACC inalterado

### 4.5 F05 — Leitura do sensor (READ)
A função `lerDistancia()` aciona o `TRIG` por 10 µs, mede o `ECHO` com `pulseIn()` (timeout 30 ms) e converte a duração em centímetros (`duracao * 0.034 / 2`). Valores fora da faixa 1–400 cm são tratados como 0.

### 4.6 F06 — Controle de LEDs
A função `controlarLED(led, ligar)` atende as instruções `LEDON` e `LEDOFF`. O operando indica qual LED (1, 2 ou 3, mapeados para os pinos 42, 43 e 44).

### 4.7 F07 — Controle de buzzer
As instruções `BUZON` e `BUZOFF` acionam diretamente `digitalWrite(PIN_BUZZER, HIGH/LOW)` (pino 45).

### 4.8 F08 — Display de 7 segmentos
A função `exibirDisplay(indice)` aciona os pinos 22–28 conforme a tabela `PADROES[][]`. A função `exibirValorDisplay(valor)` é chamada por `DISP` e trata os três casos:

- `0 ≤ valor ≤ 9` → exibe o dígito (índices 0–9 da tabela).
- `valor > 9` → exibe `E` (índice 10 — overflow) e imprime aviso no Serial Monitor.
- `valor < 0` → exibe `-` (índice 11 — negativo) e imprime aviso no Serial Monitor.

### 4.9 F09 — Memória simulada (STORE / LOADM)
O vetor `MEM[16]` é a memória de dados. `STORE k` grava `ACC` em `MEM[k]`; `LOADM k` lê `MEM[k]` para `ACC`. Endereços inválidos (fora de 0..15) geram mensagem de erro no Serial Monitor.

### 4.10 F10 — Instrução ALERT
Implementada em `executarALERT()`. Lê o sensor via `lerDistancia()` e:

- `distância < 10 cm` → liga LED 1 e buzzer (alerta crítico); desliga LED 2.
- `10 ≤ distância < 20 cm` → liga LED 1, desliga buzzer e LED 2.
- `distância ≥ 20 cm` → desliga LED 1, LED 2 e buzzer.

### 4.11 F11 — Finalização por HALT
A instrução `HALT` (opcode 15) define `EXECUTANDO = false`, apaga o display (índice 12) e imprime mensagem no Serial Monitor. O loop principal passa a ignorar `*` até que um novo `*` ou `RUN` seja emitido.

---

## 5. Tratamento de resultados não representáveis

### 5.1 Overflow
Ocorre quando `DISP` é chamado com `ACC > 9`. Caso típico: somar duas constantes cujo resultado ultrapasse 9 (ex.: `LOADK 7; ADDK 5; DISP`). O display mostra `E` e o Serial Monitor imprime:

```
[DISPLAY] OVERFLOW: valor 12 nao representavel em 1 digito decimal. Exibindo 'E'.
```

### 5.2 Resultado negativo
Ocorre quando `DISP` é chamado com `ACC < 0`. Caso típico: subtrair uma constante maior que `ACC` (ex.: `LOADK 3; SUBK 5; DISP`). O display mostra `-` e o Serial Monitor imprime:

```
[DISPLAY] NEGATIVO: valor -2 nao representavel. Exibindo '-'.
```

---

## 6. Esquema elétrico

### 6.1 Componentes

| Componente             | Especificação                                |
|------------------------|----------------------------------------------|
| Microcontrolador       | Arduino Mega 2560 (ATmega2560)               |
| Teclado                | Matricial 4x4                                |
| Sensor de distância    | HC-SR04                                      |
| LEDs                   | 3 LEDs 5 mm (cores a critério do grupo)      |
| Buzzer                 | Buzzer ativo                                 |
| Display                | 7 segmentos cátodo comum, 1 dígito           |
| Resistores LEDs        | 220 Ω – 330 Ω                                |
| Resistores display     | 220 Ω – 330 Ω por segmento                   |
| Protoboard e jumpers   | Padrão de laboratório                        |

### 6.2 Pinagem (obrigatória conforme enunciado)

| Pino Arduino | Elemento                          |
|--------------|-----------------------------------|
| 22           | Display segmento `a`              |
| 23           | Display segmento `b`              |
| 24           | Display segmento `c`              |
| 25           | Display segmento `d`              |
| 26           | Display segmento `e`              |
| 27           | Display segmento `f`              |
| 28           | Display segmento `g`              |
| 29           | Display ponto decimal (opcional)  |
| 30           | Teclado linha 1                   |
| 31           | Teclado linha 2                   |
| 32           | Teclado linha 3                   |
| 33           | Teclado linha 4                   |
| 34           | Teclado coluna 1                  |
| 35           | Teclado coluna 2                  |
| 36           | Teclado coluna 3                  |
| 37           | Teclado coluna 4                  |
| 40           | Sensor `TRIG`                     |
| 41           | Sensor `ECHO`                     |
| 42           | LED 1                             |
| 43           | LED 2                             |
| 44           | LED 3                             |
| 45           | Buzzer                            |

### 6.3 Regras de montagem

- Cada LED com resistor em série (220–330 Ω) entre pino digital e anodo do LED; cátodo no GND.
- Cada segmento do display com resistor em série entre pino digital e segmento; pino comum (cátodo comum) ao GND.
- Buzzer ativo: pino digital ao terminal `+`, terminal `–` ao GND.
- Sensor HC-SR04: VCC em 5V, GND em GND, `TRIG` no pino 40, `ECHO` no pino 41.
- Teclado matricial: as 8 ligações (4 linhas + 4 colunas) vão diretamente aos pinos digitais 30–37.

---

## 7. Estrutura do sketch

O arquivo `interpretador.ino` está organizado em seções comentadas:

1. **Pinagem** — definição das constantes de pinos.
2. **Configuração do teclado matricial** — mapa de teclas e instância `Keypad`.
3. **Registradores e memória simulada** — declaração de `MEM[16]`, `PC`, `IR`, `ACC`, `FLAG_Z`, `EXECUTANDO`, `programa[]` e `totalInstrucoes`.
4. **Modos de operação** — variável `modoLOAD`.
5. **ISA** — array `MNEMONICOS[]` com os 16 mnemônicos.
6. **Display** — tabela `PADROES[][]` e funções `exibirDisplay()` / `exibirValorDisplay()`.
7. **Sensor** — função `lerDistancia()`.
8. **Saídas** — função `controlarLED()` e `desligarTodasSaidas()`.
9. **ULA** — função `operacaoULA()`.
10. **Codificação** — funções `codificarParaOpcode()`, `extrairOperando()`, `obterMnemonico()`, `opcodeParaBinario()`.
11. **Modo LOAD** — funções `entrarModoLOAD()`, `sairModoLOAD()`, `confirmarInstrucao()` e `imprimirProgramaArmazenado()`.
12. **Modo RUN** — funções `iniciarExecucao()`, `executarProximaInstrucao()` e `decodificarEExecutar()` (UC + ciclo de instrução).
13. **Instrução ALERT** — função `executarALERT()`.
14. **Tratamento de teclas** — função `processarTecla()`.
15. **`setup()` e `loop()`** — inicialização e laço principal (leitura de teclado + Serial Monitor).

### 7.1 Dependências externas

- Biblioteca **Keypad** (Mark Stanley & Alexander Brevig) — instalável pelo Library Manager da IDE Arduino.

---

## 8. Roteiro de testes (correspondente à seção 4 do enunciado)

| Teste | Ação                                                                                  | Resultado esperado                                  |
|-------|---------------------------------------------------------------------------------------|-----------------------------------------------------|
| 4.1   | Carregar `LOADK 5` e executar `BINC`                                                   | Serial mostra opcode `0010`                         |
| 4.2   | Carregar `READ` e `DISP`; executar                                                     | `ACC` recebe distância em cm; display mostra valor  |
| 4.3   | `LOADK 5; ADDK 3; DISP` e `LOADK 9; SUBK 4; DISP`                                      | `ACC` final = 8 e 5; display mostra cada um         |
| 4.4   | `LOADK 7; CMPK 7; BINC`                                                                | `FLAG_Z = 1`; opcode CMPK = `0101`                  |
| 4.5   | `LOADK 6; STORE 0; LOADK 0; LOADM 0; DISP`                                             | Display final = 6                                   |
| 4.6   | `LEDON 1; BUZON; LEDOFF 1; BUZOFF`                                                     | LED e buzzer ligam e desligam conforme instrução    |
| 4.7   | `ALERT` com objeto a <10 cm, entre 10–20 cm e >20 cm                                   | Comportamentos descritos na seção 4.10              |
| 4.8   | `LOADK 7; DISP`                                                                        | Display mostra `7`                                  |
| 4.9   | `LOADK 7; ADDK 5; DISP`                                                                | Display mostra `E`; Serial avisa overflow           |
| 4.10  | `LOADK 3; SUBK 5; DISP`                                                                | Display mostra `-`; Serial avisa valor negativo     |
| 4.11  | `LOADK 1; HALT` seguido de novos `*`                                                   | Após HALT, novos `*` são ignorados                  |
| 4.12  | Identificação no código: `MEM`, `PC`, `IR`, `ACC`, UC, ULA, conversão mnemônico→opcode | Localizar nas seções comentadas do sketch           |

### 8.1 Programa-exemplo do enunciado

Sequência de teclas:

```
#                    (entra em LOAD)
2 D 5 A              (LOADK 5)
3 D 3 A              (ADDK 3)
1 0 A                (DISP)
1 5 A                (HALT)
#                    (sai de LOAD)
*                    (inicia RUN — ou digitar RUN no Serial Monitor)
* * * *              (executa as 4 instruções, uma por *)
```

Saída esperada no Serial Monitor após cada `*`:

```
[LOADK] ACC = 5
PC: 0 | IR: 0010 (LOADK) | ACC: 5 | FLAG_Z: 0
[ADDK] ACC = 8
PC: 1 | IR: 0011 (ADDK) | ACC: 8 | FLAG_Z: 0
[DISP] Exibindo ACC = 8
PC: 2 | IR: 1010 (DISP) | ACC: 8 | FLAG_Z: 0
[HALT] Execucao encerrada.
PC: 3 | IR: 1111 (HALT) | ACC: 8 | FLAG_Z: 0
```

---

## 9. Localização dos elementos arquiteturais no código (mapa rápido para a apresentação)

| Pergunta esperada do professor                | Onde mostrar no `interpretador.ino`                                    |
|-----------------------------------------------|------------------------------------------------------------------------|
| Onde está a memória de dados?                 | Seção 3 — `int MEM[16];`                                               |
| Onde está a memória de programa?              | Seção 3 — `String programa[MAX_INSTRUCOES];`                           |
| Onde está o PC?                               | Seção 3 — `int PC = 0;`                                                |
| Onde está o IR?                               | Seção 3 — `byte IR = 0;`                                               |
| Onde está o ACC?                              | Seção 3 — `int ACC = 0;`                                               |
| Onde está a UC?                               | Seção 12 — funções `executarProximaInstrucao()` e `decodificarEExecutar()` |
| Onde está a ULA?                              | Seção 9 — função `operacaoULA(valorACC, operando, op)`                 |
| Onde ocorre a conversão mnemônico → opcode?   | Seção 10 — funções `codificarParaOpcode()` e `obterMnemonico()`        |
| Onde está o ciclo busca/decodificação/execução? | Seção 12 — comentários explícitos das etapas em `executarProximaInstrucao()` |
| Onde a instrução é interrompida (HALT)?       | Seção 12 — `case 15` em `decodificarEExecutar()` define `EXECUTANDO = false` |

---

## 10. Entregáveis

- **Sketch:** `interpretador.ino`
- **Documentação:** este arquivo `DOCUMENTACAO.md`
- **Repositório GitHub:** (a ser definido) na conta `https://github.com/claytonjasilva`
- **Protótipo físico:** montado em protoboard conforme pinagem da seção 6.2.

---

## 11. Conformidade com o enunciado

| Requisito do enunciado                                            | Atendido em                                                |
|-------------------------------------------------------------------|------------------------------------------------------------|
| 2.1 Variáveis arquiteturais (`MEM`, `PC`, `IR`, `ACC`, etc.)      | Seção 3 do sketch                                          |
| 2.2.1 Modo LOAD ativado por `#`, instruções armazenadas, não executadas | Seções `entrarModoLOAD()` e `processarTecla()`       |
| 2.2.2 Modo RUN com `*` (ou `RUN` no Serial) e avanço instrução por instrução via `*` | `iniciarExecucao()` e `executarProximaInstrucao()` |
| 2.2.3 Encerramento exclusivo por `HALT`                           | `case 15` em `decodificarEExecutar()`                      |
| 2.2.4 Apresentação de PC, IR, ACC, FLAG_Z após cada instrução     | Bloco de impressão ao fim de `executarProximaInstrucao()`  |
| 2.3 ISA com 16 instruções de 4 bits                                | Array `MNEMONICOS[]` + `switch` em `decodificarEExecutar()`|
| 2.4 F01–F11                                                       | Implementadas e mapeadas na seção 4 deste documento        |
| 2.5 Tratamento de overflow e negativo                             | Função `exibirValorDisplay()`                              |
| 3 Esquema elétrico e pinagem obrigatória                          | Seção 6 deste documento e `#define PIN_*` no sketch        |
| 5.1–5.4 Boas práticas, identificação do grupo                     | Cabeçalho do sketch e organização em seções comentadas     |