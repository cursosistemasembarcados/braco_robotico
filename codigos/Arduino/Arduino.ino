#include <Wire.h> 
#include <Servo.h>
#include <EEPROM.h>

char payload[8];
char *valor;
int modo = 1; // onde 0 é run e 1 é program
int passos[10][4];
int passo = 0;
int max_passo = 0;
int x[4];
int adrrEsp = 10;
unsigned long int tempo;
int estaDisp = 0;
bool b = true; // variável que decide se algo será printado no monitor serial
Servo servo; // base - Pin(9)
Servo servo1; // garra - Pin(6)
Servo servo2; // ângulo braço - Pin(10)
Servo servo3; // Avanço braço - Pin(11)
Servo servos[4] = {servo, servo1, servo2, servo3};

void conversaoPayload();
void salvarDados();
void lerDados();
void resetarDados();

void setup(){
  Wire.begin(4);
  Wire.onReceive(receiveEvent);
  
  if (b) {
    Serial.begin(9600);
    Serial.println();
    Serial.println("Programa Começando"); 
  }
  
  for (int i = 0; i < 10; i++){
    for (int j = 0; j < 4; j++){
      passos[i][j] = -1;
    }
  }

  lerDados();
  for (int i = 0; i < 4; i++){
    servos[i].write(90);
  }
  servos[0].attach(9);
  servos[1].attach(6);
  servos[2].attach(10);
  servos[3].attach(11);
  tempo = millis();
  pinMode(8, OUTPUT);
  for (int i = 0; i < 2; i++){
    digitalWrite(8, HIGH);
    delay(100);
    digitalWrite(8, LOW);
    delay(100); 
  }
}

void loop(){
  digitalWrite(8, modo);
  if (millis() - tempo >= 10){
    tempo = millis();
    for (int i = 0; i < 4; i++){ // toda essa parte serve para suavização dos movimentos
      if (x[i] < passos[passo][i]) x[i] += 1;
      if (x[i] > passos[passo][i]) x[i] -= 1;
      servos[i].write(x[i]);
    }

    if (b) {
        Serial.println();
        Serial.println("valores: ");
        for (int i = 0; i < 4; i++){
          Serial.print("\t");
          Serial.print(x[i]);
        }
      }
    
    if (modo == 0) {
      if (x[0] == passos[passo][0] && x[1] == passos[passo][1] && x[2] == passos[passo][2] && x[3] == passos[passo][3]) {
        if (passo == max_passo) { // se chegou no último movimento
          estaDisp--;
          passo = 0;
        } else { // se não, passa pro próximo
          passo++;
        }
        delay(500);
      }
    }
  }
}

void receiveEvent(int m){  // tratamento i2c
  byte i = 0;
  while(Wire.available()) {
    payload[i++] = Wire.read();
  }
  payload[i] = '\0';

  if (b) {
    Serial.println();
    Serial.print("Dado recebido: ");
    for (int i = 0; i < strlen(payload); i++){
      Serial.print(payload[i]);
    } 
  }

  if (strcmp(payload, "pr") == 0){
    modo = 1;
  } else if (strcmp(payload, "run") == 0){
    modo = 0;
    passo = 0;
  } else if (strcmp(payload, "rs") ==  0){
    resetarDados();
  } else if (strcmp((char *)payload, "sv") == 0){
    salvarDados();
  } else if (strcmp((char *)payload, "rt") == 0){
    for (int i = 0; i < 10; i++){
      for (int j = 0; j < 4; j++){
        passos[i][j] = -1;
      }
    }
    passo = 0;
    max_passo = 0;
    resetarDados();
  } else if (strcmp((char *)payload, "go") == 0) {
    estaDisp++;
  }
  
  if (b){
    Serial.print("\nModo: ");
    Serial.println(modo);
    Serial.println(); 
  }
  
  if (payload[0] == 59){
    valor = (char *)payload;
    max_passo = valor[1] - '0';
    max_passo = valor[1] - '0';
    passo = valor[3] - '0';
    
    if (b){
      Serial.print("Max_passo: ");
      Serial.println(max_passo);
      Serial.print("Passo: ");
      Serial.println(passo);  
    }
    
    if (passos[passo][0] == -1){ // essa parte serve para caso o valor do próximo passo não tenha sido configurado ainda, ele puxa ou do anterior, ou do passo da frente
      if (passo >= 1 && passos[passo-1][0] != -1){
        for (int i = 0; i < 4; i++){
          passos[passo][i] = passos[passo-1][i];    
        }
      } else if (passo <= 9 && passos[passo+1][0] != -1){
        for (int i = 0; i < 4; i++){
          passos[passo][i] = passos[passo+1][i];    
        }
      } 
    }
    
  } else if (payload[1] == ';'){
     if (payload[0] == '1'){
      conversaoPayload();
      passos[passo][0] = atoi(valor);
     } else if (payload[0] == '2'){
      conversaoPayload();
      passos[passo][1] = atoi(valor);
     } else if (payload[0] == '3'){
      conversaoPayload();
      passos[passo][2] = atoi(valor);
     } else if (payload[0] == '4'){
      conversaoPayload();
      passos[passo][3] = atoi(valor);
     }
    if (b) Serial.println(passo);
  }
  
  if (b){
    for (int i = 0; i < 4; i++){
      Serial.print("\t");
      Serial.print(passos[passo][i]);
    }  
  }
}

void conversaoPayload(){
  valor = (char *)payload;
  for (int i = 0; i < strlen(valor); i++){ // o valor é recebido no formato: "slider;valor", esse "for" isola o parâmetro "valor"
    if (i < strlen(valor)-1) {
      valor[i] = valor[i+2];
    } else {
      valor[i] = '\0';
    }  
  }
}

void lerDados(){
  int linha = 0;
  int coluna = 0;
  int value;
  int z = 0;

  if (b) {
    Serial.println();
    Serial.println("Valores: "); 
  }

  for (int i = 0; i < 10; i++){  
    for (int j = 0; j < 4; j++){
      value = EEPROM.read(z);
      if (value == 255){
        passos[i][j] = -1;
      } else {
        passos[i][j] = value;
      }
      z++;
    }
  }
  
  if (b){
    for (int i = 0; i < 10; i++){
      Serial.println();
      for (int j = 0; j < 4; j++){
        Serial.print(passos[i][j]);
        Serial.print("\t");
      }
    }  
  }
}

void salvarDados(){
  int z = 0;
  if (b){
    Serial.println();
    Serial.println("Salvando dados: ");  
  }
  
  for (int i = 0; i < 10; i++){  
    for (int j = 0; j < 4; j++){
      EEPROM.update(z, passos[i][j]);
      if (b){
        Serial.print(passos[i][j]);
        Serial.print("\t"); 
      }
      z++;
    }
  }
}

void resetarDados(){
  passos[passo][0] = 90;
  passos[passo][1] = 75;
  passos[passo][2] = 85;
  passos[passo][3] = 70;
}
