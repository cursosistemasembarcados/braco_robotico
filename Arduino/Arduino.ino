#include <Wire.h> 
#include <Servo.h>
#include <EEPROM.h>

char payload[8];
char *valor;
int modo = 1; // onde 0 é run e 1 é program
int passos[10][4];  // [linhas][colunas]
int passo = 0;
int max_passo = 0;
int u = 0;
int x[4];
unsigned long int tempo;
Servo servo; // base
Servo servo1; // mão
Servo servo2; // ângulo braço
Servo servo3; // Avanço braço

void conversaoPayload();
void salvarDados();
void lerDados();

void setup(){
  Wire.begin(4);
  Wire.onReceive(receiveEvent);
  //Serial.begin(9600);
  //Serial.println();
  //Serial.println("Programa Começando");

  for (int i = 0; i < 10; i++){
    for (int j = 0; j < 4; j++){
      passos[i][j] = -1;
    }
  }

  lerDados();
  
  servo.attach(9);
  servo1.attach(6);
  servo2.attach(10);
  servo3.attach(11);
  tempo = millis();
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);
  delay(200);
  digitalWrite(8, LOW);
  delay(200);
  digitalWrite(8, HIGH);
  delay(200);
  digitalWrite(8, LOW);
}

void loop(){
  digitalWrite(8, modo);
  if (millis() - tempo >= 10){
    tempo = millis();
    if (modo) {
      u = 0;
      u = passo;
      if (x[0] != passos[passo][0]){
        servo.write(x[0]);
        if (x[0] < passos[passo][0]) x[0] += 1; 
        else if (x[0] > passos[passo][0]) x[0] -= 1;
      }
       
      if (x[1] != passos[passo][1]){
        servo1.write(x[1]);
        if (x[1] < passos[passo][1]) x[1] += 1;  
        else if (x[1] > passos[passo][1]) x[1] -= 1;
      }
      
      if (x[2] != passos[passo][2]){
        servo2.write(x[2]);
        if (x[2] < passos[passo][2]) x[2] += 1;
        else if (x[2] > passos[passo][2]) x[2] -= 1;
      }

      if (x[3] != passos[passo][3]){
        servo3.write(x[3]);
        if (x[3] < passos[passo][3]) x[3] += 1;          
        else if (x[3] > passos[passo][3]) x[3] -= 1;
      }
    } else {
      if (x[0] != passos[u][0] || x[1] != passos[u][1] || x[2] != passos[u][2] || x[3] != passos[u][3]) {
        if (x[0] < passos[u][0]) x[0] += 1;     
        else if (x[0] > passos[u][0]) x[0] -= 1;
      
        if (x[1] < passos[u][1]) x[1] += 1;        
        else if (x[1] > passos[u][1]) x[1] -= 1;
                    
        if (x[2] < passos[u][2]) x[2] += 1;
        else if (x[2] > passos[u][2]) x[2] -= 1;    
      
        if (x[3] < passos[u][3]) x[3] += 1;       
        else if (x[3] > passos[u][3]) x[3] -= 1;
        
        servo.write(x[0]);
        servo1.write(x[1]);
        servo2.write(x[2]);
        servo3.write(x[3]);
        //Serial.println();
        //Serial.println("valores: ");
        //for (int i = 0; i < 4; i++){
          //Serial.print("\t");
          //Serial.print(x[i]);
        //}
      } else {
        if (u == max_passo) {
          u = 0;
          delay(500);
        } else {
          u++;
          delay(500);
        }
      }
    } 
  }
}

void receiveEvent(int m){  
  byte i = 0;
  while(Wire.available()) {
    payload[i++] = Wire.read();
  }
  
  payload[i] = '\0';

  //Serial.println();
  //Serial.print("Dado recebido: ");
  
  for (int i = 0; i < strlen(payload); i++){
    //Serial.print(payload[i]);
  }

  if (strcmp(payload, "pr") == 0){
    modo = 1;
  } else if (strcmp(payload, "run") == 0){
    modo = 0;
  } else if (strcmp(payload, "rs") ==  0){
    passos[passo][0] = 90;
    passos[passo][1] = 65;
    passos[passo][2] = 75;
    passos[passo][3] = 105;
  } else if (strcmp((char *)payload, "sv") == 0){
    salvarDados();
  } else if (strcmp((char *)payload, "rt") == 0){
    for (int i = 0; i < 10; i++){
      for (int j = 0; j < 4; j++){
        passos[i][j] = -1;
      }
    }
    passos[passo][0] = 90;
    passos[passo][1] = 65;
    passos[passo][2] = 75;
    passos[passo][3] = 105;
  }
  //Serial.print("\nModo: ");
  //Serial.println(modo);

  //Serial.println();
  if (payload[0] == 59){
    valor = (char *)payload;
    max_passo = valor[1] - '0';    valor = (char *)payload;
    max_passo = valor[1] - '0';
    passo = valor[3] - '0';
    //Serial.print("Max_passo: ");
    //Serial.println(max_passo);
    //Serial.print("Passo: ");
    //Serial.println(passo);
    if (passos[passo][0] == -1){
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
  //Serial.println(passo);
  }
  for (int i = 0; i < 4; i++){
    //Serial.print("\t");
    //Serial.print(passos[passo][i]);
  }
}

void conversaoPayload(){
  valor = (char *)payload;
  for (int i = 0; i < strlen(valor); i++){
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
  //Serial.println();
  //Serial.println();
  //Serial.println("Valores: ");

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

  for (int i = 0; i < 10; i++){
    //Serial.println();
    for (int j = 0; j < 4; j++){
      //Serial.print(passos[i][j]);
      //Serial.print("\t");
    }
  }
}

void salvarDados(){
  int z = 0;
  //Serial.println();
  //Serial.println("Salvando dados: ");
  for (int i = 0; i < 10; i++){  
    for (int j = 0; j < 4; j++){
      EEPROM.update(z, passos[i][j]);
      //Serial.print(passos[i][j]);
      //Serial.print("\t");
      z++;
    }
  }
}
