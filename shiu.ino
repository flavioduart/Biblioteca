//Testar os níveis

#define DEBUG 1
#define NUM_SENSORES 2
#define LUZ 13
#define SOM1 8
#define SOM2 9
#define NIVEL_AVISO 500
#define NIVEL_LIMITE 500
#define TEMPO 3000
#define ON 1
#define OFF 0
#define TOLERANCIA 2000

int sensores[NUM_SENSORES] = {A0, A1};
int nivel = 0;
unsigned long t0 = 0;
bool deveAlertar = false;

void setup() {
	Serial.begin(9600);
	pinMode(LUZ, OUTPUT);
	pinMode(SOM1, OUTPUT);
	pinMode(SOM2, OUTPUT);

	for(int i = 0; i < NUM_SENSORES; i++) {
		pinMode(sensores[i], INPUT);
	}
}

void loop() {
	nivel = ouvirNivel();
	
	if(nivel < NIVEL_AVISO) {
		t0 = millis();
	}

        deveAlertar = (millis() - t0) > TOLERANCIA;
  
	if(deveAlertar && nivel < NIVEL_LIMITE) {
		luz(ON);
		delay(TEMPO);
		luz(OFF);
	}
	else if(deveAlertar && nivel >= NIVEL_LIMITE) {
		luz(ON);
		som(ON);
		delay(TEMPO);
		luz(OFF);
		som(OFF);
    delay(TEMPO);
	}

if(DEBUG)delay(1000);
}

int ouvirNivel() {
	int value = 0;
	int leitura = 0;
	for(int i = 0; i < NUM_SENSORES; i++) {
		leitura = read_sensor(sensores[i], 100);
		if(leitura > value) value = leitura;
		Serial.print("Sensor ");
		Serial.print(i+1);
		Serial.print(": ");
		Serial.println(leitura);
	}
        Serial.print("Maior Valor: ");
        Serial.println(value);
	return value;
}

int read_sensor(int port, int times){
  unsigned long value = 0;
  for(int x = 0; x < times; x++){
    value += analogRead(port);
    delay(1);
  }
  value /= times;
  return value;
}

void luz(int tipo) {
	if(tipo == ON) {
		digitalWrite(LUZ, HIGH);
    Serial.println("Disparado Luz");
	}
	else if(tipo == OFF) {
		digitalWrite(LUZ, LOW);
    Serial.println("Desligando Luz");
	}
}

void som(int tipo) {
	if(tipo == ON) {
		digitalWrite(SOM1, HIGH);
		digitalWrite(SOM2, HIGH);
    Serial.println("Disparado Som");
	}
	else if(tipo == OFF) {
		digitalWrite(SOM1, LOW);
		digitalWrite(SOM2, LOW);
    Serial.println("Desligando Som");
	}
}
