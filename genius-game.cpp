const int in_vermelho  = 35;
const int in_verde     = 25;
const int in_azul      = 33;
const int in_amarelo   = 32;

const int out_vermelho = 5;
const int out_verde    = 4;
const int out_azul     = 16;
const int out_amarelo  = 17;

const int buzzer = 19;

typedef enum {
  STARTUP,
  GAME,
  GAME_OVER
} estados;

estados estadoAtual;

#define MAX_NIVEL 100

int gameState = 0;
int cores[MAX_NIVEL];   
int nivel = 0;
int atual = 0;

const int TEMPO_BASE    = 400;
const int INTERVALO_BASE = 300;

int tempoAcendimento() {
  int t = (int)(TEMPO_BASE * pow(0.9, nivel));
  return max(t, 80);
}

int tempoIntervalo() {
  int t = (int)(INTERVALO_BASE * pow(0.9, nivel));
  return max(t, 60);
}

void runStartup() {
  Serial.println("BEM VINDO, ESTADO STARTUP");
  digitalWrite(out_azul,     HIGH); 
  tone(buzzer, 485, 250); 
  delay(200);
  digitalWrite(out_verde,    HIGH); 
  tone(buzzer, 164, 250); 
  delay(200);
  digitalWrite(out_amarelo,  HIGH); 
  tone(buzzer, 500, 250); 
  delay(200);
  digitalWrite(out_vermelho, HIGH); 
  tone(buzzer, 170, 250); 
  delay(200);
  noTone(buzzer);

  digitalWrite(out_vermelho, LOW);
  digitalWrite(out_verde,    LOW);
  digitalWrite(out_azul,     LOW);
  digitalWrite(out_amarelo,  LOW);

  estadoAtual = GAME;
}

void mostraCor(int cor) {
  int t = tempoAcendimento(); 
  Serial.printf("Mostrando cor: %d por %d ms\n", cor, t);

  int pino, freq;
  switch (cor) {
    case 1: pino = out_vermelho; freq = 170; 
      Serial.println("-> VERMELHO"); break;
    case 2: pino = out_verde;    freq = 164; 
      Serial.println("-> VERDE");    break;
    case 3: pino = out_azul;     freq = 485; 
      Serial.println("-> AZUL");     break;
    case 4: pino = out_amarelo;  freq = 500; 
      Serial.println("-> AMARELO");  break;
    default: return;
  }

  digitalWrite(pino, HIGH);
  tone(buzzer, freq, t);
  delay(t);
  digitalWrite(pino, LOW);
  noTone(buzzer);
}

void tocarSomBotao(int cor) {
  int freqs[] = {0, 170, 164, 485, 500};
  if (cor >= 1 && cor <= 4) tone(buzzer, freqs[cor], 150);
}

void tocarSomErro() {
  tone(buzzer, 100, 300);
  delay(150);
  tone(buzzer, 80, 300);
}

void tocarSomAcerto() {
  tone(buzzer, 600, 100);
  delay(100);
  tone(buzzer, 800, 100);
}

int leInput() {
  static int lastV = HIGH, lastG = HIGH, lastB = HIGH, lastA = HIGH;
  static unsigned long lastDebounceTime = 0;
  const unsigned long debounceDelay = 50;

  int v = digitalRead(in_vermelho);
  int g = digitalRead(in_verde);
  int b = digitalRead(in_azul);
  int a = digitalRead(in_amarelo);

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (lastV == HIGH && v == LOW) { lastDebounceTime = millis(); lastV = v; Serial.println("leu vermelho"); tocarSomBotao(1); return 1; }
    if (lastG == HIGH && g == LOW) { lastDebounceTime = millis(); lastG = g; Serial.println("leu verde");    tocarSomBotao(2); return 2; }
    if (lastB == HIGH && b == LOW) { lastDebounceTime = millis(); lastB = b; Serial.println("leu azul");     tocarSomBotao(3); return 3; }
    if (lastA == HIGH && a == LOW) { lastDebounceTime = millis(); lastA = a; Serial.println("leu amarelo");  tocarSomBotao(4); return 4; }
  }

  lastV = v; lastG = g; lastB = b; lastA = a;
  return 0;
}


