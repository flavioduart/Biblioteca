//Testar os níveis
#define DEBUG          1          // Ativar(1) ou desativar(0) a comunicação com o serial.
#define NUM_SENSORES   2          // Numero de sensores usados
#define NUM_INTERACOES 500        // Numero de interções no filtro
#define OVERFLOW       4000000000 // Over flow para o unsigned long
#define OCIO           5          // Tempo minimo entre uma ativação e outra
#define LUZ            6          // Sinalizador luminoso ligado à porta digital do arduino 
#define SOM            7          // Sirene ligada à porta digital do arduino
#define COOLER         10         // Define a porta do cooler
#define TEMPO_COOLER   3          // Tempo que o cooler permanecerá ligado 
#define NIVEL_AVISO    400        // Determina nível de ruído/pulsos para ativar o sinalizador luminoso.
#define NIVEL_LIMITE   400        // Determina nível de ruído/pulsos para ativar a sirene.
#define TEMPO_LUZ      3          // Define o tempo de duração em que o sinalizador permanecerá ativo.
#define TEMPO_SOM      3          // Define o tempo de duração em que a sirene permanecerá ativo.
#define REP_SOM        2          // Quantidade de vezes que a sirene irá disparar 
#define REP_LUZ        2          // Quantidade de vezes que o sinalizador luminoso irá disparar
#define ON             1
#define OFF            0
#define TOLERANCIA     2          // EM SEGUNDOS PELO AMOR DE DEUS!

int           sensores[NUM_SENSORES] = {A0, A6}; // Sensores ligados às portas analógicas
int           nivel                  = 0;
unsigned long t0                     = 0;
unsigned long tc                 = 0;
int           deveAlertar            = 1;
unsigned int  expoente               = 1;
bool          ligarcooler            = false;

void(* reset) (void)= 0;

//Função que define os componentes sinalizador,sirene e sensores como entrada ou saída

void setup() 
{
  if(DEBUG) Serial.begin(9600);
  pinMode(LUZ,  OUTPUT); //@param[OUT]
  pinMode(SOM, OUTPUT); //@param[OUT]

  for(int i = 0; i < NUM_SENSORES; i++) 
  {
    pinMode(sensores[i], INPUT); //@param[IN]
  }
}

void loop() 
{
  nivel = ouvirNivel();
  lerTempoCooler();
  if(nivel < NIVEL_AVISO) 
  {
    if(t0 < OVERFLOW)
    {
      t0 = (millis()/1000); // Transforma para segundo para armazenar mais valores na variável
    }
    else 
    reset();
  }

  deveAlertar = ((millis()/1000) - t0) > TOLERANCIA; // Se a condicao for aceita deveAlertar = 1(true) se a condicao for falsa deveAlertar = 0(false)

  if(deveAlertar && (nivel > NIVEL_AVISO && nivel < NIVEL_LIMITE)) //se deveAlertar for verdadeiro e nivel < NIVEL_LIMITE aviso luminoso
  {
    luz();
  }
  else if(deveAlertar && nivel >= NIVEL_LIMITE) //se deveAlertar for verdadeiro e nivel >= NIVEL_LIMITE aviso luminoso e sonoro
  {
    luz();
    som();
    if(DEBUG)
      Serial.println("Em Ocio");
    delay(OCIO*1000);
  }
  if(DEBUG)delay(1000);
}

int ouvirNivel() 
{
  int value = 0;
  int leitura = 0;
  for(int i = 0; i < NUM_SENSORES; i++) 
  {
    leitura = read_sensor(sensores[i]);
    if(DEBUG)
      imprime_valor(i,leitura);
    if(leitura > value) 
      value = leitura;
  }
  return value;
}

int read_sensor(int port)
{
  unsigned long value = 0;
  for(int x = 0; x < NUM_INTERACOES; x++)//Antes tinha times, uma funcao que passava como argumento. Desnecessario muito mais viavel usar um define N_INTERACOES
  {
    value += map(analogRead(port),0,1023,1023,0); //Inverte os valores começando em 1023 e terminando em 0
                            //Especificamente esse sensor utiliza 1023 como silencio e 0 como barulho máximo.
  }
  value /= NUM_INTERACOES;
  return value;
}
void lerTempoCooler()
{
  if(tc<OVERFLOW)
  {
    if(millis()/1000 - tc > TEMPO_COOLER)
    {
      ligarcooler = expoente%2;    //Fazendo o modulo da divisão por 2, o valor de ligarcooler vai variar de 0 - 1 a cada TEMPO_COOLER
      if(DEBUG)
      {
        Serial.print("ligarcooler = "); 
        Serial.println(ligarcooler);
        Serial.print("Expoente    = ");      
        Serial.println(expoente);
      }
      expoente++;
      tc = millis()/1000;
    }
  }
  else
  {
    reset();
  }
  cooler();
}
void imprime_valor(int x, int leitura) 
{
  Serial.print("Sensor ");
  Serial.print(x+1);
  Serial.print(": ");
  Serial.println(leitura);
}
void luz() 
{
    for(int n=0 ; n< REP_LUZ; n++)
    {
      digitalWrite(LUZ, HIGH);
      if(DEBUG) 
        Serial.println("Disparado Luz");
        delay(TEMPO_LUZ*1000);
        digitalWrite(LUZ, LOW);
      if(DEBUG) 
        Serial.println("Desligando Luz");
    }
}

void som() 
{
    for(int n=0; n < REP_SOM ; n++ )
    {
      digitalWrite(SOM, HIGH);
      if(DEBUG)
        Serial.println("Disparado Som");
    delay(TEMPO_SOM*1000);
        digitalWrite(SOM, LOW);
      if(DEBUG) 
        Serial.println("Desligando Som");
    }
}
void cooler()
{
  if(ligarcooler == 1 && deveAlertar == 0) //Afim de proteger o circuito, apenas ligar o coller quando os outros componentes desligados
  {
    digitalWrite(COOLER, HIGH);
    if(DEBUG) 
      Serial.println("Cooler Ligado");
  }
  else if(ligarcooler == 0)
  {
    digitalWrite(COOLER, LOW);
    if(DEBUG) 
      Serial.println("Cooler desligado");      
  }
}
