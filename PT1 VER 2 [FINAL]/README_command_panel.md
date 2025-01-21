### README: Painel de Comando com Fila de Mensagens IPC

---
##### Author:
#### Luan Fábio Marinho Galindo

#### **Descrição**
O projeto faz parte da grade curricular do curso de **Padrão POSIX**, ministrado pelo professor Renato Coral Sampaio, no programa de **Residência Tecnológica Stellantis 2024**. Este projeto é um dos quatro artefatos gerados para a execução do **TRABALHO 1**, sendo os outros três o **Controlador** (controller.c), o **Simulador dos Sensores** (sensor_sim.c) e o **makefile** desenvolvido para automatizar o processo de compilação. Por questão de organização, cada arquivo terá seu readme. No entanto, haverá um outro readme resumindo o projeto holisticamente. 

Neste artefato, implementa-se um **Painel de Comando** que interage com um controlador utilizando **filas de mensagens IPC**. O painel permite que o usuário envie comandos relacionados ao controle de setas, faróis e pedais de um sistema automotivo, além de receber mensagens do controlador para ações específicas, como o encerramento do sistema.

---

#### **Funcionalidades**
1. **Menu Interativo:**
   - Permite ao usuário selecionar opções de controle, como ligar/desligar setas e faróis ou acionar pedais.
   - Comando especial para encerrar o sistema.

2. **Comunicação com o Controlador:**
   - O painel envia mensagens para o controlador usando uma fila de mensagens identificada por uma chave única (`MSG_KEY`).
   - Verifica mensagens recebidas do controlador antes de cada interação com o menu.

3. **Mensagens Enviadas:**
   - Cada comando do painel é encapsulado em uma mensagem contendo:
     - Tipo da mensagem (`msg_type` = 1 para comandos do painel).
     - Texto do comando (e.g., "Ligar Seta Esquerda").

4. **Mensagens Recebidas:**
   - O painel monitora mensagens do controlador com tipo `msg_type` = 2.
   - Caso receba o comando `"Encerrar"`, o painel encerra sua execução.

---

#### **Como Executar**

1. **Pré-requisitos:**
   - Sistema operacional com suporte a IPC POSIX (Linux/Unix).
   - Compilador C (e.g., `gcc`).

2. **Compilação:**
   Abra o terminal na pasta do arquivo, e execute o makefile:
   ```bash
   make
   ```
   
4. **Execução:**
   Execute o programa com o comando:
   ```bash
   ./command_panel
   ```
  **Nota**: Recomenda-se executar primeiramente o Controlador (controler), em seguida o Simulador dos Sensores (sensor_sim)
  e por fim o Painel de Comando. 
  
5. **Interação:**
   - Escolha opções no menu inserindo o número correspondente ao comando desejado.
   - Para encerrar, selecione a opção `0`.

---

#### **Menu de Comandos**

| **Opção** | **Comando**                  |
|-----------|------------------------------|
| 1         | Ligar Seta Esquerda          |
| 2         | Ligar Seta Direita           |
| 3         | Ligar Farol Baixo            |
| 4         | Ligar Farol Alto             |
| 5         | Acionar Pedal do Acelerador  |
| 6         | Acionar Pedal do Freio       |
| 7         | Desligar Seta Esquerda       |
| 8         | Desligar Seta Direita        |
| 9         | Desligar Farol Baixo         |
| 10        | Desligar Farol Alto          |
| 11        | Desligar Farol               |
| 0         | Sair                         |

---

#### **Estrutura do Código**

1. **Headers e Definições:**
   - Bibliotecas padrão e definidas pelo sistema (`stdio.h`, `stdlib.h`, `string.h`, `sys/ipc.h`, `sys/msg.h`, `unistd.h`).
   - Constante `MSG_KEY` para identificação da fila de mensagens.

2. **Estrutura de Mensagem:**
   - Estrutura `Message` contém:
     - `long msg_type`: Tipo da mensagem.
     - `char command[100]`: Texto do comando.

3. **Funções Principais:**
   - `display_menu()`: Exibe o menu de opções ao usuário.
   - `send_message(int msg_queue_id, Message msg)`: Envia mensagens para o controlador.
   - `main()`: Gerencia a execução do programa, menu, envio e recebimento de mensagens.

---

#### **Tratamento de Erros**
- Falha na criação/acesso à fila de mensagens: Imprime erro e encerra.
- Erros ao enviar mensagens: Imprime mensagem de erro e encerra.
- Entradas inválidas no menu: Exibe mensagem de erro e reapresenta o menu.

---

#### **Possíveis Melhorias**
1. Adicionar logs para depuração.
2. Modularizar o código para melhorar a legibilidade.
3. Implementar tratamento de timeout para mensagens não enviadas/recebidas.
4. Expandir o sistema para suportar comandos adicionais.

---

#### **Licença**
Este projeto é disponibilizado sob a licença **MIT**. Sinta-se à vontade para usar, modificar e distribuir o código conforme desejar.

---
