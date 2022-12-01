/*BIBLIOTECAS*/
#include <Servo.h>
#include <Wire.h>
#include "Adafruit_TCS34725.h"

/*DECLARAÇÃO DE VARIÁVEIS*/
const int sensPE = 4;                 // Sensor de peça na esteira ENVIAR SS
const int sensPR = 12;                 // Sensor de peça rejeitada ENVIAR SS
const int sensPA = 11;                 // Sensor de peça aceita ENVIAR SS
const int ME = 3;                      // Controle Motor Esteira
const int selectP = 10;                // Controle do Servo Motor Seletor de Peças
const int rejeitado = 135;             // Constante do servo para peças rejeitadas
const int aceito = 45;                 // Constante do servo para peças aceitas
const int amb_light = 6;
volatile short estado = 0;                      // Variável de controle de entrada nos estados
volatile short PA = 0;
volatile short PR = 0;
volatile short PE = 0;

int peca_aceita = 0;                   // Variável de contagem de peças aceitas ENVIAR SS
int peca_rejeitada = 0;                // Variável de contagem de peças rejeitadas ENVIAR SS 
uint16_t r, g, b, c;                   // Variáveis para uso com o sensor de cor
int confirmar_pecas= 0;                // Variável para a confirmar o total de peças ENVIAR SS
int total_pecas;                         // Variável para contagem do total de peças ENVIAR SS
char env[31];
bool peca = false;

/*TCS34725 - SENSOR COR*/
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_120MS, TCS34725_GAIN_4X);

/*SERVO MOTORES*/
Servo SELETOR;                         // Cria objeto para controlar um servo

void setup() {// Inicio Setup
  // put your setup code here, to run once:
  Serial.begin(9600);                  // Habilita a porta serial
  Wire.begin(); // join I2C bus (address optional for master)
  
  SELETOR.attach(selectP);             // Conecta o servo no objeto SELETOR
  SELETOR.write(85);                   // Posiciona o servo no centro (HOME)

  attachInterrupt(digitalPinToInterrupt(2), isr_treat, RISING); // Configura a interrupção no D2 com acionamento na borda de subida
 // noInterrupts();
  
  pinMode(ME, OUTPUT);
  pinMode(amb_light, OUTPUT);
  //pinMode(selectP, OUTPUT);
  //pinMode(2, INPUT);
  pinMode(sensPA, INPUT);
  pinMode(sensPR, INPUT);
  pinMode(sensPE, INPUT);

  if (!tcs.begin()) {
    while (1);
  }
}// Fim Setup

void loop() {
  /*Inicio Máquina de estados*/
  switch (estado) {
  /*------------------------------------ESTADO 0-----------------------------------------*/
    case 0: // Stand-By  - Aguarda entrada de peça
    strcpy(env, "SB");
    Serial.write(env);
    PA = 0;
    PR = 0;
    PE = 0;
    //Serial.println("ESTADO 0 - Stand-By");
    while(PE == 0){
      delay(10);
    };
    PE = 0;
    estado = 1;
    delay(100);
    break;
  /*------------------------------------ESTADO 1-----------------------------------------*/
    case 1: //  Inicia Processo
      //Serial.println("ESTADO 1 - INICIA PROCESSO");
      analogWrite(amb_light, 100);     // Liga a luz ambiente
      analogWrite(ME, 40);             // Liga a esteira
      strcpy(env, "SE");
      Serial.write(env);
      estado = 2;
      delay(100);
    break;
  /*------------------------------------ESTADO 2-----------------------------------------*/
    case 2: //  Identifica Peças
      //Serial.println("ESTADO 2 - IDENTIFICA PEÇAS");
      do {
        tcs.getRawData(&r, &g, &b, &c);
      }while ((g < 100) && (b < 100));
      analogWrite(ME, 0);             // Desliga a esteira
  
      for (int i = 0; i<= 5; i++) {
        tcs.getRawData(&r, &g, &b, &c);
      }

      if(g > b){
        SELETOR.write(aceito);         // Posiciona o servo
        total_pecas++;
      }else if(b > g){
        SELETOR.write(rejeitado);      // Posiciona o servo
        total_pecas++;
      }

//      Serial.print("GREEN: "); Serial.println(g);
//      Serial.print("BLUE: "); Serial.println(b);

      estado = 3;
      delay(100);
     
    break;
  /*------------------------------------ESTADO 3-----------------------------------------*/
    case 3: //  Conta Peças (Aceitas/Rejeitadas)
      strcpy(env, "SC");
      Serial.write(env);
//      Serial.println("ESTADO 3 - CONTA PEÇAS");
      analogWrite(ME, 40);             // Liga a esteira
      while((PA == 0) && (PR == 0));

      if(PA == 1){
        peca_aceita++;
      }else if(PR == 1){
         peca_rejeitada++;
      }

      PA = 0;
      PR = 0;
      PE = 0;
      confirmar_pecas = peca_aceita + peca_rejeitada;
      
//      if (total_pecas == confirmar_pecas){
//        Serial.print("Peças totais: "); Serial.println(confirmar_pecas);
//      }
//      else {
//        Serial.print("Erro peças extraviadas!!");
//      }
      
      
//      Serial.print("Peças Aceitas: "); Serial.println(peca_aceita);
//      Serial.print("Peças Rejeitadas: "); Serial.println(peca_rejeitada);
      analogWrite(ME, 0);             // Deliga a esteira
      delay(100);
      estado = 4;
    break;
  /*------------------------------------ESTADO 4-----------------------------------------*/
    case 4: //  Aciona Braço
//      Serial.println("ESTADO 4 - ACIONA BRAÇO");
      sprintf(env, "%d;%d;%d;%d;%d;%d;%d;", peca_aceita, peca_rejeitada, confirmar_pecas, total_pecas, r, g, b);
      Serial.write(env);
      SELETOR.write(90);
      estado = 0;
    break;
  }
  /*FiM Máquina de estados*/

}

/*INTERRUPÇÃO*/
void isr_treat()  {
 
  if(digitalRead(sensPE) == 1){
    //Serial.println("Interrupção Sensor Entrada");
    while(digitalRead(sensPE));
    PE = 1;
  }else if(digitalRead(sensPA) == 1){
    //Serial.println("Interrupção Sensor Peça Aceita");
    while(digitalRead(sensPA));
    PA = 1;
  }else if(digitalRead(sensPR) == 1){
    //Serial.println("Interrupção Sensor Peça Rejeitada");
    while(digitalRead(sensPR));
    PR = 1;
  }
}
