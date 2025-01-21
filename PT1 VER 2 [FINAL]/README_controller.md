### README: Controlador do Veículo com Comunicação IPC

---
##### Author:
#### Luan Fábio Marinho Galindo

#### **Descrição**
O projeto faz parte da grade curricular do curso de **Padrão POSIX**, ministrado pelo professor Renato Coral Sampaio, no programa de **Residência Tecnológica Stellantis 2024**. 
Este projeto é um dos quatro artefatos gerados para a execução do **TRABALHO 1**, sendo os outros três o **Painel de Comando** (command_panel.c), o **Simulador dos Sensores** (sensor_sim.c) 
e o **makefile** desenvolvido para automatizar o processo de compilação. Por questão de organização, cada arquivo terá seu readme. No entanto, haverá um outro readme resumindo o projeto holisticamente. 

Neste artefato, implementa-se o **Controlador do Veículo**, responsável por gerenciar os dados dos sensores e o estado dos acionadores. 
Ele também processa os comandos enviados pelo Painel de Comando por meio de **memória compartilhada**, **fila de mensagens IPC** e **semáforos**, garantindo a comunicação 
eficiente e segura entre os componentes do sistema.

---

#### **Funcionalidades**
1. **Monitoramento dos Sensores:**
   - Lê e atualiza valores de sensores como velocidade, RPM e temperatura do motor.
   - Aplica limites de segurança para evitar condições críticas (e.g., superaquecimento, excesso de velocidade).

2. **Gerenciamento dos Acionadores:**
   - Controla o estado de setas, faróis e outros componentes com base nos comandos do Painel de Comando.

3. **Intercomunicação com o Painel de Comando:**
   - Utiliza memória compartilhada para armazenar dados dos sensores e estado dos acionadores.
   - Recebe comandos do Painel de Comando por meio de uma fila de mensagens.
   - Envia mensagens ao Painel para sinalizar eventos como o encerramento do sistema.

4. **Sinalização e Sincronização:**
   - Pausa ou encerra o programa com base nos sinais recebidos (`SIGUSR1` para pausar e `SIGUSR2` para encerrar).
   - Sincroniza o acesso aos recursos compartilhados usando semáforos.

5. **Relatório de Atividade:**
   - Gera um relatório ao final da execução, detalhando quantas vezes os limitadores foram acionados.

---

#### **Como Executar**

1. **Pré-requisitos:**
   - Sistema operacional com suporte a IPC POSIX (Linux/Unix).
   - Compilador C (e.g., `gcc`).

2. **Compilação:**
   Abra o terminal na pasta do arquivo e execute o makefile:
   ```bash
   make
   ```

3. **Execução:**
   Execute o programa com o comando:
   ```bash
   ./controller
   ```
   **Nota:** Recomenda-se executar o Controlador primeiro, seguido pelo Simulador dos Sensores (sensor_sim) e, por fim, o Painel de Comando (command_panel).

4. **Encerramento:**
   - O Controlador encerra automaticamente ao receber o comando "Encerrar" do Painel de Comando ou o sinal `SIGUSR2`.

---

#### **Estrutura do Código**

1. **Headers e Definições:**
   - Bibliotecas padrão e POSIX (`stdio.h`, `stdlib.h`, `sys/ipc.h`, `sys/msg.h`, `semaphore.h`, entre outras).
   - Chaves únicas (`SHM_KEY_SENSORS`, `SHM_KEY_TRIGGERS`, `MSG_KEY`) para identificar recursos IPC.

2. **Estruturas de Dados:**
   - `SensorData`: Armazena informações de velocidade, RPM e temperatura.
   - `Status_trigg`: Gerencia o estado dos acionadores (setas, faróis).
   - `Message`: Representa mensagens trocadas com o Painel de Comando.

3. **Funções Principais:**
   - `setup_signals()`: Configura handlers para os sinais (`SIGUSR1`, `SIGUSR2`).
   - `init_shared_memory()`: Cria e inicializa a memória compartilhada.
   - `init_message_queue()`: Cria a fila de mensagens e limpa mensagens residuais.
   - `init_semaphore()`: Inicializa o semáforo para sincronização.
   - `process_control()`: Loop principal que monitora sensores, processa comandos e aplica regras de segurança.
   - `cleanup()`: Libera todos os recursos IPC antes de encerrar.

4. **Relatório:**
   - Exibe o número de vezes que os limitadores de velocidade, RPM e temperatura foram acionados.

---

#### **Tratamento de Erros**
- Falha ao criar/acessar memória compartilhada, fila de mensagens ou semáforo: Imprime mensagem de erro e encerra o programa.
- Comandos inválidos do Painel: Ignorados com mensagem de aviso.
- Recursos alocados são sempre liberados com a função `cleanup()`.

---

#### **Possíveis Melhorias**
1. Adicionar logs detalhados para monitoramento em tempo real.
2. Implementar paralelismo avançado para melhorar o desempenho.
3. Criar alertas visuais ou sonoros para condições críticas (e.g., superaquecimento).
4. Expandir as funcionalidades para suportar novos sensores e acionadores.

---

#### **Licença**
Este projeto é disponibilizado sob a licença **MIT**. Sinta-se à vontade para usar, modificar e distribuir o código conforme desejar.

---
