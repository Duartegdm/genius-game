#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <PubSubClient.h>
 
// ================= WIFI =================
const char* ssid = "Wokwi-GUEST";
const char* password = "";
 
// ================= WIFI =================
void setup_wifi() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
 
  Serial.println("Conectando ao WiFi...");
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("\nWiFi conectado!");
}
// ================= MQTT =================
const char* mqtt_server = "broker.hivemq.com";
const char* topic = "disruptive/genius/rank";
 
WiFiClient espClient;
PubSubClient client(espClient);
 
// ================= MQTT =================
void reconnect() {
  while (!client.connected()) {
    Serial.println("Conectando ao MQTT...");
 
    if (client.connect("ESP32_Genius")) {
      Serial.println("MQTT conectado!");
    } else {
      Serial.print("Erro: ");
      Serial.print(client.state());
      delay(2000);
    }
  }
}
 
// ===================== PINOS =====================
const int in_vermelho = 25;
const int in_verde    = 33;
const int in_azul     = 32;
const int in_amarelo  = 35;
 
const int out_vermelho = 5;
const int out_verde    = 17;
const int out_azul     = 16;
const int out_amarelo  = 4;
 
const int buzzer = 19;
 
LiquidCrystal_I2C lcd(0x27, 16, 2);
 
// ===================== ESTADOS =====================
typedef enum {
  WELCOME,
  INSTRUCTIONS,
  TAG_SELECT,
  STARTUP,
  GAME,
  GAME_OVER
} estados;
 
estados estadoAtual;
 
// ===================== TAG =====================
char tag[4] = {'A', 'A', 'A', '\0'};
int tagPos = 0;
 
 
#define MAX_NIVEL 100
 
int gameState = 0;
int cores[MAX_NIVEL] = {};
int nivel  = 0;
int atual  = 0;
int pontos = 0;
 
 
const int DELAY_BASE     = 600;   
const int DELAY_ENTRE    = 200;  
const int DELAY_MIN      = 100;   
 
 
int calcularDelay(int nivelAtual) {
  float fator = pow(0.90, nivelAtual);          
  int d = (int)(DELAY_BASE * fator);
  return max(d, DELAY_MIN);
}
 
// ===================== DEBOUNCE =====================
unsigned long lastDebounce = 0;
const unsigned long DEBOUNCE_MS = 50;
 
// ===================== UTILITÁRIOS LCD =====================
void lcdClear() { lcd.clear(); }
 
void lcdPrint(int col, int row, const char* msg) {
  lcd.setCursor(col, row);
  lcd.print(msg);
}
 
void lcdPrintStr(int col, int row, String msg) {
  lcd.setCursor(col, row);
  lcd.print(msg);
}
 
// ===================== LEITURA DE BOTÕES =====================
int leInput() {
  static int lastV = HIGH, lastG = HIGH, lastB = HIGH, lastA = HIGH;
  int v = digitalRead(in_vermelho);
  int g = digitalRead(in_verde);
  int b = digitalRead(in_azul);
  int a = digitalRead(in_amarelo);
 
  if ((millis() - lastDebounce) > DEBOUNCE_MS) {
    if (lastV == HIGH && v == LOW) { lastDebounce = millis(); lastV = v; return 1; }
    if (lastG == HIGH && g == LOW) { lastDebounce = millis(); lastG = g; return 2; }
    if (lastB == HIGH && b == LOW) { lastDebounce = millis(); lastB = b; return 3; }
    if (lastA == HIGH && a == LOW) { lastDebounce = millis(); lastA = a; return 4; }
  }
  lastV = v; lastG = g; lastB = b; lastA = a;
  return 0;
}
 
void aguardaSoltar() {
  delay(200);
  while (leInput() != 0) delay(50);
}
 
// ===================== SONS =====================
void tocarSomCor(int cor) {
  int freqs[] = {0, 170, 164, 485, 500};
  tone(buzzer, freqs[cor], 400);
}
 
