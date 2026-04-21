const int in_vermelho = 35;
const int in_verde = 25;
const int in_azul = 33;
const int in_amarelo = 32;
const int out_vermelho = 5;
const int out_verde = 4;
const int out_azul = 16;
const int out_amarelo = 17;

const int buzzer = 19;

typedef enum ESTADOS {
  STARTUP,
  GAME,
  GAME_OVER
}estados;

estados estadoAtual;

void runStartup(){
  Serial.println("BEM VINDO, ESTADO STARTUP");
  digitalWrite(out_vermelho, HIGH);
  tone(buzzer, 170, 250);
  delay(200);
  digitalWrite(out_verde, HIGH);
  tone(buzzer, 164, 250);
  delay(200);
  digitalWrite(out_azul, HIGH);
  tone(buzzer, 485, 250);
  delay(200);
  digitalWrite(out_amarelo, HIGH);
  tone(buzzer, 500, 250);
  delay(200);
  noTone(buzzer);

  digitalWrite(out_vermelho, LOW);
  digitalWrite(out_verde, LOW);
  digitalWrite(out_azul, LOW);
  digitalWrite(out_amarelo, LOW);
  estadoAtual = GAME;
}

int gameState = 0;
int cores[10] = {0,0,0,0,0,0,0,0,0,0};
int nivel = 0;
int atual = 0;

void mostraCor(int cor) {
  Serial.printf("Mostrando cor: %d\n", cor);
  
  if(cor == 1) {
    Serial.println("-> VERMELHO");
    digitalWrite(out_vermelho, HIGH);
    tone(buzzer, 170, 400);
    delay(400);
    digitalWrite(out_vermelho, LOW);
    noTone(buzzer);
  }
  else if(cor == 2) {
    Serial.println("-> VERDE");
    digitalWrite(out_verde, HIGH);
    tone(buzzer, 164, 400);
    delay(400);
    digitalWrite(out_verde, LOW);
    noTone(buzzer);
  }
  else if(cor == 3) {
    Serial.println("-> AZUL");
    digitalWrite(out_azul, HIGH);
    tone(buzzer, 485, 400);
    delay(400);
    digitalWrite(out_azul, LOW);
    noTone(buzzer);
  }
  else if(cor == 4) {
    Serial.println("-> AMARELO");
    digitalWrite(out_amarelo, HIGH);
    tone(buzzer, 500, 400);
    delay(400);
    digitalWrite(out_amarelo, LOW);
    noTone(buzzer);
  }
}

// Função para tocar som quando um botão é pressionado
void tocarSomBotao(int cor) {
  switch(cor) {
    case 1: // Vermelho
      tone(buzzer, 170, 150);
      break;
    case 2: // Verde
      tone(buzzer, 164, 150);
      break;
    case 3: // Azul
      tone(buzzer, 485, 150);
      break;
    case 4: // Amarelo
      tone(buzzer, 500, 150);
      break;
  }
}

// Função para tocar som de erro
void tocarSomErro() {
  tone(buzzer, 100, 300);
  delay(150);
  tone(buzzer, 80, 300);
}

// Função para tocar som de acerto
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
  
  // Só processa se passou o tempo de debounce
  if ((millis() - lastDebounceTime) > debounceDelay) {
    
    // Detecção de borda de descida (HIGH -> LOW) - botão foi pressionado
    if (lastV == HIGH && v == LOW) {
      lastDebounceTime = millis();
      lastV = v;
      Serial.println("leu vermelho");
      tocarSomBotao(1); // Toca som do botão vermelho
      return 1;
    }
    else if (lastG == HIGH && g == LOW) {
      lastDebounceTime = millis();
      lastG = g;
      Serial.println("leu verde");
      tocarSomBotao(2); // Toca som do botão verde
      return 2;
    }
    else if (lastB == HIGH && b == LOW) {
      lastDebounceTime = millis();
      lastB = b;
      Serial.println("leu azul");
      tocarSomBotao(3); // Toca som do botão azul
      return 3;
    }
    else if (lastA == HIGH && a == LOW) {
      lastDebounceTime = millis();
      lastA = a;
      Serial.println("leu amarelo");
      tocarSomBotao(4); // Toca som do botão amarelo
      return 4;
    }
  }
  
  // Atualiza últimos estados
  lastV = v;
  lastG = g;
  lastB = b;
  lastA = a;
  
  return 0;
}

