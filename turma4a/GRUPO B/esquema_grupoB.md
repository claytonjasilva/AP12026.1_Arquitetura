Arquitetura de Computadores Trabalho AP1

Grupo:
Guilherme Machado Viana (202408356404)
João Gabriel Meirelles Guedes (202402697706)
Enzo Araujo Zambrotti (202407095917)
Pedro Carvalho (202503076936)

Sobre nosso trabalho: 
# 💡 Sistema de Controle com Arduino (LED, Buzzer, Sensor e Teclado)


Trabalho realizado para a matéria de **Arquitetura de Computadores**, com a finalidade de reproduzir o funcionamento de um sistema computacional básico por meio de um microcontrolador.

---

## 📌 Informações sobre o Projeto

O sistema foi criado para mostrar, na prática, os princípios da **arquitetura de computadores**, concentrando-se na entrada, processamento e saída de informações.

O projeto permite:

- Inserção de comandos por meio do teclado matricial
- Execução de comandos armazenados em memória
- Gerenciamento de LEDs
- Ativação do buzzer
- Leitura do sensor de distância
- Apresentação de dados em tela

O funcionamento adota o princípio do **programa armazenado**, parecido com o modelo de Von Neumann.


---

## 🎯 Objetivo

Desenvolver um sistema embarcado capaz de:

- Receber comandos do usuário por teclado
- Processar instruções armazenadas
- Executar ações físicas no hardware
- Simular o fluxo de funcionamento de um processador

---

## 🧩 Componentes Utilizados

- Arduino Mega 2560
- Teclado matricial (Keypad)
- Sensor de distância (ultrassônico)
- LEDs (verde, amarelo e vermelho)
- Buzzer
- Display
- Resistores
- Protoboard
- Jumpers

---

## 🔌 Ligações do Projeto

| Componente | Pinos |
|---|---|
| Teclado | 24, 26, 28, 30, 32, 34, 36 |
| Sensor de distância | 6 (TRIG), 7 (ECHO) |
| LED Verde | 38 |
| LED Amarelo | 39 |
| LED Vermelho | 40 |
| Buzzer | 41 |
| Display | 46 a 53 |

---

## ⚙️ Funcionamento do Sistema

O sistema segue o seguinte fluxo:

1. Entrada de dados pelo teclado
2. Processamento pelo microcontrolador
3. Execução das instruções
4. Saída nos componentes físicos

As saídas podem ser:

- Acionamento de LEDs
- Emissão sonora pelo buzzer
- Exibição de dados no display
- Leitura do sensor

---

## 🎮 Teclas de Controle

| Tecla | Função |
|---|---|
| `#` | Modo LOAD (gravação) |
| `A` | Modo RUN (execução) |
| `*` | Execução passo a passo |
| `C` | Separador de instrução |
| `D` | Salvar na memória |

---

## 🧾 Como Inserir Instruções

```txt
1. Pressione #
2. Digite a instrução
3. Use C (caso tenha operando)
4. Pressione D para salvar
5. Repita para as demais instruções
6. Pressione # para sair
```

---

## ▶️ Como Executar

```txt
1. Pressione A
2. Pressione * para executar passo a passo
```

---

## 🧮 Exemplo de Programa (Soma 3 + 4)

```txt
2 C 3 D -> Carrega 3
3 C 4 D -> Soma 4
10 D -> Exibe resultado
15 D -> Finaliza
```

Resultado esperado:

```txt
7
```

---

## 📏 Leitura do Sensor

```txt
1 D -> Leitura do sensor
10 D -> Exibir no display
```

---

## 🚨 Alerta Automático

```txt
11 D
```

Esse comando ativa o alerta automático com base na leitura do sensor de distância.

---

## 🔊 Controle do Buzzer

```txt
8 D -> Liga
9 D -> Desliga
```

---

## 🧪 Como Executar o Projeto

### 🔌 1. Montagem do Hardware

Realizar a montagem de todos os componentes conforme a tabela de pinos.

Recomendações:

- Utilizar resistores nos LEDs
- Revisar todas as conexões
- Validar alimentação do circuito

---

### 💻 2. Upload do Código

1. Abrir a Arduino IDE
2. Selecionar a placa **Arduino Mega 2560**
3. Selecionar a porta correta
4. Fazer upload do código

---

### ▶️ 3. Execução

- Inserir instruções pelo teclado
- Executar usando `A`
- Utilizar `*` para passo a passo
- Observar comportamento do hardware

---

## ⚠️ Problemas Encontrados

Durante o desenvolvimento, foram encontrados alguns desafios:

- Dificuldade na ligação do teclado
- Falhas na leitura das teclas
- Problemas no acionamento dos LEDs
- Problemas no funcionamento do buzzer
- Integração inicial não funcionava completamente
- Necessidade de **refatoração do código devido à integração do sensor de distância com os demais componentes**

---

## 🛠️ Soluções Aplicadas

Para solucionar os problemas, foram realizadas as seguintes ações:

- Revisão completa das conexões
- Ajuste do mapeamento dos pinos
- Testes individuais dos componentes
- Depuração do código
- Correção da lógica do buzzer
- Refatoração do fluxo principal do sistema
- Integração gradual entre hardware e software

---

## 🧠 Conceitos Aplicados

Este projeto aplica os seguintes conceitos:

- Arquitetura de computadores
- Programa armazenado
- Entrada e saída (I/O)
- Sistemas embarcados
- Processamento sequencial
- Integração hardware/software

---

## ✅ Resultado Final

Ao final do desenvolvimento, o sistema apresentou:

- Funcionamento correto do teclado
- Controle adequado dos LEDs
- Acionamento correto do buzzer
- Leitura funcional do sensor
- Exibição de dados no display
- Execução correta das instruções 
