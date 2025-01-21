### README: Makefile para Gerenciamento do Projeto (Versão Atualizada para o TRABALHO 2)

---

##### Author:
**Luan Fábio Marinho Galindo**

---

#### **Descrição**

Este Makefile é parte do projeto desenvolvido no curso de **Padrão POSIX**, ministrado pelo professor Renato Coral Sampaio, no programa de **Residência Tecnológica Stellantis 2024**. Ele é projetado para gerenciar a compilação dos dois principais componentes do sistema:

1. **Painel de Comando (`command_panel.c`)**
2. **Controlador (`controller.c`)**

Além disso, o Makefile oferece comandos para limpar os executáveis gerados. Ele organiza e automatiza o processo de compilação, permitindo maior produtividade e controle sobre os artefatos do projeto.

---

#### **Novidades da Versão Atualizada**

Esta versão do Makefile apresenta as seguintes mudanças e melhorias em relação à versão anterior:

1. **Remoção do `sensor_sim`:**
   - O Makefile foi simplificado para incluir apenas os dois componentes principais do sistema: `command_panel` e `controller`. Não houve necessidade de simulação dos sensores já que a placa já está ligada a atuadores e motores.

2. **Uso de `WiringPi`:**
   - Adicionada a biblioteca **WiringPi** (`-lwiringPi`) ao processo de compilação do controlador, necessária para lidar com GPIOs e PWM.

3. **Flags de Linkagem Consolidada:**
   - Criada a variável `LDFLAGS` para consolidar as flags de linkagem comuns entre os componentes (`-pthread`, `-lrt`).

4. **Melhorias na Organização:**
   - As variáveis de compilação e linkagem foram reorganizadas para maior clareza.

5. **Atualização dos Comentários:**
   - Comentários revisados para refletir as mudanças no Makefile.

---

#### **Funcionalidades**

1. **Compilação dos Componentes:**
   - Compila os dois programas do projeto, garantindo que todas as dependências sejam atendidas.
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
   - Biblioteca **WiringPi** instalada para o controlador.

2. **Compilar Todos os Programas:**
   - Execute o seguinte comando no terminal (na mesma pasta que os arquivos estiverem):
     ```bash
     make
     ```
   - Isso irá compilar os dois componentes do sistema:
     - Painel de Comando (`command_panel`)
     - Controlador (`controller`)

3. **Compilar um Programa Específico:**
   - Para compilar apenas um dos programas, use:
     ```bash
     make command_panel
     ```
     ou
     ```bash
     make controller
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
   - `LDFLAGS`: Consolida flags comuns para threads (`-pthread`) e semáforos (`-lrt`).
   - `WIRINGPI`: Adiciona suporte à biblioteca WiringPi para o controlador.
   - `LIBM`: Inclui a biblioteca matemática (`-lm`).

2. **Alvos Principais:**
   - **`all`**: Alvo padrão que compila todos os programas.
   - **`command_panel`**: Compila o Painel de Comando, incluindo flags de linkagem comuns.
   - **`controller`**: Compila o Controlador, adicionando WiringPi e outras dependências específicas.

3. **Limpeza:**
   - **`clean`**: Remove todos os executáveis gerados.

---

#### **Comandos Disponíveis**

| **Comando**          | **Descrição**                                             |
|-----------------------|----------------------------------------------------------|
| `make`               | Compila todos os componentes do projeto.                 |
| `make command_panel` | Compila apenas o Painel de Comando.                       |
| `make controller`    | Compila apenas o Controlador.                             |
| `make clean`         | Remove os executáveis gerados pela compilação.            |

---

#### **Tratamento de Erros**

- O Makefile verifica erros de compilação e exibe mensagens claras para cada componente.
- Se algum componente falhar durante a compilação, o processo é interrompido imediatamente.

---

#### **Possíveis Melhorias**

1. Adicionar um alvo `run` para executar os programas em sequência.
2. Incluir suporte para variáveis que definam o diretório de saída dos executáveis.
3. Adicionar logs detalhados para monitorar o processo de compilação.

---

#### **Licença**

Este projeto está licenciado sob a **MIT License**. Sinta-se à vontade para usar, modificar e distribuir conforme desejar.

--- 