void runGame(){
  Serial.printf("runGame - gameState=%d, nivel=%d, atual=%d\n", gameState, nivel, atual);
  
  if (gameState == 0){ // Mostro pro jogador
    Serial.println("=== INICIANDO SEQUÊNCIA ===");
    Serial.printf("NÍVEL ATUAL: %d\n", nivel);
    
    cores[nivel] = random(1,5);
    Serial.printf("Nova cor adicionada na posição %d: %d\n", nivel, cores[nivel]);
    
    Serial.println("Sequência completa:");
    for (int i = 0; i <= nivel; i++) {
      Serial.printf("  Posição %d: Cor %d\n", i, cores[i]);
    }
    
    // Mostra TODAS as cores da sequência
    for (int i = 0; i <= nivel; i++) {
      Serial.printf("Exibindo cor %d da sequência (valor=%d)\n", i, cores[i]);
      mostraCor(cores[i]);
      delay(300);
    }
    
    gameState = 1;
    Serial.println("=== FIM DA SEQUÊNCIA, AGUARDANDO JOGADOR ===");
    
    // Aguarda os botões serem soltos antes de começar
    delay(500);
    while(leInput() != 0) {
      delay(50); // Limpa qualquer leitura pendente
    }
  }
  else { // Aguarda input
    if (atual <= nivel) {
      int cor = leInput();
      if (cor != 0) {
        Serial.printf("LEU COR: %d (esperava %d)\n", cor, cores[atual]); 
        if(cores[atual] == cor) {
          tocarSomAcerto(); // Toca som de acerto
          Serial.println("ACERTOU!");
          atual++;
          
          // Aguarda o botão ser solto antes de continuar
          delay(200);
          while(leInput() != 0) {
            delay(50);
          }
        }
        else {
          tocarSomErro(); // Toca som de erro
          Serial.println("ERROU! Game Over!");
          estadoAtual = GAME_OVER;
        }
      }
    }
    else {
      Serial.println("Nível completo! Avançando...");
      gameState = 0;
      nivel++;
      atual = 0;
      delay(500);
    }
  }
}

void runGameOver(){
  Serial.println("=== GAME OVER ===");
  
  digitalWrite(out_vermelho, HIGH);
  tone(buzzer, 170, 250);
  delay(250);
  digitalWrite(out_vermelho, LOW);
  noTone(buzzer);
  delay(100);

  digitalWrite(out_verde, HIGH);
  tone(buzzer, 164, 250);
  delay(250);
  digitalWrite(out_verde, LOW);
  noTone(buzzer);
  delay(100);

  digitalWrite(out_azul, HIGH);
  tone(buzzer, 146, 250);
  delay(250);
  digitalWrite(out_azul, LOW);
  noTone(buzzer);
  delay(100);

  digitalWrite(out_amarelo, HIGH);
  tone(buzzer, 130, 1000);
  delay(1000);
  digitalWrite(out_amarelo, LOW);
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
  
  // INPUT_PULLUP: botão conecta ao GND
  pinMode(in_vermelho, INPUT_PULLUP);
  pinMode(in_verde, INPUT_PULLUP);
  pinMode(in_azul, INPUT_PULLUP);
  pinMode(in_amarelo, INPUT_PULLUP);  

  pinMode(out_vermelho, OUTPUT);
  pinMode(out_verde, OUTPUT);
  pinMode(out_azul, OUTPUT);
  pinMode(out_amarelo, OUTPUT);

  pinMode(buzzer, OUTPUT);
  
  randomSeed(analogRead(0));
}

void loop() {
  switch (estadoAtual){
    case STARTUP:
      runStartup();
      break;
    case GAME:
      runGame();
      break;
    case GAME_OVER:
      runGameOver();
      break;
  }
  delay(10);
}
