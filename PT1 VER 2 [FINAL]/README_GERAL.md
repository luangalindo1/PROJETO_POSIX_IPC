### README: Projeto de Simulação Automotiva com Comunicação IPC POSIX

---
##### **Author:**
#### Luan Fábio Marinho Galindo

#### **Descrição Geral**
Este projeto foi desenvolvido como parte do curso de **Padrão POSIX**, ministrado pelo professor Renato Coral Sampaio no programa de **Residência Tecnológica Stellantis 2024**. Ele simula um sistema automotivo distribuído que utiliza conceitos de comunicação interprocessual (IPC) POSIX, englobando três componentes principais:

1. **Painel de Comando (`command_panel.c`)**
2. **Controlador do Veículo (`controller.c`)**
3. **Simulador de Sensores (`sensor_sim.c`)**

A integração desses componentes utiliza **memória compartilhada**, **fila de mensagens IPC** e **semáforos POSIX**, permitindo a comunicação eficiente e sincronizada entre processos.

---

#### **Componentes do Projeto**

1. **Painel de Comando (`command_panel.c`):**
   - Oferece uma interface interativa para o usuário enviar comandos ao controlador, como ligar/desligar faróis e setas, ou acionar pedais.
   - Comunica-se com o controlador por meio de uma fila de mensagens (`msgget`, `msgsnd` e `msgrcv`).
   - Recebe mensagens do controlador para ações específicas, como encerramento do sistema.

2. **Controlador do Veículo (`controller.c`):**
   - Responsável por gerenciar os dados dos sensores e executar os comandos recebidos do Painel de Comando.
   - Utiliza memória compartilhada para acessar os dados dos sensores gerados pelo Simulador de Sensores.
   - Implementa regras de segurança, como limites de velocidade, RPM e temperatura.
   - Sincroniza o acesso aos dados compartilhados com semáforos (`sem_wait` e `sem_post`).

3. **Simulador de Sensores (`sensor_sim.c`):**
   - Simula a geração de dados para sensores de velocidade, RPM e temperatura.
   - Armazena os dados em memória compartilhada, permitindo que o Controlador os acesse em tempo real.
   - Cada sensor é executado em uma thread independente, simulando leituras simultâneas e contínuas.

4. **Makefile:**
   - Automatiza a compilação de todos os componentes do projeto.
   - Inclui alvos para compilar individualmente cada programa e um comando para limpar os executáveis gerados.

---

#### **Decisões de Design**

1. **Modularidade:**
   - O sistema foi dividido em três componentes principais para separar as responsabilidades:
     - O Painel de Comando lida com a interação do usuário.
     - O Controlador executa a lógica de negócios e segurança.
     - O Simulador de Sensores gera dados de forma independente.
   - Essa modularização facilita a manutenção, teste e escalabilidade do sistema.

2. **Uso de IPC POSIX:**
   - **Memória Compartilhada (`shmget`, `shmat`)**:
     - Escolhida para armazenar os dados dos sensores devido ao alto desempenho e simplicidade de acesso.
     - Todos os processos compartilham os mesmos dados sem a necessidade de cópias redundantes.
   - **Fila de Mensagens (`msgget`, `msgsnd`, `msgrcv`)**:
     - Utilizada para comunicação assíncrona entre o Painel de Comando e o Controlador.
     - Permite que o Controlador processe comandos sem bloquear o Painel.
   - **Semáforos (`sem_open`, `sem_wait`, `sem_post`)**:
     - Garantem exclusão mútua ao acessar os dados na memória compartilhada.
     - Previnem condições de corrida entre threads e processos.

3. **Threads no Simulador:**
   - Cada sensor é executado em uma thread separada para simular leituras paralelas.
   - Threads foram escolhidas por sua leveza em relação aos processos, já que todas compartilham o mesmo espaço de memória.

4. **Segurança e Resiliência:**
   - O Controlador implementa limitadores para evitar condições críticas:
     - Velocidade acima de 200 km/h ou abaixo de 20 km/h.
     - RPM fora do intervalo de 800 a 8000.
     - Temperatura do motor acima de 140°C.
   - Os sinais (`SIGUSR1`, `SIGUSR2`) são utilizados para pausar ou encerrar a execução do sistema de forma controlada.

---

#### **Fluxo de Execução**

