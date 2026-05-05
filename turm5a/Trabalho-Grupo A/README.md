# Projeto de Arquitetura de Computadores - Mega 2560

## Sobre o Projeto

Este projeto implementa um interpretador de instruções baseado na arquitetura de Von Neumann, utilizando um Arduino Mega 2560 e outros materiais. O sistema é capaz de receber comandos via teclado matricial, decodificar e executá-los, interagindo com periféricos conforme o ensinado (E/S), por exemplo, sensores de distância, LEDs, buzzer e display de 7 segmentos.

Em suma, ele demonstra o funcionamento de uma CPU simples, onde o sistema executa instruções armazenadas em memória, utilizando registradores como PC, IR e ACC, seguindo o ciclo de busca, decodificação e execução, e interagindo com dispositivos físicos comentados anteriormentee

## Operação

O sistema possui dois estados isolados de funcionamento:

1. **Modo LOAD:** Permite a inserção de instruções mnemônicas via teclado matricial, convertendo-as para opcodes e armazenando-as sequencialmente na memória.
2. **Modo EXECUTE:** Inicia o ciclo de máquina contínuo, buscando os opcodes na memória, decodificando e acionando a ULA e as portas de I/O.



\*\*DETALHES\*\*





PC (Program Counter)

Responsável por indicar qual instrução será executada. Ele funciona como um marcador que avança a cada passo do programa.

IR (Instruction Register)

Armazena a instrução atual que está sendo executada.

ACC (Acumulador)

Utilizado para armazenar valores e resultados de operações matemáticas, como soma e subtração.

MEM (Memória de Dados)

Estrutura com 16 posições utilizada para armazenar valores temporários.

FLAG\_Z (Flag Zero)

Indica se o resultado de uma operação é zero, auxiliando em comparações.



O sistema suporta um conjunto básico de instruções, como:



LOADK → carrega um valor no acumulador

ADDK → soma um valor ao acumulador

SUBK → subtrai um valor

LEDON / LEDOFF → controla LEDs

BUZON / BUZOFF → controla o buzzer

DISP → exibe o valor no display

STORE / LOADM → manipula memória

HALT → encerra o programa



## Hardware

* **Placa:** Arduino Mega 2560
* **Entrada:** Teclado Matricial 4x4, Sensor de Distância (Ultrassônico)
* **Saída:** Display de 7 Segmentos, LEDs indicadores, Buzzer, Serial Monitor

## Como Executar?

1. Clone o repositório
2. Monte o circuito conforme o diagrama, desenho.
3. Faça o upload do código `main.ino` para o Arduino Mega.
4. Abra o Serial Monitor
5. Se divirta!

Obs: Existe um print e um desenho nada intuitivo na pasta de sketch, entretanto, há uma forma mais dinâmica de interagir com um esboço do projeto através do seguinte [link](https://wokwi.com/projects/461948142420101121)

## Como colaborar?

1. Clone o repositório
2. Crie uma branch separada com `git checkout -b sua-branch`
3. Faça commit da sua contribuição e faça um push
4. Abra um PR no repositório



## Tecnologias Utilizadas

* C (Linguagem)
* [Wokwi](https://wokwi.com/projects/461948142420101121)
* Arduino IDE

## 👥 Equipe

* \[MariaSinesio]
* \[Henrique]

## LICENSE

[MIT](LICENSE)

