#include <WiFi.h>
#include "UbidotsEsp32Mqtt.h"

// Definindo os pinos dos LEDs do semáforo 1
const int pinAmarelo1 = 25;
const int pinVerde1 = 26;
const int pinVermelho1 = 27;

// Definindo os pinos dos LEDs do semáforo 2
const int pinAmarelo2 = 32;
const int pinVerde2 = 33;
const int pinVermelho2 = 35;

// Definindo o pino do LDR
const int ldrPin = 34;

// Definindo os pinos do sensor ultrassônico
const int trigPin = 2;
const int echoPin = 4;

// Variáveis para armazenar valores de sensores e estados
int ldrValue = 0;
long distancia = 0;
const int limiarLdr = 2000;
const int distanciaProxima = 5;
int botao1 = 1; // 0 = Desativado, 1 = Ativado (inicialmente ativado)
int botaoSemaforo1 = 0; // 0 = Desativado, 1 = Ativado
int botaoSemaforo2 = 0; // 0 = Desativado, 1 = Ativado

// Variáveis para o Wi-Fi e Ubidots
const char *UBIDOTS_TOKEN = "BBUS-aL6Ea8DCCoCqVS8smnan7205Zp71S5";
const char *WIFI_SSID = "Inteli.Iot";
const char *WIFI_PASS = "@Intelix10T#";
const char *DEVICE_LABEL = "esp32_t12_g05";
const char *VARIABLE_LABEL1 = "botao1";
const char *VARIABLE_LABEL_LDR = "ldrValue";
const char *VARIABLE_LABEL_DIST = "distancia";
const char *VARIABLE_LABEL_SEMAFORO1 = "semaforo1";
const char *VARIABLE_LABEL_SEMAFORO2 = "semaforo2";

Ubidots ubidots(UBIDOTS_TOKEN);

// Função para medir a distância com o sensor ultrassônico
long medirDistancia() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duracao = pulseIn(echoPin, HIGH);
  long distancia = duracao * 0.034 / 2; // Conversão para centímetros
  return distancia;
}

// Função de callback do Ubidots para tratar a mensagem recebida
void callback(char *topic, byte *payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  if (String(topic) == VARIABLE_LABEL_SEMAFORO1) {
    botaoSemaforo1 = message.toInt();
    Serial.print("Botão Semáforo 1: ");
    Serial.println(botaoSemaforo1);
  } else if (String(topic) == VARIABLE_LABEL_SEMAFORO2) {
    botaoSemaforo2 = message.toInt();
    Serial.print("Botão Semáforo 2: ");
    Serial.println(botaoSemaforo2);
  } else {
    botao1 = message.toInt();
    if (botao1 == 0) {
      Serial.println("Sistema desativado. Desligando todos os LEDs...");
      // Desliga todos os LEDs ao desativar o sistema
      digitalWrite(pinAmarelo1, LOW);
      digitalWrite(pinVerde1, LOW);
      digitalWrite(pinVermelho1, LOW);
      digitalWrite(pinAmarelo2, LOW);
      digitalWrite(pinVerde2, LOW);
      digitalWrite(pinVermelho2, LOW);
    } else {
      Serial.println("Sistema ativado!");
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Configura os pinos dos LEDs como saída
  pinMode(pinAmarelo1, OUTPUT);
  pinMode(pinVerde1, OUTPUT);
  pinMode(pinVermelho1, OUTPUT);
  pinMode(pinAmarelo2, OUTPUT);
  pinMode(pinVerde2, OUTPUT);
  pinMode(pinVermelho2, OUTPUT);

  // Configura o pino do LDR como entrada
  pinMode(ldrPin, INPUT);

  // Configura os pinos do sensor ultrassônico
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  ubidots.setDebug(true);
  ubidots.setCallback(callback);
  ubidots.setup();

  // Conecta ao Wi-Fi
  Serial.print("Conectando ao Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println(" Conectado com sucesso!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  // Inscreve-se nas variáveis para receber atualizações
  ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL1);
  ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL_SEMAFORO1);
  ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL_SEMAFORO2);
}