1. **Iniciar o Controlador:**
   - O Controlador cria as memórias compartilhadas e a fila de mensagens, aguardando dados do Simulador e comandos do Painel.

2. **Iniciar o Simulador de Sensores:**
   - O Simulador gera valores para velocidade, RPM e temperatura, armazenando-os na memória compartilhada.
   - Recomenda-se usar o Simulador de Sensores apenas como semente para interagir com o Painel de Comando e Controlador.

3. **Iniciar o Painel de Comando:**
   - O Painel permite ao usuário interagir com o sistema, enviando comandos como ligar/desligar faróis e acionar pedais.

4. **Comunicação:**
   - O Controlador lê os dados dos sensores e aplica os comandos recebidos do Painel.
   - Caso o comando "Encerrar" seja enviado, o Controlador notifica o Painel e encerra o sistema.

---

#### **Testes e Validação**

O sistema foi testado e validado em diversos cenários para garantir sua robustez e conformidade com os requisitos. Abaixo estão os principais cenários abordados:

1. **Aceleração e Redução Gradual:**
   - **Objetivo:** Garantir que o sistema reage corretamente a comandos de aceleração e frenagem.
   - **Resultado:** Velocidade e RPM ajustaram-se corretamente, dentro dos limites estabelecidos.

2. **Velocidade Máxima e Limitador:**
   - **Objetivo:** Validar o comportamento do limitador quando a velocidade atinge 200 km/h.
   - **Resultado:** Velocidade e RPM foram limitados corretamente, conforme esperado.

3. **Comando Inválido:**
   - **Objetivo:** Verificar a resiliência do sistema ao receber comandos não reconhecidos.
   - **Resultado:** Comandos inválidos foram ignorados sem impactar o funcionamento.

4. **Falha de Sensor:**
   - **Objetivo:** Testar o comportamento do Controlador ao perder comunicação com o Simulador de Sensores.
   - **Resultado:** O Controlador manteve os últimos valores conhecidos, sem interrupções.

5. **Desempenho Sob Carga:**
   - **Objetivo:** Avaliar o tempo de resposta e estabilidade ao processar múltiplos comandos.
   - **Resultado:** O sistema respondeu corretamente mesmo com alta frequência de comandos.

---

#### **Como Compilar e Executar**

1. **Compilação:**
   - Abra o terminal na pasta do projeto e execute:
     ```bash
     make
     ```
   - Isso compilará os três componentes do sistema.

2. **Execução:**
   - Inicie os programas na seguinte ordem (recomendada) abrindo três terminais na pasta onde os arquivos se encontram.
     ```bash
     ./controller
     ```
     ```bash
     ./sensor_sim
     ```
     ```bash
     ./command_panel
     ```

3. **Encerramento:**
   - Use a opção `0` no Painel de Comando para encerrar o sistema.
   - Alternativamente, envie o sinal `SIGUSR2` ao Controlador:
     ```bash
     kill -SIGUSR2 <PID_DO_CONTROLLER>
     ```

4. **Limpeza:**
   - Para remover os executáveis gerados, execute:
     ```bash
     make clean
     ```

---

#### **Conceitos POSIX Aplicados**

1. **Intercomunicação de Processos:**
   - Memória compartilhada para comunicação em tempo real.
   - Fila de mensagens para comunicação assíncrona.

2. **Sincronização:**
   - Semáforos para exclusão mútua no acesso aos dados compartilhados.

3. **Threads:**
   - Uso de threads no Simulador para simular sensores paralelos.

4. **Sinais:**
   - Controle do fluxo do sistema com `SIGUSR1` e `SIGUSR2`.

5. **Automação com Makefile:**
   - Uso de makefiles para simplificar a compilação e organização do projeto.

---

#### **Possíveis Melhorias**

1. Adicionar logs persistentes para auditoria e depuração.
2. Implementar uma interface gráfica mais avançada para o Painel de Comando.
3. Incluir novos sensores, como consumo de combustível ou pressão dos pneus.
4. Adicionar suporte a testes automatizados para validar o comportamento do sistema.
5. Configurar tempos dinâmicos para simular diferentes condições operacionais.

---

#### **Licença**
Este projeto é disponibilizado sob a licença **MIT**. Sinta-se à vontade para usar, modificar e distribuir o código conforme desejar.

---

Para mais detalhes sobre cada componente, consulte os READMEs individuais.
