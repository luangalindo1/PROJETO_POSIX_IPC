### README Geral: Projeto de Sistema Automotivo com IPC POSIX (TRABALHO 2)

---
##### Autor:

**Luan Fábio Marinho Galindo**

---

#### **Descrição Geral**

Este projeto foi desenvolvido no âmbito do curso de **Padrão POSIX**, ministrado pelo professor Renato Coral Sampaio, como parte do programa de **Residência Tecnológica Stellantis 2024**. O projeto é composto por três principais componentes:

1. **Painel de Comando** (`command_panel.c`):

   - Interage com o usuário por meio de um menu textual.
   - Envia comandos ao controlador usando **filas de mensagens IPC**.

2. **Controlador do Veículo** (`controller.c`):

   - Gerencia os sensores e acionadores do sistema automotivo.
   - Utiliza **memória compartilhada**, **semafóros** e **GPIO/PWM** para comunicação com hardware físico e o Painel de Comando.

3. **Makefile**:

   - Automatiza o processo de compilação e limpeza dos artefatos do projeto.

O sistema integra conceitos de comunicação interprocessual (IPC POSIX) com a interação de hardware físico, incluindo sensores, motores e acionadores, proporcionando um controle robusto e eficiente.

---

#### **Arquitetura do Sistema**

1. **Painel de Comando:**

   - Recebe comandos do usuário para controlar setas, faróis, pedais e pisca-alerta.
   - Processa mensagens recebidas do controlador, como o comando de encerramento.

2. **Controlador do Veículo:**

   - Monitora os sensores (velocidade, RPM e temperatura do motor).
   - Processa os comandos recebidos do Painel de Comando ou do Dashboard.
   - Controla os atuadores (motores, faróis, setas e luzes de alerta).

3. **Hardware Integrado:**

   - Biblioteca **WiringPi** para GPIO e PWM.
   - Sensores Hall para cálculo de RPM e velocidade.
   - LEDs (no Dashboard) para indicações visuais (setas, faróis e luzes de alerta).

4. **Comunicação IPC POSIX:**

   - **Filas de mensagens:** Troca de comandos e notificações entre o Painel e o Controlador.
   - **Memória compartilhada:** Compartilhamento de dados de sensores e estados dos acionadores.
   - **Semáforos:** Sincronização no acesso a dados compartilhados.

---

#### **Funcionalidades do Sistema**

1. **Painel de Comando:**

   - Oferece um menu interativo para o usuário.
   - Envia comandos ao controlador para ativar/desativar setas, faróis, pedais e pisca-alerta.
   - Trata sinais como `SIGINT` (Ctrl + C) para encerramento seguro.

2. **Controlador do Veículo:**

   - Monitora sensores:
     - Velocidade e RPM (cálculos baseados nos pulsos dos sensores Hall).
     - Temperatura do motor (fórmula dada).
   - Controla os atuadores:
     - Direção do motor (frente, ré, neutro, freio).
     - Setas (pisca de forma assíncrona).
     - Faróis e luzes de freio/alerta.
   - Responde aos comandos recebidos do Painel ou Dashboard.

3. **Makefile:**

   - Compila os dois componentes principais (`command_panel` e `controller`).
   - Remove os arquivos gerados pela compilação com o comando `make clean`.

---

#### **Novidades da Versão Atualizada (TRABALHO 2)**

1. **Painel de Comando:**

   - Adicionado suporte ao sinal `SIGINT` (Ctrl + C) para encerramento seguro.
   - Novos comandos no menu: **Pisca-Alerta** e **Desligar todos os faróis**.
   - Função `clear_input_buffer()` para tratar entradas inválidas.

2. **Controlador do Veículo:**

   - Integração com hardware físico utilizando **WiringPi** (GPIO/PWM).
   - Threads adicionais para controle das setas.
   - Thread dedicada à leitura de comandos do Dashboard.
   - Tratamento adicional para o sinal `SIGINT`.
   - Limpeza abrangente dos recursos antes do encerramento.

3. **Makefile:**

   - Simplificado para compilar apenas os dois componentes principais (sem `sensor_sim`).
   - Adicionada a biblioteca **WiringPi** à compilação do controlador.
   - Melhor organização das variáveis e flags de compilação/linkagem.

---

#### **Como Executar o Sistema**

1. **Pré-requisitos:**

   - Sistema operacional compatível com IPC System V (Linux/Unix).
   - Compilador C (e.g., `gcc`).
   - Biblioteca **WiringPi** instalada.

2. **Compilação:**
   No terminal, execute o seguinte comando na pasta do projeto:

   ```bash
   make
   ```

3. **Execução:** (Dentro da pasta dos arquivos)

   - Inicie o **Controlador** primeiro:
     ```bash
     ./controller
     ```
   - Em seguida, inicie o **Painel de Comando**:
     ```bash
     ./command_panel
     ```

4. **Encerramento:**

   - Use a opção `0` no menu do Painel de Comando.
   - Ou pressione `Ctrl + C` para um encerramento seguro via `SIGINT`.

---

#### **Estrutura do Projeto**

1. **Arquivos Principais:**

   - `command_panel.c`: Implementa o Painel de Comando.
   - `controller.c`: Implementa o Controlador do Veículo.
   - `Makefile`: Automatiza a compilação e limpeza dos artefatos.

2. **Estruturas de Dados:**

   - `SensorData`: Dados dos sensores (velocidade, RPM, temperatura).
   - `Status_trigg`: Estados dos acionadores (setas, faróis).
   - `Message`: Mensagens trocadas entre Painel e Controlador.

3. **IPC e Hardware:**

   - **Filas de mensagens IPC**: Para comandos e notificações.
   - **Memória compartilhada e semáforos**: Para sincronização entre processos.
   - **WiringPi**: Integração com hardware (GPIO/PWM).

---

#### **Cenários de Teste e Validação**

1. **Teste de Comunicação IPC:**
   - Iniciar o controlador e o painel de comando.
   - Enviar comandos do painel ao controlador e verificar a resposta correta (e.g., ativar/desativar faróis e setas).
   - Validar que o controlador processa os comandos enviados e atualiza os atuadores corretamente.

2. **Validação de Sensores:**
   - Simular pulsos nos sensores Hall para medir velocidade e RPM.
   - Confirmar que os valores calculados pelo controlador correspondem aos pulsos gerados.
   - Testar os limites definidos para velocidade e RPM (e.g., redução automática ao ultrapassar os valores máximos).

3. **Teste de Atuação Física (via Dashboard):**
   - Verificar a ativação dos LEDs conectados aos GPIOs:
     - Setas piscando alternadamente.
     - Faróis baixos e altos alternando entre ligado e desligado.
     - Luz de alerta de temperatura acionando corretamente.
   - Monitorar os sinais PWM gerados para os motores (aceleração e frenagem).

4. **Encerramento Seguro:**
   - Usar o sinal `SIGINT` para encerrar o controlador e validar que os recursos (memória compartilhada, semáforos, etc.) são liberados corretamente.
   - Enviar o comando "Encerrar" pelo painel de comando e confirmar que o controlador finaliza.

5. **Robustez do Sistema:**
   - Inserir entradas inválidas no painel de comando e garantir que o sistema continua operando normalmente.
   - Simular falhas nos sensores Hall e verificar o comportamento do controlador (e.g., travamento ou ajuste seguro dos valores).

---

#### **Licença**

Este projeto está licenciado sob a **MIT License**. Sinta-se à vontade para usar, modificar e distribuir conforme desejar.

---

Para mais detalhes sobre cada componente, consulte os READMEs individuais.
