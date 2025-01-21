### README: Controlador do Veículo com Comunicação IPC (Versão Atualizada para o TRABALHO 2)

---

##### Author:
**Luan Fábio Marinho Galindo**

---

#### **Descrição**

O projeto faz parte da grade curricular do curso de **Padrão POSIX**, ministrado pelo professor Renato Coral Sampaio, no programa de **Residência Tecnológica Stellantis 2024**. Este é um dos três artefatos gerados para o **TRABALHO 2**, sendo os outros dois o **Painel de Comando** (command_panel.c) e o **Makefile** desenvolvido para automatizar o processo de compilação. Por questão de organização, cada arquivo possui seu próprio README. No entanto, haverá um README geral que resume o projeto holisticamente.

Neste artefato, implementa-se o **Controlador do Veículo**, responsável por gerenciar os dados dos sensores e o estado dos acionadores. Ele processa comandos enviados pelo Painel de Comando ou pelo Dashboard dado no enunciado do trabalho e interage diretamente com componentes físicos usando **GPIOs** e **PWM**, além de utilizar **memória compartilhada**, **fila de mensagens IPC** e **semáforos**, garantindo a comunicação eficiente e segura entre os componentes do sistema.

---

#### **Novidades da Versão Atualizada**

Esta versão do código possui as seguintes melhorias e novos recursos em comparação com a versão anterior:

1. **Integração com Hardware Físico (GPIO e PWM):**
   - Controle de componentes físicos utilizando a biblioteca **WiringPi**:
     - Motores com controle de direção e potência (PWM).
     - Pedais de acelerador e freio.
     - Sensores Hall para RPM e velocidade.
     - Faróis, setas e luzes indicadoras.
   - Configuração de **interrupções** para os sensores Hall.

2. **Novas Threads para Setas:**
   - Adicionadas threads para controlar o piscar das setas esquerda e direita de forma assíncrona.

3. **Thread para Leitura de Comandos do Dashboard:**
   - Nova thread dedicada para interpretar comandos do Dashboard e executar ações correspondentes nos acionadores.

4. **Melhorias nos Sinais:**
   - Suporte adicional para o sinal `SIGINT` (Ctrl+C), permitindo o encerramento seguro do programa.

5. **Funções Estendidas:**
   - Controle mais detalhado do motor (direção e potência).
   - Cálculos empíricos para velocidade e RPM baseados nos sensores Hall.

6. **Lógica de Limpeza Abrangente:**
   - Desativa todos os componentes físicos antes de liberar recursos.

---

#### **Funcionalidades**

1. **Monitoramento dos Sensores:**
   - Lê e atualiza valores de sensores como:
     - **Velocidade**: Calculada a partir dos pulsos dos sensores Hall das rodas.
     - **RPM do motor**: Calculado a partir dos pulsos do sensor Hall do motor.
     - **Temperatura do motor**: Calculada com base em uma fórmula empírica.
   - Aplica limites de segurança para evitar condições críticas.

2. **Gerenciamento dos Acionadores:**
   - Controle físico de:
     - Setas (esquerda e direita) com funcionalidade de piscar.
     - Faróis (baixo e alto).
     - Pedais de acelerador e freio.
     - Direção do motor (frente, ré, neutro, freio).

3. **Intercomunicação com o Painel de Comando:**
   - **Memória compartilhada**: Armazena dados dos sensores e o estado dos acionadores.
   - **Fila de mensagens**: Recebe comandos do painel e envia notificações, como o comando de encerramento.

4. **Sinalização e Sincronização:**
   - Trata sinais:
     - `SIGUSR1`: Pausa a execução do programa.
     - `SIGUSR2`: Encerra o programa e envia mensagem "Encerrar" ao painel.
     - `SIGINT` (Ctrl+C): Encerra o programa com desativação segura.
   - Usa **semáforos** para sincronizar o acesso a recursos compartilhados.

5. **Relatório de Atividade:**
   - Gera um relatório ao final da execução, detalhando acionamentos dos limitadores.

---

#### **Como Executar**

1. **Pré-requisitos:**
   - Sistema operacional com suporte a IPC POSIX (Linux/Unix).
   - Compilador C (e.g., `gcc`).
   - Biblioteca **WiringPi** instalada.

2. **Compilação:**
   Abra o terminal na pasta do arquivo e execute:
   ```bash
   make
   ```

3. **Execução:**
   Execute o programa com o comando:
   ```bash
   ./controller
   ```
   **Nota:** Recomenda-se executar o Controlador primeiro, depois o Painel de Comando (command_panel), sendo este opcional.

4. **Encerramento:**
   - O Controlador encerra automaticamente ao receber o comando "Encerrar" do Painel de Comando, o sinal `SIGUSR2` ou `SIGINT`.

---

#### **Estrutura do Código**

1. **Headers e Definições:**
   - Bibliotecas padrão, IPC (`sys/ipc.h`, `sys/msg.h`, `sys/shm.h`), sincronização (`semaphore.h`), GPIO/PWM (`wiringPi.h`, `softPwm.h`) e sinais (`signal.h`).
   - Definições para mapeamento dos GPIOs:
     - Motores, pedais, faróis, setas e sensores Hall.

2. **Estruturas de Dados:**
   - `SensorData`: Armazena informações de velocidade, RPM e temperatura.
   - `Status_trigg`: Gerencia o estado dos acionadores (setas, faróis).
   - `Message`: Representa mensagens trocadas com o Painel de Comando.

3. **Funções Principais:**
   - `setup_signals()`: Configura handlers para os sinais.
   - `init_shared_memory()`: Cria e inicializa a memória compartilhada.
   - `init_message_queue()`: Cria a fila de mensagens e limpa mensagens residuais.
   - `init_semaphore()`: Inicializa o semáforo.
   - `init_gpio()`: Configura GPIOs, PWM e interrupções dos sensores Hall.
   - `process_control()`: Loop principal que gerencia sensores, threads, comandos e regras de segurança.
   - `cleanup()`: Libera todos os recursos IPC e desativa os componentes físicos.

---

#### **Novas Threads**

1. **PiscaSetaEsq e PiscaSetaDir:**
   - Controlam o piscar das setas esquerda e direita, respectivamente.
   - Ativadas/desativadas de acordo com o estado do acionador.

2. **ThreadComandosDash:**
   - Lê comandos do Dashboard (pedais, faróis e setas) e executa ações nos componentes físicos.

---

#### **Relatório Final**

Ao encerrar, o programa exibe:
- Número de vezes que os limites de velocidade, RPM e temperatura foram atingidos.
- Número total de acionamentos dos limitadores.

---

#### **Possíveis Melhorias**

1. **Logs Detalhados:**
   - Adicionar logs em arquivos para análise offline.

3. **Expansão de Funcionalidades:**
   - Adicionar suporte a novos sensores e acionadores, além do Cruise Control. Apesar do trabalho especificar a necessidade da implementação do CC, a ausência de pinos de controle de velocidade tornaram essa função sem sentido. 

---

#### **Licença**

Este projeto está licenciado sob a **MIT License**. Sinta-se à vontade para usar, modificar e distribuir o código conforme desejar.

--- 
