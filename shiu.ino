//! Código para o dispositivo SHIU
 /*! Projeto que visa diminuir o nível de ruído no ambiente.
 * @author Ruamberg Vasconcelos e Kewin Lima
 * @since 25/05/2016
 * @version 2.0
 */

//Testar os níveis
#define DEBUG        1
#define NUM_SENSORES 2
#define OVERFLOW     4000000000
#define LUZ          13
#define SOM1         8
#define NIVEL_AVISO  500
#define NIVEL_LIMITE 500
#define TEMPO_LUZ    3000
#define TEMPO_SOM    3000
#define REP_SOM      2
#define REP_LUZ      2
#define ON           1
#define OFF          0
#define TOLERANCIA   2000

int sensores[NUM_SENSORES] = {A0, A1};
int nivel = 0;
unsigned long t0 = 0;
bool deveAlertar = false;
void(* reset) (void) = 0;

void setup() 
{
  Serial.begin(9600);
  pinMode(LUZ,  OUTPUT);
  pinMode(SOM1, OUTPUT);

  for(int i = 0; i < NUM_SENSORES; i++) 
  {
    pinMode(sensores[i], INPUT);
  }
}

void loop() 
{
  nivel = ouvirNivel();
	
  if(nivel < NIVEL_AVISO) 
  {
    if(t0 >= OVERFLOW)
    {
      reset();
    }
    else t0 = millis(); // transformar para segundos
  }

  deveAlertar = (millis() - t0) > TOLERANCIA;
  
  if(deveAlertar && nivel < NIVEL_LIMITE) 
  {
    luz();
  }
  else if(deveAlertar && nivel >= NIVEL_LIMITE) 
  {
    luz();
    som();
  }
  if(DEBUG)delay(1000);
}

int ouvirNivel() 
{
  int value = 0;
  int leitura = 0;
  for(int i = 0; i < NUM_SENSORES; i++) 
  {
    leitura = read_sensor(sensores[i], 100);
    if(leitura > value) value = leitura;
    if(DEBUG) imprime_valor(&i, &leitura, &value); // NÂO TESTEI COM OS PONTEIROS 
    return value;
  }
}

int read_sensor(int port, int times)
{
  unsigned long value = 0;
  for(int x = 0; x < times; x++)
  {
    value += analogRead(port);
  }
  value /= times;
  return value;
}

void imprime_valor(int *i, int *leitura, int *value) //TESTAR ESSA FUNÇÃO
{
  Serial.print("Sensor ");
  Serial.print(*i+1);
  Serial.print(": ");
  Serial.println(*leitura);
  Serial.print("Maior Valor: ");
  Serial.println(*value);
}
void luz() 
{
    for(int n=0 ; n< REP_LUZ; n++)
    {
      digitalWrite(LUZ, HIGH);
      if(DEBUG) Serial.println("Disparado Luz");
      delay(TEMPO_LUZ);
      digitalWrite(LUZ, LOW);
      if(DEBUG) Serial.println("Desligando Luz");
    }
}

void som() 
{
    for(int n=0; n < REP_SOM ; n++ )
    {
      digitalWrite(SOM1, HIGH);
      if(DEBUG) Serial.println("Disparado Som");
      delay(TEMPO_SOM);
      digitalWrite(SOM1, LOW);
      if(DEBUG) Serial.println("Desligando Som");
    }
}
