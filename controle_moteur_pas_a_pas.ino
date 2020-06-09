#include <LiquidCrystal_I2C.h> // https://github.com/johnrickman/LiquidCrystal_I2C
#include <SimpleRotary.h> // https://github.com/mprograms/SimpleRotary
#include <Thread.h> //https://github.com/ivanseidel/ArduinoThread

#define DEBUG 0 // define utilise pour debugger le programme

// Gestion du moteur pas a pas
#define stepsByRevolution 200 //variable dédiée à la gestion du moteur pas à pas. A adapter selon le moteur
#define stepPin 6
#define vitesseMin 0 // la vitesse minimale ne peut pas être inferieur a 0
#define vitesseMax 200
#define increment 10

volatile int VitesseCourante = 0;

Thread motorThread = Thread();
void motorCallback()
{
	if (VitesseCourante > 31) // https://www.arduino.cc/reference/en/language/functions/advanced-io/tone/
		tone(stepPin, VitesseCourante, 5);
	else
		noTone(stepPin);
}

// Gestion de l'ecran LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
Thread lcdThread = Thread();

void printVitesse()
{
	char msg[21];
	sprintf(msg, "%-5d rpm", VitesseCourante);
	lcd.setCursor(0, 1);
	lcd.print(msg);
}

// Encodeur rotatif
SimpleRotary rotary(3, 2, 8);
Thread rotaryThread = Thread();

void rotaryCallback()
{
	byte rotation;

	if (rotary.push()) { // on arrete le moteur si il y a un appui long ou cours
		VitesseCourante = 0;
#if DEBUG
		Serial.println("arret moteur");
#endif
	}

	rotation = rotary.rotate();
	if (rotation == 1) { // decrementation
		if ((VitesseCourante - increment) > vitesseMin) {
			VitesseCourante -= increment;
		} else {
			VitesseCourante = 0;
		}
#if DEBUG
		Serial.println("Vitesse Courante --");
		Serial.println(VitesseCourante);
#endif
	} else if (rotation == 2) { // incrementation
		if ((VitesseCourante + increment) < vitesseMax) {
			VitesseCourante += increment;
		} else {
			VitesseCourante = vitesseMax;
		}
#if DEBUG
		Serial.println("Vitesse Courante ++");
		Serial.println(VitesseCourante);
#endif
	}
}

void setup()
{
  	// Initialisation des E/S
  	// Gestion de l'encodeur rotatif
	pinMode(2, INPUT_PULLUP);
	pinMode(3, INPUT_PULLUP);
	pinMode(8, INPUT_PULLUP);

	rotaryThread.onRun(rotaryCallback);
	rotaryThread.setInterval(1);

	// Gestion du moteur pas a pas
	pinMode(stepPin, OUTPUT);
	motorThread.onRun(motorCallback);
	motorThread.setInterval(5);

	// Gestion de l'écran LCD
	lcd.init();
	lcd.backlight();
	lcd.setCursor(0, 0); // on se place sur la première ligne, première colonne
	lcd.print("Vitesse moteur :");
	lcdThread.onRun(printVitesse);
	lcdThread.setInterval(100);

#if DEBUG
  	Serial.begin(9600);
#endif
}

void loop()
{
	if (lcdThread.shouldRun())
		lcdThread.run();

	if (rotaryThread.shouldRun())
		rotaryThread.run();

	if (motorThread.shouldRun())
		motorThread.run();
}
