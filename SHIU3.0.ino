#include <EEPROM.h>
#include <LiquidCrystal.h>

#define LIMITE           6          // Define o limite de erro do sinal do potenciometro
#define DEBUG            1          // Ativar(1) ou desativar(0) a comunicação com o serial.
#define ZERAR            0          // (1) zera o EEPROM (0) mantem o EEPROM com leituras anteriores
#define DELAY            500        // Define o tempo para o delay de debug em milissegundos
#define DELAY_BOTAO      200        // Define o tempo de espera para o delay do erro humano em relação aos botões
#define DELAY_AVISO      1000       // Define o tempo de espera para o usuario ler uma menssagem de aviso no display
#define DELAY_DISPLAY    80         // Define o tempo de espera para o delay do display evitando que a tela fique piscando
#define DELAY_INICIAL    2000       // Define o tempo para o delay quando o sistema é ligado na energia
#define TAMANHO_VETOR    30         // Aproximadamente 10 interações por segundo
#define NUM_SENSOR       1          // Numero de sensores usados
#define NUM_INTERACAO    700        // Numero de interções no filtro linear
#define NUM_BOTAO        4          // Numero de botões ativos
#define NUM_DADO         4          // Define o numero do slot de dados da memoria EEPROM
#define NUM_REPETICAO    2          // Quantidade de vezes que a sirene irá disparar  
#define OVERFLOW         4000000000 // Over flow para o unsigned long 
#define SIRENE           6          // Sinalizador luminoso ligado à porta digital do arduino
#define NIVEL_LIMITE     180        // Determina nível de ruído/pulsos para ativar a sirene.
#define TEMPO_SIRENE     3          // Define o tempo de duração em que o sinalizador permanecerá ativo. 
#define ON               1
#define OFF              0
#define slot_Acionamento 0

short  slot_limite[NUM_DADO]           = {}; // Responsável  por guardar memória no EEPROM
short  botao[NUM_BOTAO]                 = {8, 6, 7, 9}; // Portas dos botoes
short  sensor_porta[NUM_SENSOR]         = {A1};         // Sensores ligados às portas analógicas
short  sensor_sinal[NUM_SENSOR]         = {};            // Responsáveis por gravar saida do sensor
short  potenciometro_porta[NUM_SENSOR]  = {A7};         // Responsáveis por gravar saida do potenciometro
short  potenciometro_sinal[NUM_SENSOR]  = {};           // Potenciometros ligados às portas analógicas
int   valor                             = 0;
short  limite_potenciometro[NUM_SENSOR] = {EEPROM.get(slot_limite[0], valor)/*, EEPROM.get(slot_limite[1], valor), EEPROM.get(slot_limite[2], valor),EEPROM.get(slot_limite[3], valor)*/};  // Variável responsável por definir o limiar do potenciometro medido analogicamente em relação à sensibilidade do sensor

int   vetor[TAMANHO_VETOR]              = {};    // Vetor responsável por guardar os ultimos TAMANHO_VETOR's níveis de ruído
int   media_vetor                       =  0;    // Valor medio do vetor de valores
int   endereco                          =  0;    // Endereço de memória que vai armazenar quantidade de vezes que a sirene acionou
int   Q_Acionamentos                     =  0;    // Responsável por armazenar quantidade de vezes que a sirene acionou
int   potenciometro_ideal[NUM_SENSOR]   = {60};    // Valor ideal do potenciometro
int   sensibilidade_ideal[NUM_SENSOR]   = {540};   // Valor ideal da sensibilidade

char  flag_potenciometro[NUM_SENSOR]    = {};    // Guarda caracteres 'S' ou 'N' para a calibração dos potenciometros 
int   flag_sensor[NUM_SENSOR]           = {};    // Guarda caracteres '1' ou '0' para a calibração dos sensores


LiquidCrystal lcd(5, 4, 3, 2, 1, 0);   //LiquidCrystal lcd(<pino RS>, <pino enable>, <pino D4>, <pino D5>, <pino D6>, <pino D7>)

void(* reset) (void) = 0;  //Função responsável por reiniciar a programação pelo código.

