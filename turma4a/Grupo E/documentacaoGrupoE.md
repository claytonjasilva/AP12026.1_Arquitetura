

# Documentação do Projeto: Sistema Interpretador de Instruções (Arduino Mega 2560)

## 1. Identificação da Equipe
* [cite_start]**Lucas Calil Cavalcanti** — TA (Trabalhou Ativamente)[cite: 1].
* [cite_start]**Marco Antonio** — TA (Trabalhou Ativamente)[cite: 1].

## 2. Descrição Geral da Solução
O projeto consiste em um sistema embarcado que simula o funcionamento de um processador real através de um **Interpretador de Instruções**. O sistema permite que um usuário insira comandos via teclado matricial, os armazene em uma memória de programa e os execute seguindo o ciclo de instrução clássico (Busca, Decodificação e Execução).

### 2.1 Mapeamento de Elementos Arquiteturais
[cite_start]Para atender aos requisitos da disciplina, os conceitos de hardware foram abstraídos no código através das seguintes estruturas[cite: 2]:

* [cite_start]**Memória de Dados (`MEM[16]`)**: Vetor para armazenamento de variáveis globais do programa[cite: 2].
* [cite_start]**Memória de Instruções (`programa[16]`)**: Vetor de strings que armazena os mnemônicos inseridos no modo LOAD[cite: 6].
* **Program Counter (`PC`)**: Registrador que indica o endereço da instrução atual.
* **Instruction Register (`IR`)**: Armazena o opcode da instrução que está sendo processada no momento.
* [cite_start]**Acumulador (`ACC`)**: Registrador principal para operações da ULA e armazenamento temporário de dados[cite: 2].
* [cite_start]**Flag Zero (`FLAG_Z`)**: Bit de status que indica se o resultado de uma comparação foi igual[cite: 2].

## 3. Funcionamento do Sistema

[cite_start]O sistema opera obrigatoriamente em dois modos, alternados pela tecla `#`[cite: 15].

### 3.1 Modo LOAD (Entrada de Programa)
1.  [cite_start]O usuário pressiona `#` para limpar a memória e iniciar a carga[cite: 16, 49].
2.  [cite_start]As instruções são digitadas no formato `OPCODE OPERANDO` (ex: `2 5` para LOADK 5)[cite: 16].
3.  [cite_start]A tecla **'B'** funciona como o "Enter" para confirmar e armazenar a instrução na memória de programa[cite: 47].

### 3.2 Modo RUN (Execução)
1.  [cite_start]Pressiona-se a tecla **'A'** para ativar o modo de execução[cite: 19].
2.  [cite_start]A execução é **passo a passo**: cada vez que a tecla `*` é pressionada, o sistema realiza um ciclo completo (Busca -> Decodifica -> Executa)[cite: 21].
3.  [cite_start]O monitor serial exibe o estado interno dos registradores após cada instrução[cite: 26].
4.  [cite_start]O programa para ao encontrar a instrução `HALT` (Opcode 15)[cite: 38].

## 4. Pinagem e Conexões (Padrão IBMEC)

| Elemento | Pinos Arduino |
| :--- | :--- |
| **Display 7 Segmentos (a-g)** | [cite_start]22, 23, 24, 25, 26, 27, 28 [cite: 9] |
| **Teclado Matricial (Linhas/Colunas)** | [cite_start]30-33 (Linhas) / 34-37 (Colunas) [cite: 11] |
| **Sensor Ultrassônico (Trig/Echo)** | [cite_start]40 / 41 [cite: 7] |
| **LEDs 1, 2 e 3** | [cite_start]42, 43, 44 [cite: 8] |
| **Buzzer** | [cite_start]45 [cite: 8] |

## 5. Tratamento de Limites (Display e ULA)

Como o display de 7 segmentos é limitado a um dígito, implementamos as seguintes proteções:
* [cite_start]**Overflow**: Se o valor no `ACC` for superior a 9, o display exibe a letra **'E'** e o Serial Monitor alerta sobre o erro[cite: 44].
* [cite_start]**Negativo**: Se o resultado de uma subtração (`SUBK`) for menor que 0, o display exibe o símbolo **'-'**[cite: 45].

## 6. Lógica da Instrução ALERT
A instrução `ALERT` (Opcode 11) automatiza a segurança baseada na distância:
* [cite_start]**< 10cm**: Liga LED 1 e Buzzer[cite: 41].
* [cite_start]**10cm - 20cm**: Liga apenas LED 1[cite: 42].
* [cite_start]**> 20cm**: Desliga ambos[cite: 43].

---

