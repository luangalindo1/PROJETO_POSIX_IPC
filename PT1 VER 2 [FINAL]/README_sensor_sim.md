### README: Simulador de Sensores com Memória Compartilhada e Semáforos

---
##### Author:
#### Luan Fábio Marinho Galindo

#### **Descrição**
Este projeto é parte integrante do curso de **Padrão POSIX**, ministrado pelo professor Renato Coral Sampaio, no programa de **Residência Tecnológica Stellantis 2024**. O projeto inclui quatro componentes principais: **Painel de Comando** (`command_panel.c`), **Controlador** (`controller.c`), **Simulador de Sensores** (`sensor_sim.c`) e um **makefile** para gerenciamento da compilação.

Neste artefato, implementa-se um **Simulador de Sensores** responsável por gerar valores de velocidade, RPM e temperatura do motor de um veículo. Ele utiliza **memória compartilhada** para armazenar os dados dos sensores, permitindo que outros componentes (como o Controlador) acessem essas informações em tempo real. O acesso à memória compartilhada é sincronizado usando **semáforos**, garantindo a exclusão mútua e evitando condições de corrida.

---

#### **Funcionalidades**
1. **Geração de Dados de Sensores:**
   - **Velocidade**: Valores aleatórios entre 0 e 200 km/h.
   - **RPM**: Valores aleatórios entre 500 e 8000.
   - **Temperatura**: Calculada dinamicamente com base na velocidade e no RPM usando uma fórmula específica (fornecida no enunciado do projeto).

2. **Memória Compartilhada:**
   - Armazena os dados dos sensores em uma estrutura (`SensorData`) para que outros processos possam acessá-los.

3. **Sincronização:**
   - Utiliza um semáforo para garantir que apenas uma thread ou processo acesse a memória compartilhada por vez.

4. **Threads Independentes:**
   - Cada sensor (velocidade, RPM, temperatura) é simulado em uma thread separada, rodando continuamente.

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
   ./sensor_sim
   ```
  **Nota**: Deixe o simulador rodar algumas vezes depois feche-o usando `Ctrl+C` para manipular o Controlador via Painel de Comandos. Isso porque fica difícil de interagir com o Controlador se os Sensores ficarem enviando valores aleatórios (conforme especificado) repetidamente para o controlador. 
4. **Encerramento:**
   - O programa continua gerando dados até que seja encerrado manualmente (`Ctrl+C`).

---

#### **Estrutura do Código**

1. **Headers e Definições:**
   - Bibliotecas padrão e POSIX (`stdio.h`, `stdlib.h`, `pthread.h`, `sys/ipc.h`, `sys/shm.h`, `semaphore.h`).
   - Chaves únicas (`SHM_KEY_SENSORS`) para identificar a memória compartilhada.

2. **Estruturas de Dados:**
   - `SensorData`: Estrutura para armazenar os dados dos sensores.
     - `velocidade`: Velocidade do veículo em km/h.
     - `rpm`: Rotação do motor em RPM.
     - `temperatura`: Temperatura do motor em graus Celsius.

3. **Funções Principais:**
   - `sensor_velocidade()`: Gera valores aleatórios de velocidade e os armazena na memória compartilhada.
   - `sensor_rpm()`: Gera valores aleatórios de RPM e os armazena na memória compartilhada.
   - `sensor_temperatura()`: Calcula a temperatura do motor com base na velocidade e no RPM.
   - `init_shared_memory()`: Cria e associa a memória compartilhada.
   - `init_semaphore()`: Inicializa o semáforo para sincronização.

4. **Threads:**
   - Cada sensor é executado em uma thread independente para simular leituras simultâneas.

---

#### **Tratamento de Erros**
- Falha ao criar memória compartilhada: Imprime mensagem de erro e encerra o programa.
- Falha ao criar semáforo: Imprime mensagem de erro e encerra o programa.
- Falha ao criar threads: Imprime mensagem de erro e encerra o programa.

---

#### **Possíveis Melhorias**
1. Implementar encerramento controlado para liberar recursos IPC.
2. Implementar um timer para o programa encerrar automaticamente ou entrar em estado dormente para posterior reativação.
3. Adicionar logs detalhados para monitorar a geração de dados em tempo real.
4. Expandir o sistema para incluir novos sensores (e.g., consumo de combustível).
5. Configurar os intervalos de geração de dados para simular diferentes condições de uso.

---

#### **Licença**
Este projeto é disponibilizado sob a licença **MIT**. Sinta-se à vontade para usar, modificar e distribuir o código conforme desejar.

---