void tocarSomBotao(int cor) {
  int freqs[] = {0, 170, 164, 485, 500};
  tone(buzzer, freqs[cor], 150);
}
 
void tocarSomErro() {
  tone(buzzer, 100, 300); delay(150);
  tone(buzzer, 80, 300);  delay(300);
  noTone(buzzer);
}
 
void tocarSomAcerto() {
  tone(buzzer, 600, 100); delay(100);
  tone(buzzer, 800, 100); delay(100);
  noTone(buzzer);
}
 
void tocarSomNavegacao() {
  tone(buzzer, 1000, 60); delay(60);
  noTone(buzzer);
}
 
void tocarSomConfirma() {
  tone(buzzer, 800, 80);   delay(80);
  tone(buzzer, 1000, 80);  delay(80);
  tone(buzzer, 1200, 120); delay(120);
  noTone(buzzer);
}
 
// ===================== TELAS =====================
 
void runWelcome() {
  lcdClear();
  lcdPrint(2, 0, "BEM-VINDO AO");
  lcdPrint(4, 1, "GENIUS! :)");
 
  int leds[]  = {out_vermelho, out_verde, out_azul, out_amarelo};
  int freqs[] = {170, 164, 485, 500};
  for (int i = 0; i < 4; i++) {
    digitalWrite(leds[i], HIGH);
    tone(buzzer, freqs[i], 250);
    delay(220);
    digitalWrite(leds[i], LOW);
  }
  noTone(buzzer);
  delay(1200);
  estadoAtual = INSTRUCTIONS;
}
 
void runInstructions() {
  lcdClear(); lcdPrint(0, 0, "Repita a sequen-"); lcdPrint(0, 1, "cia de cores!"); delay(2500);
  lcdClear(); lcdPrint(0, 0, "Use os 4 botoes"); lcdPrint(0, 1, "coloridos."); delay(2500);
  lcdClear(); lcdPrint(0, 0, "Erre->Fim de jogo"); lcdPrint(0, 1, "Acerte -> +1 pt!"); delay(2500);
  lcdClear(); lcdPrint(0, 0, "Escolha sua TAG"); lcdPrint(0, 1, "de 3 letras!"); delay(2500);
  lcdClear(); lcdPrint(0, 0, "V/G/A: mudam as"); lcdPrint(0, 1, "letras. AM=OK!"); delay(2500);
  estadoAtual = TAG_SELECT;
}
 
void desenhaTag() {
  lcdClear();
  lcdPrint(0, 0, "Sua TAG:");
  lcd.setCursor(0, 1);
  for (int i = 0; i < 3; i++) {
    lcd.print(' '); lcd.print(tag[i]); lcd.print(' ');
    if (i < 2) lcd.print('|');
  }
  lcd.setCursor(1 + tagPos * 4, 1);
  lcd.blink();
}
 
void runTagSelect() {
  tag[0] = 'A'; tag[1] = 'A'; tag[2] = 'A';
  tagPos = 0;
  desenhaTag();
 
  while (true) {
    int btn = leInput();
    if (btn == 0) { delay(10); continue; }
 
    if (btn == 1) {
      tag[0] = (tag[0] - 'A' + 1) % 26 + 'A';
      tocarSomNavegacao(); tagPos = 0; desenhaTag(); aguardaSoltar();
    } else if (btn == 2) {
      tag[1] = (tag[1] - 'A' + 1) % 26 + 'A';
      tocarSomNavegacao(); tagPos = 1; desenhaTag(); aguardaSoltar();
    } else if (btn == 3) {
      tag[2] = (tag[2] - 'A' + 1) % 26 + 'A';
      tocarSomNavegacao(); tagPos = 2; desenhaTag(); aguardaSoltar();
    } else if (btn == 4) {
      lcd.noBlink();
      tocarSomConfirma();
      lcdClear();
      lcdPrint(0, 0, "TAG confirmada:");
      lcd.setCursor(6, 1); lcd.print(tag);
      delay(1800);
      estadoAtual = STARTUP;
      return;
    }
  }
}
 
