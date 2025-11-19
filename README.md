# IoT Domestic Heat & Humidity Alert — ESP32 + DHT22 + MQTT (ODS 3)
### Monitoramento doméstico de calor e umidade com alerta local e remoto

<p align="center">
  <img src="https://img.shields.io/badge/IoT-ESP32-blue?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/MQTT-PubSub-purple?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/License-MIT-green?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/Status-Working-success?style=for-the-badge"/>
  <img src="https://img.shields.io/badge/Platform-Wokwi-orange?style=for-the-badge"/>
</p>

---

## Resumo do Projeto

Este projeto apresenta um sistema IoT de alerta de calor e baixa umidade, alinhado à ODS 3 – Saúde e Bem-Estar, utilizando:

- ESP32  
- Sensor DHT22  
- Display LCD 16x2 (I2C)  
- Buzzer + LED  
- Relé acionando umidificador  
- MQTT para telemetria e alertas  

O dispositivo calcula o Temperature-Humidity Index (THI) com base na fórmula da NOAA/NWS, disparando alertas locais e remotos quando condições críticas são detectadas.

Simulação no Wokwi: https://wokwi.com/projects/446018892722856961  
Código no GitHub: https://github.com/Marcus-Nastasi/Iot-mack-domestic-heat-hum-alert

---

## 📚 Contexto e Motivação

A Organização Mundial da Saúde (WHO, 2024) e o Ministério da Saúde (2023) alertam para o aumento de eventos extremos de calor. Ambientes internos podem atingir níveis perigosos, aumentando riscos de:

- Desidratação  
- Golpe de calor  
- Agravamento de doenças respiratórias e cardiovasculares  

Este protótipo fornece alertas domésticos automáticos, contribuindo com ações rápidas e alinhando-se às metas da ODS-3.

---

## ⚙️ Arquitetura do Sistema

```
ESP32 → DHT22 → LCD 16x2
         ↓
Cálculo THI (Heat Index)
         ↓
[Limite ultrapassado?] → Buzzer/LED/Relé
         ↓
       MQTT Publish
```

---

## 🔧 Materiais Utilizados

### Microcontrolador
- ESP32-WROOM-32 (Wi-Fi integrado)

### Sensor
- DHT22 (AM2302) — Temperatura e Umidade

### Atuadores
- LED indicador  
- Buzzer ativo  
- Módulo Relé 5V/10A com optoacoplador  

### Periféricos
- Display LCD 16x2 com módulo I2C  

---

## 📡 Comunicação MQTT

### Broker
- Uso de broker público fornecido pelo Wokwi.

### QoS
- QoS 0: telemetria  
- QoS 1: alertas  

### Exemplo de payload enviado

```json
{
  "temp": 29.5,
  "hum": 42.3,
  "humidificador": "ON"
}
```

---

## 🌡️ Cálculo de Risco (Heat Index / THI)

Baseado na NOAA/NWS (2022, 2024).

### Limiar de alerta
- THI > 30 ºC → alerta de calor  
- UR < 45% → alerta de baixa umidade  

---

## 🧪 Demonstração do Protótipo

| Recurso | Link |
|--------|------|
| Simulação completa no Wokwi | https://wokwi.com/projects/446018892722856961 |
| Exemplo MQTT no Wokwi | https://wokwi.com/projects/387000682507243521 |
| Repositório do código | https://github.com/Marcus-Nastasi/Iot-mack-domestic-heat-hum-alert |

---

## 💻 Como Executar o Projeto

### ✔️ 1. Executando no Wokwi (100% Digital)

1. Acesse: https://wokwi.com/projects/446018892722856961  
2. Clique em *Play*  
3. Observe o LCD, o acionamento do relé e a telemetria MQTT  
4. Monitore o console MQTT no painel lateral  

---

### ✔️ 2. Execução em Hardware Real (opcional)

Ajuste as credenciais em `config.h`:

```cpp
#define WIFI_SSID "SeuWifi"
#define WIFI_PASS "SuaSenha"
#define MQTT_BROKER "test.mosquitto.org"
```

### Principais funções do código

- setup_wifi() – conexão Wi-Fi  
- readDHT() – leitura de T/UR  
- computeHeatIndex() – cálculo THI  
- triggerLocalAlert() – alerta (relé, buzzer, LED)  
- publishTelemetry() – envio MQTT  

---

## 📈 Integração com Outros Sistemas

Este protótipo pode ser integrado a:

- Node-RED  
- Home Assistant  
- Aplicativos mobile  
- Dashboards IoT  
- Automação de ventiladores, ar-condicionado e umidificadores inteligentes  

---

## 📝 Exemplos de Funcionamento

### 🔥 Temperatura > 30 ºC
- LCD exibe alerta  
- Relé ON  
- Buzzer ON  
- MQTT envia evento crítico  

### 💧 Umidade < 45%
- LED ON  
- Relé ON  
- Telemetria MQTT prioritária  

---

## 📖 Referências

- WHO. *Climate change, heat and health*. 2024.  
- Ministério da Saúde. *Ondas de calor – orientações*. 2023.  
- IBM Developer. *Why MQTT is good for IoT*. 2021.  
- NOAA/NWS. *Heat Index Equation*. 2022.  
- Espressif Systems. *ESP32-WROOM Datasheet*. 2025.  
- AOSONG Electronics. *DHT22 Datasheet*. 2010.  
- Wokwi. *Simuladores MQTT e ESP32*.  
