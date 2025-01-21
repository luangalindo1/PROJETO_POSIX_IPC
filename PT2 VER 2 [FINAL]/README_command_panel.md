### README: Painel de Comando com Fila de Mensagens IPC (Versão Atualizada para o TRABALHO 2)

---
##### Autor:
**Luan Fábio Marinho Galindo**

#### **Descrição**
Este projeto faz parte do curso de Padrão POSIX, ministrado pelo professor Renato Coral Sampaio, no programa de **Residência Tecnológica Stellantis 2024**. Este é um dos três artefatos gerados para o **TRABALHO 2**, sendo os outros dois: o **Controlador** (controller.c) e o **Makefile** para automação da compilação. Cada componente possui seu próprio README, com um README geral resumindo o projeto como um todo.

Este artefato implementa um **Painel de Comando** que se comunica com um controlador utilizando **filas de mensagens IPC**. A interação ocorre através de um menu que permite ao usuário enviar comandos relacionados a setas, faróis, pedais e outras funções automotivas. Este painel também processa mensagens recebidas do controlador, como o comando de encerramento.

---

#### **Novidades da Versão Atualizada para o TRABALHO 2**

Esta versão do código apresenta melhorias e novos recursos em relação à versão anterior:

1. **Handler de Sinal SIGINT:**
   - Adicionada funcionalidade para tratar o sinal `Ctrl + C` (SIGINT).
   - Ao pressionar `Ctrl + C`, o programa:
     - Imprime uma mensagem de encerramento.
     - Envia o comando `"Encerrar"` ao controlador.
     - Finaliza a execução de forma segura.

2. **Novos Comandos no Menu:**
   - Adicionados comandos para ligar/desligar o **pisca-alerta** e desligar **todos os faróis** (baixo e alto).
   - Menu atualizado para incluir esses novos comandos:
     - `3`: Ligar Pisca-Alerta.
     - `10`: Desligar Pisca-Alerta.
     - `13`: Desligar Farol (ambos).

3. **Função de Limpeza do Buffer de Entrada:**
   - Criada a função `clear_input_buffer()` para limpar o buffer de entrada após leituras inválidas ou quando necessário, melhorando a robustez da interação com o usuário.

4. **Variáveis Globais e Simplificação de Funções:**
   - A variável `msg_queue_id` foi movida para o escopo global, eliminando a necessidade de passá-la como argumento para funções como `send_message()`.

5. **Tratamento Aprimorado de Erros:**
   - Melhor manejo de entradas inválidas no menu:
     - O programa informa o erro e reapresenta o menu.
     - Limpa o buffer de entrada para evitar repetições de erro.

6. **Mensagens Mais Informativas:**
   - Mensagens de status no terminal foram ajustadas para fornecer mais contexto ao usuário.

---

#### **Funcionalidades**
1. **Menu Interativo:**
   - O usuário pode selecionar opções para controlar setas, faróis, pedais e pisca-alerta, além de comandos para desligar todos os faróis.
   - Comando especial para encerrar o painel e o controlador.

2. **Comunicação com o Controlador:**
   - O painel envia mensagens ao controlador via fila de mensagens identificada por uma chave única (`MSG_KEY`).
   - Mensagens recebidas com o tipo `msg_type = 2` são processadas. Caso o comando seja `"Encerrar"`, o painel finaliza.

3. **Tratamento de Sinais:**
   - O sinal `SIGINT` (`Ctrl + C`) é tratado para garantir um encerramento seguro, enviando o comando `"Encerrar"` ao controlador antes de sair.

4. **Robustez no Tratamento de Entradas:**
   - Entradas inválidas são detectadas e tratadas, com limpeza do buffer para evitar comportamentos inesperados.

---

#### **Como Executar**

1. **Pré-requisitos:**
   - Sistema operacional compatível com IPC POSIX (Linux/Unix).
   - Compilador C (e.g., `gcc`).

2. **Compilação:**
   Abra o terminal na pasta do arquivo e execute:
   ```bash
   make
   ```

3. **Execução:**
   Para iniciar o painel:
   ```bash
   ./command_panel
   ```
   **Nota:** Recomenda-se executar primeiro o **Controlador** (controller) antes de iniciar o Painel de Comando.

4. **Encerramento Seguro:**
   - Use a opção `0` no menu para encerrar normalmente.
   - Ou pressione `Ctrl + C` para um encerramento seguro via sinal SIGINT.

---

#### **Menu de Comandos**

| **Opção** | **Comando**                  |
|-----------|------------------------------|
| 1         | Ligar Seta Esquerda          |
| 2         | Ligar Seta Direita           |
| 3         | Ligar Pisca-Alerta           |
| 4         | Ligar Farol Baixo            |
| 5         | Ligar Farol Alto             |
| 6         | Acionar Pedal do Acelerador  |
| 7         | Acionar Pedal do Freio       |
| 8         | Desligar Seta Esquerda       |
| 9         | Desligar Seta Direita        |
| 10        | Desligar Pisca-Alerta        |
| 11        | Desligar Farol Baixo         |
| 12        | Desligar Farol Alto          |
| 13        | Desligar Farol (ambos)       |
| 0         | Sair                         |

---

#### **Estrutura do Código**

1. **Headers e Definições:**
   - Bibliotecas padrão e para IPC (`stdio.h`, `stdlib.h`, `string.h`, `sys/ipc.h`, `sys/msg.h`, `unistd.h`, `signal.h`).
   - Constante `MSG_KEY` para identificação da fila de mensagens.

2. **Estrutura de Mensagem:**
   - Estrutura `Message` contém:
     - `long msg_type`: Tipo da mensagem.
     - `char command[100]`: Texto do comando.

3. **Funções Principais:**
   - `display_menu()`: Exibe o menu de opções ao usuário.
   - `send_message(Message msg)`: Envia mensagens para o controlador.
   - `sigint_handler(int signal)`: Trata o sinal SIGINT (`Ctrl + C`).
   - `clear_input_buffer()`: Limpa o buffer de entrada.
   - `main()`: Gerencia o loop principal, menu e comunicação com o controlador.

---

#### **Tratamento de Erros**
1. Falha ao criar/acessar a fila de mensagens:
   - Mensagem de erro é exibida, e o programa encerra.
2. Falha ao enviar mensagens:
   - Mensagem de erro é exibida, e o programa encerra.
3. Entradas inválidas no menu:
   - Mensagem informativa é exibida, e o menu é reapresentado.

---

#### **Possíveis Melhorias**
1. Adicionar logs para facilitar a depuração.
2. Expandir o sistema para suportar novos comandos.
3. Melhorar a modularidade para separar ainda mais responsabilidades.
4. Implementar timeout para mensagens não enviadas/recebidas.

---

#### **Licença**
Este projeto está licenciado sob a **MIT License**. Sinta-se à vontade para usar, modificar e distribuir o código conforme desejar.

--- 