void loop() {
  if (!ubidots.connected()) {
    ubidots.reconnect();
    ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL1);
    ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL_SEMAFORO1);
    ubidots.subscribeLastValue(DEVICE_LABEL, VARIABLE_LABEL_SEMAFORO2);
  }
  ubidots.loop();

  // Verifica se o sistema está ativado
  if (botao1 == 1) {
    ldrValue = analogRead(ldrPin);
    distancia = medirDistancia();

    // Exibe os valores dos botões no console serial
    Serial.print("Botão Semáforo 1: ");
    Serial.println(botaoSemaforo1);
    Serial.print("Botão Semáforo 2: ");
    Serial.println(botaoSemaforo2);

    // Envia os dados dos sensores para o Ubidots
    ubidots.add(VARIABLE_LABEL_LDR, ldrValue);
    ubidots.publish(DEVICE_LABEL);
    ubidots.add(VARIABLE_LABEL_DIST, distancia);
    ubidots.publish(DEVICE_LABEL);

    // Verifica os botões dos semáforos
    if (botaoSemaforo1 == 1) {
      // Apenas o semáforo 1 fica amarelo
      digitalWrite(pinAmarelo1, HIGH);
      digitalWrite(pinVerde1, LOW);
      digitalWrite(pinVermelho1, LOW);
      Serial.println("Semáforo 1: Amarelo");

      // Apaga o semáforo 2
      digitalWrite(pinAmarelo2, LOW);
      digitalWrite(pinVerde2, LOW);
      digitalWrite(pinVermelho2, LOW);
      Serial.println("Semáforo 2: Desligado");

    } else if (botaoSemaforo2 == 1) {
      // Apenas o semáforo 2 fica amarelo
      digitalWrite(pinAmarelo2, HIGH);
      digitalWrite(pinVerde2, LOW);
      digitalWrite(pinVermelho2, LOW);
      Serial.println("Semáforo 2: Amarelo");

      // Apaga o semáforo 1
      digitalWrite(pinAmarelo1, LOW);
      digitalWrite(pinVerde1, LOW);
      digitalWrite(pinVermelho1, LOW);
      Serial.println("Semáforo 1: Desligado");

    } else {
      // Lógica normal dos semáforos
      if (ldrValue < limiarLdr) {
        // Ambiente escuro: ambos amarelos
        digitalWrite(pinAmarelo1, HIGH);
        digitalWrite(pinAmarelo2, HIGH);
        Serial.println("Ambiente escuro: Ambos os semáforos amarelos");
      } else if (distancia < distanciaProxima) {
        // Objeto próximo: ambos vermelhos
        digitalWrite(pinVermelho1, HIGH);
        digitalWrite(pinVermelho2, HIGH);
        digitalWrite(pinAmarelo1, LOW);
        digitalWrite(pinAmarelo2, LOW);
        Serial.println("Objeto próximo: Ambos os semáforos vermelhos");
      } else {
        // Alterna entre as fases dos semáforos
        digitalWrite(pinVerde1, HIGH);
        digitalWrite(pinVermelho2, HIGH);
        delay(1000);
        Serial.println("Semáforo 1: Verde, Semáforo 2: Vermelho");

        digitalWrite(pinAmarelo1, LOW);
        digitalWrite(pinAmarelo2, LOW);
        Serial.println("Semáforo 1: Amarelo, Semáforo 2: Amarelo");

        delay(500);

        digitalWrite(pinVerde1, LOW);
        digitalWrite(pinVermelho1, HIGH);

        digitalWrite(pinVerde2, HIGH);
        digitalWrite(pinVermelho2, LOW);
        Serial.println("Semáforo 1: Vermelho, Semáforo 2: Verde");

        delay(1000);
      }
    }
  }

  delay(1000); // Aguarda um segundo antes de repetir a verificação
}
