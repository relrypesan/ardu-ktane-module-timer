/**   Arduino: Nano
  *   created by: Relry Pereira dos Santos
  */
#include <KtaneModule.h>
#include <TM1637Display.h>

#define F_CODE_NAME F("module-display")
#define F_VERSION F("v0.1.0-a1")

#define DISPLAY_DIO 6
#define DISPLAY_CLK 7

KtaneModule module;

TM1637Display display = TM1637Display(DISPLAY_CLK, DISPLAY_DIO);

volatile long timeToExplodeMillis, timeDisplayMillis;
volatile bool newMessage = false;

Status lastStatusModule;
unsigned long previousMillis = 0;
int interval = 1000;

void setup() {
  Serial.begin(38400);

  timeToExplodeMillis = timeDisplayMillis = 12000;

  delay(1000);

  // module.setFuncModuleEnable();
  module.setFuncResetGame(resetGame);
  module.setFuncValidaResetModule(validaModuloReady);
  module.setFuncConfig(configWrite);
  module.setFuncStartGame(startGame);
  // module.setFuncStopGame();

  module.init(F_CODE_NAME, F_VERSION);

  display.clear();
  display.setBrightness(0x0a);
  printDisplayTime(timeDisplayMillis);
}

void loop() {

  // Verifica se houve mensagem nova recebida pelo barramento I2C
  if (newMessage) {
    switch (module.getRegModule()->status) {
      case IN_GAME:
        Serial.println(F("Em jogo"));
        break;
      default:
        Serial.println(F("Status diferente."));
    }

    newMessage = false;
  }

  // verifica se houve mudança no status do modulo
  if (lastStatusModule != module.getRegModule()->status) {
    Serial.println((String)F("STATUS anterior: ") + Status_name[lastStatusModule]);
    Serial.println((String)F("STATUS atual...: ") + Status_name[module.getRegModule()->status]);

    switch (module.getRegModule()->status) {
      case IN_GAME:
        Serial.println(F("Iniciando o jogo e o contador."));
        break;
      case RESETING:
        timeDisplayMillis = timeToExplodeMillis;
        printDisplayTime(timeToExplodeMillis);
        break;
    }

    lastStatusModule = module.getRegModule()->status;
  }

  switch (module.getRegModule()->status) {
    case IN_GAME:
      executeInGame();
      break;
      // default:
      //   Serial.println("module.getRegModule()->status: " + Status_name[module.getRegModule()->status]);
  }
}

void executeInGame() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    timeDisplayMillis -= interval;
    if (timeDisplayMillis <= 0) {
      timeDisplayMillis = 0;
      module.stopGame();
    }
    printDisplayTime(timeDisplayMillis);
    if (timeDisplayMillis <= 60000) {
      interval = 100;
    }
    previousMillis = currentMillis;
  }
}

void printDisplayTime(long millis) {
  byte dot = 0b00100000;
  long timeTmp = millis / 100;
  if (millis >= 60000) {
    int min = millis / 60000;
    int seg = (millis - (min * 60000)) / 1000;
    timeTmp = min * 100 + seg;
    dot = 0b01000000;
  }
  display.showNumberDecEx(timeTmp, dot);
}

void resetGame() {
  timeDisplayMillis = timeToExplodeMillis;
  printDisplayTime(timeToExplodeMillis);
}

bool validaModuloReady() {
  Serial.println(F("Validando se modulo esta pronto"));
  delay(500);
  if (timeDisplayMillis == timeToExplodeMillis) {
    Serial.println(F("Modulo pronto para iniciar."));
    return true;
  }
  Serial.println(F("Modulo NAO esta pronto para iniciar."));
  return false;
  // return true;
}

void startGame() {
  Serial.println(F("startGame chamado."));
  if (!newMessage) {
    newMessage = true;
    Serial.println(F("Nova mensagem"));
  } else {
    Serial.println(F("WARNING!!! Mensagem ignorada"));
  }
}

void configWrite(uint8_t preset) {
  Serial.println(F("Config modulo"));
  if (Wire.available()) {
    Serial.println("preset: " + ((String)(char)preset));
    if ((char)preset == 'c') {
      Serial.println(F("Configurando tempo do display."));
      int numBytes = Wire.available();
      char arrayChars[numBytes + 1];
      for (int i = 0; i < numBytes; i++) {
        arrayChars[i] = Wire.read();
      }
      arrayChars[numBytes] = '\0';
      String message = String(arrayChars);
      long time = message.toInt();
      Serial.println((String)F("Tempo recebido do master: '") + time + (String)F("'"));
      timeToExplodeMillis = time;
    }
  }
  Serial.println(F("Resetando modulo, pos config."));
  resetGame();
}
