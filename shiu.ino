//Testar os níveis
#include <EEPROM.h>
#define LIMITE         2          // Define o limite de erro do sinal do potenciometro
#define 
#define DEBUG          1          // Ativar(1) ou desativar(0) a comunicação com o serial.
#define ZERAR          1          // (1) zero o EEPROM (0) mantem o EEPROM com leituras anteriores
#define DELAY          1000        // Define o tempo para o delay de debug em milissegundos
#define NUM_SENSORES   2          // Numero de sensores usados
#define NUM_INTERACOES 500        // Numero de interções no filtro
#define OVERFLOW       4000000000 // Over flow para o unsigned long
#define OCIO           5          // Tempo minimo entre uma ativação e outra
#define LUZ            6          // Sinalizador luminoso ligado à porta digital do arduino 
#define SOM            7          // Sirene ligada à porta digital do arduino
#define COOLER         10         // Define a porta do cooler
#define TEMPO_COOLER   3          // Tempo que o cooler permanecerá ligado 
#define NIVEL_AVISO    1500        // Determina nível de ruído/pulsos para ativar o sinalizador luminoso.
#define NIVEL_LIMITE   1500        // Determina nível de ruído/pulsos para ativar a sirene.
#define TEMPO_LUZ      3          // Define o tempo de duração em que o sinalizador permanecerá ativo.
#define TEMPO_SOM      3          // Define o tempo de duração em que a sirene permanecerá ativo.
#define REP_SOM        2          // Quantidade de vezes que a sirene irá disparar 
#define REP_LUZ        2          // Quantidade de vezes que o sinalizador luminoso irá disparar
#define ON             1
#define OFF            0
#define TOLERANCIA     1          // EM SEGUNDOS PELO AMOR DE DEUS!

int           sensores[NUM_SENSORES] = {A0, A6}; // Sensores ligados às portas analógicas
int           verificadores[NUM_SENSORES] = {A1, A5};  // Resposansaveis por gravar saida do potenciometro
int           nivel                  = 0;    // Variável responsável pelo nível de ruído
int           endereco               = 0;    // Endereço de memória que vai armazenar quantidade de vezes que a sirene acionou
int           Q_Acionamento          = 0;    // Variável responsável por armazenar quantidade de vezes que a sirene acionou  
unsigned long t0                     = 0;    // Variável responsável pelo tempo inicial
unsigned long tc                     = 0;    // Variável responsável pelo tempo calculado  
int           deveAlertar            = 1;    // Variável atua como um binário (true/false)
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
  if(ZERAR)
    EEPROM.write(endereco, 0);    
  else
    EEPROM.write(endereco, EEPROM.read(endereco));
}

void loop() 
{
  nivel = ouvirNivel();
 // lerTempoCooler();
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
    if(DEBUG)
      Serial.println("Em Ocio");
    delay(OCIO*1000);  
  }
  else if(deveAlertar && nivel >= NIVEL_LIMITE) //se deveAlertar for verdadeiro e nivel >= NIVEL_LIMITE aviso luminoso e sonoro
  {
    luz();
    som();
    if(DEBUG)
      Serial.println("Em Ocio");
    delay(OCIO*1000);
    Q_Acionamento++;
    EEPROM.write(endereco, Q_Acionamento);  
  }
  if(DEBUG)
  {
    Serial.print("Numero de vezes que acionou: ");
    Serial.println(EEPROM.read(endereco));
    delay(DELAY);
  }
}

int ouvirNivel() 
{
  int value = 0;
  int leitura_sensor = 0;
  int leitura_verificador = 0;
  for(int i = 0; i < NUM_SENSORES; i++) 
  {
    leitura_sensor = read_sensor(sensores[i]);
    leitura_verificador = read_sensor(verificadores[i]);
    if(DEBUG)
    {
      imprime_valor(i,leitura_sensor);
      imprime_verificador(i, leitura_verificador);
    }
    if(leitura_sensor > value) 
      value = leitura_sensor;
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

void imprime_verificador(int y, int leitura)
{
  Serial.print("Verficador ");
  Serial.print(y+1);
  Serial.print(" : ");
  Serial.println(leitura);
}
