## LINK REFERENTE AO ESQUEMA DO GRUPO D

[Esquema do Grupo D](https://wokwi.com/projects/462047573689717761)

# Interpretador de Instruções - Arquitetura de Computadores

**Disciplina:** Arquitetura de Computadores  
**Hardware:** Arduino Mega 2560

---

## Sobre o Projeto

Este sistema simula o funcionamento de um processador real. Ele não apenas executa comandos, mas implementa o conceito de **Programa Armazenado** (Arquitetura de Von Neumann). O usuário "carrega" um software via teclado matricial, e a Unidade de Controle processa cada instrução seguindo o ciclo de Fetch-Decode-Execute.

---

## Mapeamento da arquitetura (Software-Hardware)

Para atender aos requisitos técnicos, o sistema foi estruturado com as seguintes abstrações:

- **Unidade de Controle (UC):** Gerencia o fluxo através do `PC` e a decodificação no `IR`.
- **ULA:** Realiza operações aritméticas e atualiza a `FLAG_Z`.
- **Memória de Programa:** Vetor `programa[16]` que armazena as instruções no modo **LOAD**.
- **Memória de Dados:** Vetor `MEM[16]` para armazenamento volátil (instruções `STORE`/`LOADM`).
- **Registradores:** Implementação física das variáveis `PC`, `IR`, `ACC` e `FLAG_Z`.

---

## Instruções de Uso

1. **Modo LOAD (#):** Pressione `#` para iniciar. Digite o Opcode e o Operando (ex: `2 5` para LOADK 5). Use a tecla `C` para confirmar cada linha. Pressione `#` novamente para salvar.
2. **Modo RUN:** Pressione `A` para inicializar o Program Counter (`PC=0`).
3. **Step (\*):** Cada pressão na tecla `*` executa exatamente UMA instrução, permitindo acompanhar o estado dos registradores no Serial Monitor.

---

## Pinagem Obrigatória

O projeto utiliza barramentos organizados para simular a comunicação de dados de um processador:

- **Display 7 Seg:** Pinos 22-28 (Barramento Paralelo)
- **Teclado:** Pinos 30-37 (Interface de Entrada)
- **Sensores/Atuadores:** Pinos 40-45

---

## Conjunto de Instruções (ISA)

O sistema utiliza uma arquitetura de instruções de 4 bits para o Opcode, permitindo até 16 comandos distintos.

| Decimal | Opcode | Mnemônico | Descrição                                                    |
| :------ | :----- | :-------- | :----------------------------------------------------------- |
| 0       | 0000   | NOP       | Nenhuma operação.                                            |
| 1       | 0001   | READ      | Lê o sensor ultrassônico e carrega o valor no `ACC`.         |
| 2       | 0010   | LOADK     | Carrega uma constante `K` no `ACC`.                          |
| 3       | 0011   | ADDK      | Soma uma constante `K` ao `ACC` (Operação da ULA).           |
| 4       | 0100   | SUBK      | Subtrai uma constante `K` do `ACC` (Operação da ULA).        |
| 5       | 0101   | CMPK      | Compara `ACC` com `K` e atualiza a `FLAG_Z`.                 |
| 6       | 0110   | LEDON     | Liga um LED (especificado pelo operando).                    |
| 7       | 0111   | LEDOFF    | Desliga um LED (especificado pelo operando).                 |
| 8       | 1000   | BUZON     | Ativa o alerta sonoro (Buzzer).                              |
| 9       | 1001   | BUZOFF    | Desativa o alerta sonoro (Buzzer).                           |
| 10      | 1010   | DISP      | Exibe o conteúdo do `ACC` no Display de 7 Segmentos.         |
| 11      | 1011   | ALERT     | Executa rotina automática de segurança baseada na distância. |
| 12      | 1100   | BINC      | Exibe o Opcode binário da instrução atual no Serial.         |
| 13      | 1101   | STORE     | Armazena o valor do `ACC` na posição `MEM[X]`.               |
| 14      | 1110   | LOADM     | Carrega em `ACC` o valor contido em `MEM[X]`.                |
| 15      | 1111   | HALT      | Interrompe o Ciclo de Instrução e encerra a execução.        |

---

## Tratamento de Resultados e Exceções

Para garantir a integridade dos dados e a correta interface com o usuário, o sistema trata limites representáveis:

- **Overflow (> 9):** Caso o resultado de uma operação na ULA ou um valor carregado no `ACC` seja maior que a capacidade de um dígito do display, o sistema exibe o caractere `E` (Error) e emite um alerta no Monitor Serial.
- **Resultado Negativo (< 0):** Operações que resultem em valores negativos são sinalizadas no display através do caractere `-`, indicando um valor não representável em formato decimal simples.

 # Guia de Operação: Computador Programável (Ibmec RJ)

Este guia descreve os passos necessários para programar, resetar e executar instruções na arquitetura desenvolvida. O sistema opera baseado no ciclo de Busca, Decodificação e Execução, com controlo de clock manual.

## Mapeamento do Teclado Matricial

| Tecla | Função Técnica | Descrição                                                       |
| ----- | -------------- | --------------------------------------------------------------- |
| #     | Modo LOAD      | Entra ou sai do modo de edição da Memória de Programa.          |
| A     | Reset (PC)     | Repõe o Program Counter a zero (início do programa).            |
| B     | Separador      | Delimita a transição entre o Opcode e o Operando.               |
| C     | Confirmar      | Grava a instrução na memória e avança para o próximo endereço.  |
| *     | Clock          | Executa um ciclo completo (Busca -> Decodificação -> Execução). |

# Passo a Passo para Executar uma Instrução

## 1. Fase de Programação (Gravação na RAM)

Para inserir um programa na memória, o processador deve estar no estado LOAD.

1 - Imprima # para abrir o barramento de escrita.

2 - Digite o Opcode (o código da instrução desejada).

3 - Se a instrução exigir um valor (ex: somar 5), imprima B e digite o Operando (o valor).

4 - Imprima C para confirmar a linha. O sistema avançará automaticamente para o próximo endereço de memória.

5 - Importante: Termine sempre a sua sequência com o comando de paragem: 15 e depois C.

6 - Imprima # para fechar a memória e proteger os dados.

## 2. Fase de Preparação (Reset)

Antes de iniciar a execução, a Unidade de Controle precisa de saber onde começa o programa.

1 - Imprima A. Isto garante que o ponteiro de execução (PC) volta ao endereço 0.

## 3. Fase de Execução (Ciclo de Clock)

O processamento ocorre de forma síncrona a cada pulso manual enviado pelo utilizador.

1 - Imprima * para executar a primeira instrução.

2 - Continue a imprimir * para avançar passo a passo pelo programa.

3 - Observe o Monitor Serial e os periféricos (Display, LEDs e Buzzer) para ver a alteração dos estados internos.

## Exemplo Prático: Teste de Soma e Display

Objetivo: Carregar o número 4, somar 5 e mostrar o resultado (9) no display.

Sequência de Teclas no modo LOAD:

 "# -> 2 B 4 C -> 3 B 5 C -> 10 C -> 15 C ->"

Sequência para Executar:

A -> * -> * -> * -> *

## ⚠️ Notas de Arquitetura

Instruções de 1 Operando: Exigem o uso da tecla B (ex: LOADK, ADDK, STORE).

Instruções de 0 Operando: Não utilizam a tecla B (ex: READ, DISP, HALT, BUZON).

Overflow: Se o resultado de uma operação for maior que 9, o display exibirá a letra "E" (Erro), indicando transbordo de registrador de saída.