void setup()
{
  delay(DELAY_INICIAL);
  zerar_EEPROM();
  if (DEBUG) 
  Serial.begin(9600);
  lcd.begin(16, 2); 
  
  pinMode(SIRENE, OUTPUT);
  
  for (int i = 0; i < NUM_SENSOR; i++)
    pinMode(sensor_porta[i], INPUT);
    
  for (int i = 0; i < NUM_BOTAO; i++)
    pinMode(botao[i], INPUT);
  
  lcd.clear();
}

void loop()
{
  ler_sensor();
  distribuir_vetor();
  if( analisar_regulagem() )
    menu_iniciar();
  else
    configuracao_potenciometro();
}

void menu_iniciar()
{
  lcd.clear();
  
  if(analisar_barulho())
  {
    lcd.setCursor(0, 0);
    lcd.print("SIRENE");
    lcd.setCursor(1, 0);
    lcd.print("LIM");
    lcd.print(NIVEL_LIMITE);
    lcd.print("VAL");
    lcd.print(media_vetor);
    delay(DELAY_DISPLAY);
    sirene();
  }
  else
  {
    lcd.setCursor(0, 0);
    for(int i = 0; i < NUM_SENSOR; i++)
    {
       lcd.print(sensor_sinal[i]);
       lcd.setCursor(0, i+4); // 0,4,8,12
    }
    lcd.setCursor(1, 0);
    lcd.print("2-CONFIGURACAO");
    delay(DELAY_DISPLAY);
  }
}

void menu_configuracao()
{}

void configuracao_potenciometro()
{
  int escolha = 0;
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CONFIG POTEN");
  lcd.setCursor(1, 0);
  lcd.print("PRES 1 P/ PASSAR");
  delay(DELAY_AVISO);
  
  while(ler_botao(botao[0]) == false)
  {  
    lcd.setCursor(0, 0);
    lcd.print("ANALISE ATUAL");
    lcd.setCursor(1, 0);
    
    for( int i = 0; i< NUM_SENSOR; i++) 
    {
      lcd.print(flag_potenciometro[i]);
      lcd.print(" | ");
      lcd.setCursor(1, i+4);
    }
    delay(DELAY_DISPLAY);
  }
  
  lcd.setCursor(0, 0);
  lcd.print("QUAL POT EDITAR?");
  lcd.setCursor(1, 0);
  for( int i = 0; i< NUM_SENSOR; i++) 
  {
    lcd.print(i+1);             //Exemplo 1 | 2 | 3 | 4
    lcd.print(" | ");
    lcd.setCursor(1, i+4);
  }
  
  escolha = ler_escolha();
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SUA ESCOLHA FOI");
  lcd.setCursor(1, 0);
  lcd.print(escolha);
  delay(DELAY_AVISO);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("OBJETIVO ");
  lcd.print(limite_potenciometro[NUM_SENSOR]);  
  lcd.setCursor(1, 0);
  lcd.print(escolha);
  delay(DELAY_AVISO); 
  
}

void configuracao_sensibilidade_sensor()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  while(ler_botao(botao[0]) == false)
  {
  lcd.print("CONFIG SENSOR");
  lcd.setCursor(1, 0);
  lcd.print("PRES 1 P/ PASSAR");
  delay(DELAY_AVISO);
  }

  lcd.setCursor(0, 0);
  lcd.print("QUAL SENS MUDAR?");
  lcd.setCursor(1, 0);
  for( int i = 0; i< NUM_SENSOR; i++) 
  {
    lcd.print(i+1);             //Exemplo 1 | 2 | 3 | 4
    lcd.print(" | ");
    lcd.setCursor(1, i+4);
  }
  
  int escolha = ler_escolha();
  bool ajuste = analisar_sensibilidade(escolha);
  
  if(ajuste == false)
  {
    while(ler_botao(botao[0]) == false){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DESCALIBRADO");
    delay(DELAY);
    lcd.clear();
    lcd.print("limite: ");
    lcd.print(limite_potenciometro[escolha]);
    lcd.setCursor(0, 1);
    lcd.print("POT: ");
    lcd.print(analogRead(potenciometro_sinal[escolha]));
    }
    ajuste == true;
  }
  else 
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("CALIBRADO");
    delay(DELAY);
  }
 }