// ================= PUBLICAR =================
void publicarResultado(String tag, int score) {
  if (!client.connected()) {
    reconnect();
  }
 
  client.loop();
 
  String payload = "{";
  payload += "\"tag\":\"" + tag + "\",";
  payload += "\"score\":" + String(score);
  payload += "}";
 
  client.publish(topic, payload.c_str());
 
  Serial.println("Resultado enviado:");
  Serial.println(payload);
}
 
void runStartup() {
  lcdClear();
  lcdPrint(3, 0, "INICIANDO...");
  lcdPrint(0, 1, "Prepare-se!");
 
  int leds[]  = {out_vermelho, out_verde, out_azul, out_amarelo};
  int freqs[] = {170, 164, 485, 500};
  for (int i = 0; i < 4; i++) {
    digitalWrite(leds[i], HIGH);
    tone(buzzer, freqs[i], 250);
    delay(220);
    digitalWrite(leds[i], LOW);
  }
  noTone(buzzer);
  delay(800);
 
  lcdClear();
  lcdPrint(3, 0, "** INICIAR **");
  lcdPrint(0, 1, "Boa sorte, ");
  lcd.print(tag); lcd.print("!");
 
  for (int i = 0; i < 4; i++) digitalWrite(leds[i], HIGH);
  tone(buzzer, 880, 400);
  delay(400);
  for (int i = 0; i < 4; i++) digitalWrite(leds[i], LOW);
  noTone(buzzer);
  delay(800);
 
  pontos = 0; nivel = 0; atual = 0; gameState = 0;
  memset(cores, 0, sizeof(cores));
  estadoAtual = GAME;
}
 
// ===================== EXIBE COR COM DELAY DINÂMICO =====================
void mostraCorAdaptativa(int cor, int delayOn, int delayOff) {
  int leds[]  = {0, out_vermelho, out_verde, out_azul, out_amarelo};
  int freqs[] = {0, 170, 164, 485, 500};
 
  digitalWrite(leds[cor], HIGH);
  tone(buzzer, freqs[cor], delayOn);
  delay(delayOn);
  digitalWrite(leds[cor], LOW);
  noTone(buzzer);
  delay(delayOff);
}
 
// ===================== JOGO =====================
void atualizaLcdJogo() {
  lcdClear();
  String linhaZero = String(tag) + " Nv:" + String(nivel + 1);
 
  if (nivel >= 20)       linhaZero += " >>>";
  else if (nivel >= 10)  linhaZero += " >>";
  else if (nivel >= 5)   linhaZero += " >";
 
  lcdPrintStr(0, 0, linhaZero);
  lcdPrintStr(0, 1, "Pontos: " + String(pontos));
}
 
