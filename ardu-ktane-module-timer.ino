/**   Arduino: Nano
  *   created by: Relry Pereira dos Santos
  */
#include "KtaneModule.h"
#include <Wire.h>
#include <TM1637Display.h>

#define F_CODE_NAME   F("module-display")
#define F_VERSION     F("v0.1.0-a1")

#define DISPLAY_DIO 6
#define DISPLAY_CLK 7

KtaneModule module;

TM1637Display display = TM1637Display(DISPLAY_CLK, DISPLAY_DIO);

volatile long timeToExplodeSeg, timeDisplayMillis;
volatile Status currentStatusModule = RESETING;
volatile bool newMessage = false;

Status lastStatusModule = currentStatusModule;
unsigned long previousMillis = 0;
int interval = 1000;

void setup() {
  Serial.begin(38400);

  delay(1000);

  // module.setFuncModuleEnable();
  module.setFuncResetGame(resetGame);
  module.setFuncValidaResetModule(validaModuloReady);
  module.setFuncConfig(configWrite);
  module.setFuncStartGame(startGame);
  // module.setFuncStopGame();

  module.init(F_CODE_NAME, F_VERSION);

  timeDisplayMillis = 120000;

  display.clear();
  display.setBrightness(0x0a);

  startGame();
}

void loop() {

  // Verifica se houve mensagem nova recebida pelo barramento I2C
  if (newMessage) {
    switch (currentStatusModule) {
      case IN_GAME:
        Serial.println(F("Em jogo"));
        break;
      default:
        Serial.println(F("Status diferente."));
    }

    newMessage = false;
  }

  // verifica se houve mudança no status do modulo
  if (lastStatusModule != currentStatusModule) {
    Serial.println((String)F("STATUS anterior: ") + Status_name[lastStatusModule]);
    Serial.println((String)F("STATUS atual...: ") + Status_name[currentStatusModule]);

    switch (currentStatusModule) {
      case IN_GAME:
        Serial.println(F("Iniciando o jogo e o contador."));
        break;        
    }

    lastStatusModule = currentStatusModule;
  }

  switch (currentStatusModule) {
    case IN_GAME:
      executeInGame();
      break;
    // default:
    //   Serial.println("currentStatusModule: " + Status_name[currentStatusModule]);
  }
}

void executeInGame() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    timeDisplayMillis -= interval;
    if (timeDisplayMillis <= 0) {
      timeDisplayMillis = 0;
      currentStatusModule = STOP_GAME;
    }
    long timeTmp = timeDisplayMillis;
    if (timeTmp < 60000) {
      timeTmp *= 10;
    }
    Serial.println("Tempo a ser printado: " + ((String)timeTmp));
    display.showNumberDecEx((timeTmp / 1000), 0b01000000);
    if (timeDisplayMillis <= 60000) {
      interval = 100;
    }
    previousMillis = currentMillis;
  }
}

void printDisplayTime(long millis) {
  if (millis ) {

  }
}

void resetGame() {
  if(module.getRegModule()->status == READY) {
    timeDisplayMillis = timeToExplodeSeg;    
  }
}

bool validaModuloReady() {
  Serial.println("Validando se modulo está pronto");
  delay(500);
  if(timeDisplayMillis == timeToExplodeSeg) {
    Serial.println("Modulo pronto para iniciar.");
    return true;
  }
  Serial.println("Modulo NAO esta pronto para iniciar.");
  return false;
  // return true;
}

void startGame() {
  Serial.println(F("startGame chamado."));
  if (!newMessage) {
    newMessage = true;
    currentStatusModule = IN_GAME;
    Serial.println(F("Nova mensagem"));
  } else {
    Serial.println(F("WARNING!!! Mensagem ignorada"));
  }
}

void configWrite(uint8_t preset) {
  Serial.println(F("Config modulo"));
  if(Wire.available()) {
    if((char)preset == 'c') {
      Serial.println(F("Configurando tempo do display."));
      int numBytes = Wire.available();
      char arrayChars[numBytes+1];
      for(int i = 0; i < numBytes; i++) {
        arrayChars[i] = Wire.read();
      }
      arrayChars[numBytes] = '\0';
      String message = String(arrayChars);
      long time = message.toInt();
      Serial.println((String)F("Tempo recebido do master: '") + time + (String)F("'"));
      timeDisplayMillis = time;
    }
  }
}
