### README Introdutório: Projeto de Sistema Automotivo com IPC POSIX

---
##### **Autor:**
**Luan Fábio Marinho Galindo**

#### **Descrição Geral**
Este documento serve como uma introdução geral ao projeto de Sistema Automotivo com IPC POSIX, desenvolvido no âmbito do curso de **Padrão POSIX**, ministrado pelo professor Renato Coral Sampaio, no programa de **Residência Tecnológica Stellantis 2024**. 

O projeto utiliza **memória compartilhada**, **filas de mensagens** e **semáforos POSIX**, simulando um sistema automotivo distribuído. Na segunda versão do projeto, foram integrados recursos de hardware físico, ampliando a complexidade e aplicabilidade do sistema.

Cada código fonte dentro de suas respectivas pastas possui um README específico detalhando seu funcionamento, e cada versão do projeto conta com um README geral descrevendo as particularidades do trabalho correspondente. Este documento oferece um resumo e destaca as mudanças principais entre as duas versões.

---
#### **Componentes Principais**

1. **Painel de Comando** (`command_panel.c`):
   - Interface interativa para o usuário controlar o sistema (e.g., ligar/desligar faróis, setas e pedais).
   - Comunicação com o Controlador via **filas de mensagens**.
   - Recebe sinais e notificações do Controlador.

2. **Controlador do Veículo** (`controller.c`):
   - Gerencia sensores (velocidade, RPM, temperatura) e atuadores (faróis, setas, motores).
   - Utiliza **memória compartilhada** e **semáforos** para sincronização de dados.
   - Implementa integração com hardware usando **WiringPi** para controle de GPIO/PWM (na segunda versão).

3. **Simulador de Sensores** (`sensor_sim.c` - apenas na primeira versão):
   - Simula sensores de velocidade, RPM e temperatura.
   - Armazena os dados em memória compartilhada para acesso pelo Controlador.
   - Cada sensor opera em uma thread independente, simulando medições paralelas.

4. **Makefile**:
   - Automatiza a compilação dos componentes do projeto e a limpeza dos arquivos gerados.

---
#### **Destaques das Mudanças do Trabalho 1 para o Trabalho 2**

1. **Integração com Hardware Físico:**
   - O Trabalho 2 adiciona suporte a hardware através da biblioteca **WiringPi**, permitindo o controle de uma placa Raspberry Pi e motores reais conectados a ela.

2. **Melhorias no Painel de Comando:**
   - Adicionado suporte ao sinal `SIGINT` (Ctrl + C) para encerramento seguro.
   - Inclusão de novos comandos, como **Pisca-Alerta** e **Desligar Todos os Faróis**.
   - Implementada uma função para tratar entradas inválidas.

3. **Aprimoramentos no Controlador do Veículo:**
   - Threads adicionais para controle das setas e leitura de comandos do Dashboard.
   - Limpeza automática dos recursos IPC (memória compartilhada, semáforos) no encerramento.

4. **Substituição do Simulador de Sensores:**
   - O componente `sensor_sim.c`, presente no Trabalho 1, foi removido no Trabalho 2 e substituído por dados reais de sensores conectados via hardware.

5. **Simplificação do Projeto:**
   - O Makefile foi otimizado para compilar apenas os componentes necessários.

6. **Testes e Validação:**
   - Novos cenários de validação incluindo integração de hardware, testes de pinos e sinais PWM, e robustez contra entradas inválidas ou falhas de sensores.

---
#### **Licença**
Este projeto está licenciado sob a **MIT License**. Sinta-se à vontade para usar, modificar e distribuir conforme desejar.