void runGame() {
 
  if (nivel >= MAX_NIVEL) {
    lcdClear();
    lcdPrint(0, 0, "PARABENS! 100nv!");
    lcdPrintStr(0, 1, "Pts: " + String(pontos));
    tocarSomConfirma();
    delay(3000);
    estadoAtual = GAME_OVER;
    return;
  }
 
  if (gameState == 0) {
    cores[nivel] = random(1, 5);
    atualizaLcdJogo();
    delay(600);
 
    int dOn  = calcularDelay(nivel);               
    int dOff = max(dOn / 3, DELAY_MIN / 2);        
 
    lcdClear();
    lcdPrint(0, 0, "Olhe e memorize");
    if (dOn <= 150)       lcdPrint(0, 1, "Veloc.: MAX!");
    else if (dOn <= 250)  lcdPrint(0, 1, "Veloc.: alta");
    else if (dOn <= 400)  lcdPrint(0, 1, "Veloc.: media");
    else                  lcdPrint(0, 1, "Veloc.: baixa");
    delay(500);
 
    for (int i = 0; i <= nivel; i++) {
      mostraCorAdaptativa(cores[i], dOn, dOff);
    }
 
    lcdClear();
    lcdPrint(0, 0, "Agora e sua vez!");
    lcdPrint(0, 1, "Repita a ordem!");
 
    gameState = 1;
    aguardaSoltar();
 
  } else {
    if (atual <= nivel) {
      int cor = leInput();
      if (cor != 0) {
        tocarSomBotao(cor);
 
        int leds[] = {0, out_vermelho, out_verde, out_azul, out_amarelo};
        digitalWrite(leds[cor], HIGH);
        delay(150);
        digitalWrite(leds[cor], LOW);
 
        if (cores[atual] == cor) {
          tocarSomAcerto();
          atual++;
          aguardaSoltar();
        } else {
          tocarSomErro();          
          publicarResultado(tag, pontos);
          estadoAtual = GAME_OVER;
        }
      }
    } else {
      pontos++;
      tocarSomConfirma();
 
      lcdClear();
      lcdPrintStr(0, 0, "Nivel " + String(nivel + 1) + " OK! +1pt");
      int proximoDelay = calcularDelay(nivel + 1);
      lcdPrintStr(0, 1, "Prox.spd:" + String(proximoDelay) + "ms");
      delay(1200);
 
      gameState = 0;
      nivel++;
      atual = 0;
      delay(400);
    }
  }
}
 
// ===================== GAME OVER =====================
void runGameOver() {
  int leds[]  = {out_vermelho, out_verde, out_azul, out_amarelo};
  int freqs[] = {170, 164, 146, 130};
 
  lcdClear();
  lcdPrint(1, 0, "FIM DE JOGO!");
  lcdPrintStr(0, 1, "TAG: " + String(tag) + " Pts:" + String(pontos));
 
  for (int i = 0; i < 4; i++) {
    digitalWrite(leds[i], HIGH);
    tone(buzzer, freqs[i], 250);
    delay(260);
    digitalWrite(leds[i], LOW);
    noTone(buzzer);
    delay(80);
  }
  digitalWrite(out_amarelo, HIGH);
  tone(buzzer, 110, 1000);
  delay(1000);
  digitalWrite(out_amarelo, LOW);
  noTone(buzzer);
  delay(1500);
 
  lcdClear();
  lcdPrintStr(0, 0, "Pontos: " + String(pontos));
  lcdPrintStr(0, 1, "Nivel: " + String(nivel + 1));
  delay(3000);
 
  // Reset completo
  nivel = 0; atual = 0; gameState = 0; pontos = 0;
  memset(cores, 0, sizeof(cores));
  estadoAtual = WELCOME;
}
 
// ===================== SETUP & LOOP =====================
void setup() {
  Serial.begin(115200);
 
  setup_wifi();
 
  client.setServer(mqtt_server, 1883);
 
  pinMode(in_vermelho, INPUT_PULLUP);
  pinMode(in_verde,    INPUT_PULLUP);
  pinMode(in_azul,     INPUT_PULLUP);
  pinMode(in_amarelo,  INPUT_PULLUP);
 
  pinMode(out_vermelho, OUTPUT);
  pinMode(out_verde,    OUTPUT);
  pinMode(out_azul,     OUTPUT);
  pinMode(out_amarelo,  OUTPUT);
  pinMode(buzzer,       OUTPUT);
 
  lcd.init();
  lcd.backlight();
 
  randomSeed(analogRead(0));
  estadoAtual = WELCOME;
}
 
void loop() {
  switch (estadoAtual) {
    case WELCOME:      runWelcome();      break;
    case INSTRUCTIONS: runInstructions(); break;
    case TAG_SELECT:   runTagSelect();    break;
    case STARTUP:      runStartup();      break;
    case GAME:         runGame();         break;
    case GAME_OVER:    runGameOver();     break;
  }
  delay(10);
}