void runGame() {
  Serial.printf("runGame - gameState=%d, nivel=%d, atual=%d\n", gameState, nivel, atual);

  if (gameState == 0) { 
    Serial.println("=== INICIANDO SEQUÊNCIA ===");
    Serial.printf("NÍVEL ATUAL: %d | Tempo LED: %d ms | Intervalo: %d ms\n",
                  nivel, tempoAcendimento(), tempoIntervalo());

    cores[nivel] = random(1, 5);
    Serial.printf("Nova cor na posição %d: %d\n", nivel, cores[nivel]);

    for (int i = 0; i <= nivel; i++) {
      Serial.printf("  Exibindo posição %d: cor %d\n", i, cores[i]);
      mostraCor(cores[i]);
      delay(tempoIntervalo()); 
    }

    gameState = 1;
    Serial.println("=== FIM DA SEQUÊNCIA, AGUARDANDO JOGADOR ===");

    delay(500);
    while (leInput() != 0) delay(50); 
  }
  else { 
    if (atual <= nivel) {
      int cor = leInput();
      if (cor != 0) {
        Serial.printf("LEU COR: %d (esperava %d)\n", cor, cores[atual]);
        if (cores[atual] == cor) {
          tocarSomAcerto();
          Serial.println("ACERTOU!");
          atual++;

          delay(200);
          while (leInput() != 0) delay(50);
        } else {
          tocarSomErro();
          Serial.printf("ERROU! Game Over! Pontuação final: %d\n", nivel);
          estadoAtual = GAME_OVER;
        }
      }
    } else {
      Serial.printf("Nível %d completo! Pontuação: %d\n", nivel, nivel + 1);
      gameState = 0;
      nivel++;
      atual = 0;

      if (nivel >= MAX_NIVEL) {
        Serial.println("PARABÉNS! ZEROU O JOGO!");
        estadoAtual = GAME_OVER;
      } else {
        delay(500);
      }
    }
  }
}

void runGameOver() {
  Serial.printf("=== GAME OVER === Pontuação: %d\n", nivel);

  digitalWrite(out_vermelho, HIGH); 
  tone(buzzer, 170, 250); 
  delay(250);
  digitalWrite(out_vermelho, LOW);  
  noTone(buzzer);         delay(100);

  digitalWrite(out_verde,    HIGH); 
  tone(buzzer, 164, 250); 
  delay(250);
  digitalWrite(out_verde,    LOW);  
  noTone(buzzer);         
  delay(100);

  digitalWrite(out_azul,     HIGH); 
  tone(buzzer, 146, 250); 
  delay(250);

  digitalWrite(out_azul,     LOW);  
  noTone(buzzer);         
  delay(100);

  digitalWrite(out_amarelo,  HIGH); 
  tone(buzzer, 130, 1000); 
  delay(1000);

  digitalWrite(out_amarelo,  LOW); 
   noTone(buzzer);

  nivel = 0;
  atual = 0;
  gameState = 0;
  memset(cores, 0, sizeof(cores));

  delay(2000);
  estadoAtual = STARTUP;
}

void setup() {
  estadoAtual = STARTUP;
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");

  pinMode(in_vermelho, INPUT_PULLUP);
  pinMode(in_verde,    INPUT_PULLUP);
  pinMode(in_azul,     INPUT_PULLUP);
  pinMode(in_amarelo,  INPUT_PULLUP);

  pinMode(out_vermelho, OUTPUT);
  pinMode(out_verde,    OUTPUT);
  pinMode(out_azul,     OUTPUT);
  pinMode(out_amarelo,  OUTPUT);

  pinMode(buzzer, OUTPUT);

  randomSeed(analogRead(0));
}

void loop() {
  switch (estadoAtual) {
    case STARTUP:   
      runStartup();  break;
    case GAME:      
      runGame();     break;
    case GAME_OVER: 
      runGameOver(); break;
  }
  delay(10);
}