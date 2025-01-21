### README: Makefile para Gerenciamento do Projeto

---
##### Author:
#### Luan Fábio Marinho Galindo

#### **Descrição**
Este Makefile é parte do projeto desenvolvido no curso de **Padrão POSIX**, ministrado pelo professor Renato Coral Sampaio, no programa de **Residência Tecnológica Stellantis 2024**. Ele foi projetado para gerenciar a compilação dos três principais componentes do sistema:

1. **Painel de Comando (`command_panel.c`)**
2. **Controlador (`controller.c`)**
3. **Simulador de Sensores (`sensor_sim.c`)**

Além disso, o Makefile oferece comandos para limpar os executáveis gerados. Ele organiza e automatiza o processo de compilação, permitindo maior produtividade e controle sobre os artefatos do projeto.

---

#### **Funcionalidades**
1. **Compilação dos Componentes:**
   - Compila os três programas do projeto, garantindo que todas as dependências sejam atendidas.
   - Permite compilar cada componente individualmente.

2. **Automação Parcial:**
   - Oferece um alvo padrão (`all`) que compila todos os componentes.
   - Inclui comandos específicos para cada programa.

3. **Limpeza:**
   - Remove os executáveis gerados pela compilação.

---

#### **Como Utilizar o Makefile**

1. **Pré-requisitos:**
   - Sistema operacional com suporte a ferramentas GNU Make.
   - Compilador C (e.g., `gcc`).

2. **Compilar Todos os Programas:**
   - Execute o seguinte comando no terminal:
     ```bash
     make
     ```
   - Isso irá compilar os três componentes do sistema:
     - Painel de Comando (`command_panel`)
     - Controlador (`controller`)
     - Simulador de Sensores (`sensor_sim`)

3. **Compilar um Programa Específico:**
   - Para compilar apenas um dos programas, use:
     ```bash
     make command_panel
     ```
     ou
     ```bash
     make controller
     ```
     ou
     ```bash
     make sensor_sim
     ```

4. **Limpar os Executáveis:**
   - Para remover os executáveis gerados, execute:
     ```bash
     make clean
     ```

---

#### **Estrutura do Makefile**

1. **Variáveis de Compilação:**
   - `CC`: Define o compilador (`gcc`).
   - `CFLAGS`: Especifica flags de compilação para warnings e otimizações.
   - `LIBM`: Inclui a biblioteca matemática (`-lm`).
   - `LTHREADS`: Adiciona suporte a threads POSIX (`-pthread`).
   - `LRT`: Adiciona suporte a semáforos POSIX (`-lrt`).

2. **Alvos Principais:**
   - **`all`**: Alvo padrão que compila todos os programas.
   - **`command_panel`**: Compila o Painel de Comando.
   - **`controller`**: Compila o Controlador, incluindo bibliotecas para threads, semáforos e matemática.
   - **`sensor_sim`**: Compila o Simulador de Sensores, incluindo bibliotecas para threads e matemática.

3. **Limpeza:**
   - **`clean`**: Remove todos os executáveis gerados.

---

#### **Comandos Disponíveis**

| **Comando**          | **Descrição**                                             |
|-----------------------|----------------------------------------------------------|
| `make`               | Compila todos os componentes do projeto.                 |
| `make command_panel` | Compila apenas o Painel de Comando.                       |
| `make controller`    | Compila apenas o Controlador.                             |
| `make sensor_sim`    | Compila apenas o Simulador de Sensores.                   |
| `make clean`         | Remove os executáveis gerados pela compilação.            |

---

#### **Tratamento de Erros**
- O Makefile verifica erros de compilação e exibe mensagens claras para cada componente.
- Se algum componente falhar durante a compilação, ele interrompe o processo imediatamente.

---

#### **Possíveis Melhorias**
1. Adicionar um alvo `run` para executar os programas em sequência.
2. Incluir suporte para variáveis que definam o diretório de saída dos executáveis.
3. Adicionar logs detalhados para monitorar o processo de compilação.

---

#### **Licença**
Este projeto é disponibilizado sob a licença **MIT**. Sinta-se à vontade para usar, modificar e distribuir este Makefile conforme desejar.

---
