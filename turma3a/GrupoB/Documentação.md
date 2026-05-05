# Sistema Interpretador de Instruções com Sensor de Distância, Atuadores e Display de 7 Segmentos

**Placa:** Arduino Mega 2560

---

## Integrantes

- Victor Alvarenga Hwang - 202208766005 - TA
- Arthur Calebe Carvalho Da Silva - 202508449013 - TA
- Matheus Cocenzo e Silva – 202107291052 – TA
- Raí Lamper de Avila – 202402627279 - TA

---

## 1. Descrição Geral do Projeto

Este projeto implementa um **Sistema Interpretador de Instruções** baseado no conceito de **programa armazenado**, utilizando o Arduino Mega 2560 como plataforma física.

O sistema permite:

- Inserção de instruções via teclado matricial (modo LOAD)
- Armazenamento das instruções em memória de programa
- Execução controlada passo a passo (modo RUN)
- Processamento de dados de sensor ultrassônico
- Controle de LEDs e buzzer
- Exibição de resultados em display de 7 segmentos
- Simulação de arquitetura de computador

### Conceitos aplicados

- Programa armazenado
- Ciclo de instrução
- ISA (Instruction Set Architecture)
- Unidade de Controle (UC)
- Unidade Lógica e Aritmética (ULA)
- Memória
- Registradores (PC, IR, ACC)
- Entrada e Saída

---

## 2. Elementos Arquiteturais Implementados

| Elemento                  | Implementação                 |
|---------------------------|-------------------------------|
| Memória de Dados          | `int MEM[16];`                |
| Program Counter (PC)      | `int PC;`                     |
| Instruction Register (IR) | `byte IR;`                    |
| Acumulador (ACC)          | `int ACC;`                    |
| Flag de Zero              | `bool FLAG_Z;`                |
| Controle de execução      | `bool EXECUTANDO;`            |
| Memória de Programa       | `String PROGRAMA[16];`        |
| Unidade de Controle       | `executarProximaInstrucao()`  |
| ULA                       | ADDK, SUBK, CMPK              |
| Entrada                   | Teclado 4x4                   |
| Saída                     | LEDs, Buzzer, Serial, Display |
| Sensor                    | HC-SR04                       |
| Display                   | 7 segmentos (`segmentPins`)   |

---

## 3. Modos de Operação

### 3.1 Modo LOAD

- Ativado ao pressionar `#`.

#### Ações

- Limpeza da memória de programa
- PC = 0
- Inicialização do ponteiro de carga
- Armazenamento das instruções

#### Funcionamento

- Instruções digitadas em decimal
- `D` → separador
- `A` → confirmar

#### Exemplo

- LOADK 5 → 2 D 5 A
- ADDK 3 → 3 D 3 A


---

### 3.2 Modo RUN

Ativado ao pressionar `B`.

#### Comportamento

- PC = 0
- Execução controlada
- `*` executa uma instrução por vez

---

## 4. Ciclo de Instrução

- Implementado na função `executarProximaInstrucao()`.

### Etapas

1. **Busca**

```cpp
instrucao = PROGRAMA[PC];
```

2. **Decodificação**

```cpp
decodificarInstrucao();
```

3. **Carga no IR**

```cpp
IR = opcode;
```

4. **Execução**

```cpp
executarInstrucao();
```

5. **Atualização do PC**

PC = PC + 1;


### Encerramento

- Se `HALT` → `EXECUTANDO = false`

---

## 5. ISA Implementada

| Decimal | Binário | Mnemônico | Descrição                   |
|---------|---------|-----------|-----------------------------|
| 0       | 0000    | NOP       | Não realiza operação        |
| 1       | 0001    | READ      | Lê sensor e armazena em ACC |
| 2       | 0010    | LOADK     | Carrega constante           |
| 3       | 0011    | ADDK      | Soma constante              |
| 4       | 0100    | SUBK      | Subtrai constante           |
| 5       | 0101    | CMPK      | Compara                     |
| 6       | 0110    | LEDON     | Liga LED                    |
| 7       | 0111    | LEDOFF    | Desliga LED                 |
| 8       | 1000    | BUZON     | Liga buzzer                 |
| 9       | 1001    | BUZOFF    | Desliga buzzer              |
| 10      | 1010    | DISP      | Exibe ACC                   |
| 11      | 1011    | ALERT     | Resposta por distância      |
| 12      | 1100    | BINC      | Mostra opcode binário       |
| 13      | 1101    | STORE     | MEM[X] = ACC                |
| 14      | 1110    | LOADM     | ACC = MEM[X]                |
| 15      | 1111    | HALT      | Encerra execução            |

---

## 6. Funcionalidades

- **F01 — Entrada**
- **F02 — Decodificação**
- **F03 — Controle**
- **F04 — ULA**
- **F05 — Sensor**
- **F06 — LEDs**
- **F07 — Buzzer**
- **F08 — Exibição**
- **F09 — Memória**
- **F10 — ALERT**
- **F11 — HALT**

---

## 7. Display de 7 Segmentos

### Funções

- `mostrarNumeroDisplay()`
- `mostrarACCNoDisplay()`
- `mostrarErroDisplay()`
- `mostrarNegativoDisplay()`

### Regras

- `0 a 9` → número exibido
- `> 9` → erro
- `< 0` → negativo

---

## 8. Tratamento de Erros

- Overflow → ACC > 9
- Negativo → ACC < 0
- Endereço inválido
- Opcode inválido

---

## 9. Esquema Elétrico

### Componentes

- Arduino Mega 2560
- Teclado 4x4
- Sensor HC-SR04
- 3 LEDs
- Buzzer
- Display 7 segmentos
- Resistores

### Conexões

| Componente | Pino   |
|------------|--------|
| TRIG       | 41     |
| ECHO       | 42     |
| LED1       | 2      |
| LED2       | 3      |
| LED3       | 4      |
| Buzzer     | 45     |
| Display    | 22–31  |

---

## 10. Fluxo do Sistema


# → LOAD
- A → salva instrução
- # → sai LOAD
- B → RUN
- * → executa instrução
- HALT → fim

---

## 11. Plano de Testes

- READ
- ADDK / SUBK
- CMPK
- STORE / LOADM
- LEDON / LEDOFF
- BUZON / BUZOFF
- ALERT
- DISP
- HALT

---

## 12. Conclusão

O projeto implementa uma arquitetura de computador simplificada em um sistema embarcado, integrando:

- Memória
- Registradores
- ISA
- Unidade de Controle
- Unidade Lógica e Aritmética
- Entrada e saída
- Display físico

Permitindo a aplicação prática dos conceitos de **Arquitetura de Computadores**