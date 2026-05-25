#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>
#include <AccelStepper.h>

// ================= LCD =================
LiquidCrystal_I2C lcd(0x3F, 16, 2);

// ================= TERMISTOR =================
#define THERMISTOR_PIN A0

#define SERIES_RESISTOR 100000.0
#define NOMINAL_RESISTANCE 100000.0
#define NOMINAL_TEMPERATURE 25.0
#define BETA 3950.0

// ================= MOTOR =================
#define ENABLE_PIN 10

AccelStepper motor1(1, 7, 4);

int velocidade_motor = 400;
char estadoMotor = '3';

// ================= TEMPO =================
unsigned long tempoTemp = 0;
unsigned long tempoLCD = 0;

const unsigned long intervaloTemp = 1000;
const unsigned long intervaloLCD  = 1500;

float temperatura = 0;

// ================= SETUP =================
void setup()
{
    Serial.begin(9600);

    lcd.init();
    lcd.backlight();

    lcd.setCursor(0,0);
    lcd.print("Iniciando...");
    delay(1000);
    lcd.clear();

    pinMode(ENABLE_PIN, OUTPUT);
    digitalWrite(ENABLE_PIN, HIGH);

    motor1.setMaxSpeed(1000);
}

// ================= LOOP =================
void loop()
{
    // MOTOR SEMPRE PRIMEIRO
    motor1.runSpeed();

    // ===== SERIAL =====
    if (Serial.available())
    {
        char comando = Serial.read();

        if (comando != estadoMotor)
        {
            estadoMotor = comando;

            if (comando == '1')
            {
                digitalWrite(ENABLE_PIN, LOW);
                motor1.setSpeed(velocidade_motor);
            }

            else if (comando == '2')
            {
                digitalWrite(ENABLE_PIN, LOW);
                motor1.setSpeed(-velocidade_motor);
            }

            else if (comando == '3')
            {
                motor1.setSpeed(0);
                digitalWrite(ENABLE_PIN, HIGH);
            }
        }
    }

    unsigned long agora = millis();

    // ================= TEMPERATURA =================
    if (agora - tempoTemp >= intervaloTemp)
    {
        tempoTemp = agora;

        int adcValue = analogRead(THERMISTOR_PIN);

        // chama runSpeed DURANTE processamento pesado
        motor1.runSpeed();

        if (adcValue > 0 && adcValue < 1023)
        {
            float resistance =
                SERIES_RESISTOR / ((1023.0 / adcValue) - 1.0);

            motor1.runSpeed();

            float steinhart = resistance / NOMINAL_RESISTANCE;

            motor1.runSpeed();

            steinhart = log(steinhart);

            motor1.runSpeed();

            steinhart /= BETA;
            steinhart += 1.0 / (NOMINAL_TEMPERATURE + 273.15);
            steinhart = 1.0 / steinhart;
            steinhart -= 273.15;

            temperatura = steinhart;

            Serial.print("Temp: ");
            Serial.println(temperatura);
        }
    }

    // ================= LCD =================
    if (agora - tempoLCD >= intervaloLCD)
    {
        tempoLCD = agora;

        // atualiza uma linha por vez
        lcd.setCursor(0, 0);
        lcd.print("T:");
        lcd.print(temperatura, 1);
        lcd.print((char)223);
        lcd.print("C   ");

        motor1.runSpeed();

        lcd.setCursor(0, 1);

        if (estadoMotor == '1')
            lcd.print("Horario       ");

        else if (estadoMotor == '2')
            lcd.print("Anti-Horario  ");

        else
            lcd.print("Parado        ");
    }
}