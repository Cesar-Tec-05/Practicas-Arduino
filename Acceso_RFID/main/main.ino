#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SS_PIN 10
#define RST_PIN 9
String UID = "99 05 46 02";
byte lock = 0;
#define BUZZER_PIN 7
Servo servo;

// OLED configuración
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// RFID
MFRC522 rfid(SS_PIN, RST_PIN);

// Función para mover el servo suavemente
void moverServoSuave(int posInicial, int posFinal, int paso=1, int delayMs=15) {
  if (posFinal > posInicial) {
    for (int i = posInicial; i <= posFinal; i += paso) {
      servo.write(i);
      delay(delayMs);
    }
  } else {
    for (int i = posInicial; i >= posFinal; i -= paso) {
      servo.write(i);
      delay(delayMs);
    }
  }
}

void setup() {
  Serial.begin(9600);

  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);

  // Servo
  servo.attach(3);
  servo.write(70); // posición inicial

  // OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 no encontrado"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // RFID
  SPI.begin();
  rfid.PCD_Init();

  Serial.println(F("Sistema listo. Acerca tarjeta..."));
}

void loop() {
  // Mensaje en OLED
  display.clearDisplay();
  display.setCursor(20, 0);
  display.println("Bienvenido!");
  display.setCursor(10, 20);
  display.println("Acerca tu Tarjeta");
  display.display();

  if (!rfid.PICC_IsNewCardPresent())
    return;
  if (!rfid.PICC_ReadCardSerial())
    return;

  // Construir UID leído
  String ID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (i > 0) ID += " ";
    if (rfid.uid.uidByte[i] < 0x10) ID += "0";
    ID += String(rfid.uid.uidByte[i], HEX);
  }
  ID.toUpperCase();

  Serial.print("UID detectado: ");
  Serial.println(ID);

  // Comparación
  if (ID == UID && lock == 0) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Tarjeta Aceptada");
    display.display();
    digitalWrite(BUZZER_PIN, HIGH);
    delay(300);
    digitalWrite(BUZZER_PIN, LOW);
    moverServoSuave(servo.read(), 160); // abrir suavemente
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Puerta Abierta");
    display.display();
    delay(1000);
    lock = 1;
  } 
  else if (ID == UID && lock == 1) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Tarjeta Aceptada");
    display.display();
    digitalWrite(BUZZER_PIN, HIGH);
    delay(300);
    digitalWrite(BUZZER_PIN, LOW);
    moverServoSuave(servo.read(), 70);  // cerrar suavemente
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Puerta Cerrada");
    display.display();
    delay(1000);
    lock = 0;
  } 
  else {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(600);
    digitalWrite(BUZZER_PIN, LOW);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Tarjeta Rechazada");
    display.display();
    delay(1000);
  }

  rfid.PICC_HaltA();  // Detener lectura hasta retirar tarjeta
}
