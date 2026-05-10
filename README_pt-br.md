# 🔧 Hardware Test Bench – ESP32

Um sistema modular baseado em ESP32 para testes de componentes eletrônicos durante prototipagem e validação de PCBs.

O projeto evoluiu de um simples sistema via Serial para uma **plataforma web configurável de validação de hardware**, focada em facilitar testes e melhorar rastreabilidade.

---

## 🌐 Interface Web

O sistema pode ser acessado diretamente pelo navegador.

- Controle de testes via interface
- Monitoramento em tempo real
- Visualização de logs
- Configuração de dispositivos

---

## 🚀 Funcionalidades

### 🔹 Sistema Base
- Testes individuais de componentes
- Execução de teste completo
- Arquitetura modular
- Feedback em tempo real

---

### 🔹 Funcionalidades Avançadas

#### 🧩 Custom Test Builder
Permite criar testes diretamente pela interface:

- Selecionar tipo de dispositivo
- Definir GPIOs
- Executar testes sem alterar o código

---

#### ✅ Sistema PASS/FAIL Híbrido
- Validação automática
- Sugestão de resultado
- Confirmação manual
- Possibilidade de override

---

#### 🕒 Sistema de Logs com Timestamp
- Registro de execução de testes
- Inclui:
  - início
  - fim
  - resultado
  - mensagens

---

#### 📥 Exportação de Logs
- Download direto pela interface
- Formatos:
  - `.txt`
  - `.csv`

---

#### 📡 Modos Wi-Fi (AP + STA)

- **Modo AP (Access Point)**  
  - ESP32 cria sua própria rede  
  - Acesso via `192.168.4.1`

- **Modo STA**  
  - Conecta ao Wi-Fi do usuário  

- Configuração inicial via navegador  
- Não é necessário editar código

---

## 🧰 Hardware Testado

- Buzzer
- Relay
- Sensor ultrassônico
- Botão
- LED
- Sensor PIR

---

## ⚙️ Como Usar

### 🔹 Primeira vez

1. Ligue o ESP32  
2. Conecte no Wi-Fi:  
   `TestBench_ESP32`  
3. Acesse:  
   `http://192.168.4.1`  
4. Configure sua rede Wi-Fi  

---

### 🔹 Uso normal

1. Acesse pelo navegador  
2. Configure dispositivos  
3. Execute testes  
4. Acompanhe logs  

---

## 🧾 Exemplo de Log
17:50:32 - Teste iniciado: PIR
17:50:59 - Teste finalizado: PASS

---

## 🎯 Status do Projeto

Projeto funcional como **plataforma de validação de hardware**.

Próximos passos:
- adicionar novos dispositivos
- melhorar robustez
- expandir uso prático

---

## 👩‍💻 Autor

**Maria Eduarda Pereira de Jesus**

Estudante de Engenharia da Computação  
Sistemas Embarcados | Hardware | IoT  

---