bool analisar_sensibilidade(int x)
{
  if(sensor_sinal[x] > sensibilidade_ideal[x] + LIMITE || sensor_sinal[x] < sensibilidade_ideal[x] + LIMITE)
    {
      flag_sensor[x] = false;
      return false;
    }

    else if(sensor_sinal[x] <= sensibilidade_ideal[x] + LIMITE && sensor_sinal[x] >= sensibilidade_ideal[x] + LIMITE)
    {
      flag_sensor[x] = true;
      return true;
    }
  
  /*
  for(int i = 0; i < NUM_SENSOR; i++)
  {
    if(sinal_sensor[i] > sensibilidade_ideial[i] + LIMITE || sinal_sensor[i] < sensibilidade_ideial[i] + LIMITE)
    {
      flag_sensor[i] = false;
      return false;
    }

    else if(sinal_sensor[i] <= sensibilidade_ideial[i] + LIMITE && sinal_sensor[i] >= sensibilidade_ideial[i] + LIMITE)
    {
      flag_sensor[i] = true;
      return true;
    }
  }*/
}

bool analisar_regulagem()
{
  for(int i = 0; i < NUM_SENSOR; i++)
  {
    if( potenciometro_sinal[i] > potenciometro_ideal[i] + LIMITE || potenciometro_sinal[i] < potenciometro_ideal - LIMITE)
      flag_potenciometro[i] = 'N';
    else if( potenciometro_sinal[i] <= potenciometro_ideal + LIMITE && potenciometro_sinal[i] >= potenciometro_ideal - LIMITE)
      flag_potenciometro[i] = 'S';
  }
  
  for(int i = 0; i < NUM_SENSOR; i++)
  {
    if( flag_potenciometro[i] == 'N' )
      return false;
  }
  return true;
}

void regular_potenciometro(int porta)
{
}

void ler_sensor()
{
  for(int i = 0; i < NUM_SENSOR; i++)
  {
    sensor_sinal[i] = filtro_linear(sensor_porta[i]);
    potenciometro_sinal[i] = analogRead(potenciometro_porta[i]);
  }
}

bool ler_botao(int porta)
{
  if(digitalRead[porta] == HIGH)
  {
    delay(DELAY_BOTAO);
    return true;
  }
  else
  { 
    delay(DELAY_BOTAO);
    return false;
  }
}

int ler_escolha()
{
  while(1)
  {
    for(int i=0; i< NUM_SENSOR; i++)
    {
      if(ler_botao[i])
        return i;
    }
  }
}

void distribuir_vetor()
{
   for(int i = 0; i < TAMANHO_VETOR; i++)
   {
      if(i == 0)
        vetor[i] = media_sala();
      else
        vetor[i] = vetor[i-1]; 
   }
}

bool analisar_barulho()
{
  int soma = 0;
  
  for(int i = 0; i < TAMANHO_VETOR; i++)
    soma += vetor[i];
  
  media_vetor = soma/TAMANHO_VETOR;
  
  if( media_vetor >= NIVEL_LIMITE )
    return true;
  else
    return false;
}

int filtro_linear(int porta) //Lê uma porta e retorna a mesma filtrada
{
  int soma = 0;
  
  for(int i = 0; i< NUM_INTERACAO ; i++)
    soma += analogRead(porta);
    
  return soma/NUM_INTERACAO;
}

int media_sala()
{
  int soma = 0;
  for(int i = 0; i < NUM_SENSOR; i++)
  {
    soma += sensor_sinal[i];
  }
  return soma/NUM_SENSOR;
}

int maximo_sala()
{
  int maior = sensor_sinal[0];
  
  for(int i = 0; i < NUM_SENSOR ; i++)
  {
    if( maior > sensor_sinal[i] )
      maior = sensor_sinal[i];
  }
  return maior;
}

void sirene()
{
  for(int i = 0; i < NUM_REPETICAO ; i++)
  {
    digitalWrite(SIRENE, HIGH);
    delay(DELAY);
    digitalWrite(SIRENE, LOW);
    delay(DELAY);  
  }
}

void zerar_EEPROM() // Função responsável por zerar o EEPROM caso necessário
{
  if (ZERAR)
    EEPROM.write(Q_Acionamentos, 0);
  else
    EEPROM.write(Q_Acionamentos, EEPROM.read(slot_Acionamento));
}

void zerar_vetor()
{
  for(int i = 0; i< NUM_SENSOR; i++)
    vetor[i] = 0;
}
