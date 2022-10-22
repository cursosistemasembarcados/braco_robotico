#include <WiFi.h>
#include <Wire.h>
#include <SPIFFS.h> // SPIFFS é a biblioteca responsável por controlar a memória do esp
#include <ESPAsyncWebServer.h> // Para Servidor
#include <WebSocketsServer.h> // Para webSocket
#include <stdio.h>
#include <string.h>

const char ssid[] = "ESP32-AP"; // nome da rede
const char senha[] =  "SenhaSimples"; // senha
char bufRecebido[255];
char msg[161];
char valor_msg[4];
char *valor;
char env[5];
unsigned short int passo;
unsigned short int max_passo;
unsigned short int modo = 1; // variável para decidir qual modo será enviado ao site
int short passos[10][4];
int short x[4];
int short u = 0;
bool pr = false;
bool rn = false;
bool rs = false;
unsigned int long tempo;
unsigned int long tempoBuf;
unsigned int tmp = 0;

AsyncWebServer server(80); // 80 é a porta http 
WebSocketsServer webSocket = WebSocketsServer(1337); // 1337 será usado para websocket

void onWebSocketEvent(uint8_t client_num, WStype_t type, uint8_t * payload, size_t length);
void onIndexRequest(AsyncWebServerRequest *request);
void onCSSRequest(AsyncWebServerRequest *request);
void onJSRequest(AsyncWebServerRequest *request);
void onProgHTML(AsyncWebServerRequest *request);
void onProgCSS(AsyncWebServerRequest *request);
void onProgJS(AsyncWebServerRequest *request);
void transmitirPayload(uint8_t * payload);
void conversaoPayload(uint8_t * payload);
void lerDados();
void salvarDados();
void deuErrado();
int cIntToStr(long lngNum, char strNum[], uint8_t tamNum);
long potencia(long a, long b);

void setup() {
  Serial.begin(9600);
  //Serial.printf("\nPrograma comecando\n"); 

  for (int i = 0; i < 10; i++){
    for (int j = 0; j < 4; j++){
      passos[i][j] = -1;
    }
  }
  
  if(!SPIFFS.begin(true)) deuErrado();
  //Serial.println("SPIFFS montado");
  lerDados();
  
  WiFi.softAP(ssid, senha); // configuração wifi
  //Serial.printf("\n\nMeu endereco IP: ");
  //Serial.print(WiFi.softAPIP());
  
  Wire.begin();
  Wire.beginTransmission(4);
  Wire.write("Begin ESP32");
  Wire.endTransmission();
  
  server.on("/", HTTP_GET, onIndexRequest); // atribui a função para devolver o html assim que o navegador pede no servidor, mesma coisa aos de baixo
  server.on("/style.css", HTTP_GET, onCSSRequest);
  server.on("/script.js", HTTP_GET, onJSRequest);
  
  server.on("/prog.html", HTTP_GET, onProgHTML);
  server.on("/prog.css", HTTP_GET, onProgCSS);  
  server.on("/prog.js", HTTP_GET, onProgJS);
  
  server.begin(); // inicia o servidor
  webSocket.begin(); // inicia o webSocket
  webSocket.onEvent(onWebSocketEvent); // Atribui a função de tratamento de dados do websocket à variável WebSocket
  
  tempo = millis();
  tempoBuf = millis();

  /*byte error, address;
  int nDevices;
  Serial.println();
  Serial.println("Scanneando...");
 
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
 
    if (error == 0) {
      Serial.print("I2C encontrado no endereço: ");
      if (address<16)
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");
 
      nDevices++;
    } else if (error==4) {
      Serial.print("Erro desconhecido no endereço: ");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("Nenhum endereço i2c foi encontrado.\n");
  else
    Serial.println("OK\n"); */

  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  delay(200);
  digitalWrite(2, LOW);
  delay(200);
  digitalWrite(2, HIGH);
  delay(200);
  digitalWrite(2, LOW);
  delay(200);
}

void loop() {
  if (millis() - tempo >= 10){
    tempo = millis();
    if (modo) {
      digitalWrite(2, HIGH);
      u = passo;
      if (x[0] != passos[passo][0]){
        if (x[0] < passos[passo][0]) x[0] += 1; 
        else if (x[0] > passos[passo][0]) x[0] -= 1;
      }
       
      if (x[1] != passos[passo][1]){
        if (x[1] < passos[passo][1]) x[1] += 1;  
        else if (x[1] > passos[passo][1]) x[1] -= 1;
      }
      
      if (x[2] != passos[passo][2]){
        if (x[2] < passos[passo][2]) x[2] += 1;
        else if (x[2] > passos[passo][2]) x[2] -= 1;
      }

      if (x[3] != passos[passo][3]){
        if (x[3] < passos[passo][3]) x[3] += 1;          
        else if (x[3] > passos[passo][3]) x[3] -= 1;
      }
      //Serial.println("valores: ");
      //for (int i = 0; i < 4; i++){
      //  Serial.print("\t");
      //  Serial.print(x[i]);
      //} 
    } else {
      digitalWrite(2, LOW);
      if (x[0] != passos[u][0] || x[1] != passos[u][1] || x[2] != passos[u][2] || x[3] != passos[u][3]) {
        if (x[0] < passos[u][0]) x[0] += 1;     
        else if (x[0] > passos[u][0]) x[0] -= 1;
      
        if (x[1] < passos[u][1]) x[1] += 1;        
        else if (x[1] > passos[u][1]) x[1] -= 1;
                    
        if (x[2] < passos[u][2]) x[2] += 1;
        else if (x[2] > passos[u][2]) x[2] -= 1;    
      
        if (x[3] < passos[u][3]) x[3] += 1;       
        else if (x[3] > passos[u][3]) x[3] -= 1;

        /*Serial.println();
        Serial.println("Run: ");
        for (int i = 0; i < 4; i++){
          Serial.print("\t");
          Serial.print(x[i]);
        } */
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

  if (millis() - tempoBuf >= 50) {
    tempoBuf = millis();
    char i = 0;
    char bufEnvio[] = "00000000000000000000000000000000000";
    
    char eixo0[] = "000";
    cIntToStr(passos[u][0], eixo0, 3);
    char eixo1[] = "000";
    cIntToStr(passos[u][1], eixo1, 3);
    char eixo2[] = "000";
    cIntToStr(passos[u][2], eixo2, 3);
    char eixo3[] = "000";
    cIntToStr(passos[u][3], eixo3, 3);
    char isProgram[] = "00";
    cIntToStr(pr, isProgram, 1);
    char isRun[] = "00";
    cIntToStr(rn, isRun, 1);
    char isRs[] = "00";
    cIntToStr(rs, isRs, 1);
    char Cmax_passo[] = "00";
    cIntToStr(max_passo, Cmax_passo, 1);
    char Cpasso[] = "00";
    cIntToStr(u, Cpasso, 1);

    strcpy(bufEnvio, eixo0); //1
    strcat(bufEnvio, eixo1); //4
    strcat(bufEnvio, eixo2); //7
    strcat(bufEnvio, eixo3); //10
    strcat(bufEnvio, isProgram); // 13
    strcat(bufEnvio, isRun); // 15
    strcat(bufEnvio, isRs); // 17
    strcat(bufEnvio, Cmax_passo); // 19
    strcat(bufEnvio, Cpasso); // 21
    //Serial.println(bufEnvio);
    
    if (Serial.available()){
        i = 0;
        bufRecebido[i] = -1;
        while (Serial.available()){
            bufRecebido[i] = Serial.read();
            i++;
            if (i == 20) break;
        }        
        Serial.print(bufEnvio);
        Serial.write(0x0D);
    }
     }

  if (millis() - tmp >= 2000){
    pr = false;
    rn = false;
    rs = false;
  }
  webSocket.loop(); // sem esse comando, o websocket não funcionará
}

void onWebSocketEvent(uint8_t client_num, WStype_t type, uint8_t * payload, size_t length) { 
  switch(type) { // nesse caso, é decidido o que acontece quando qualquer coisa no websocket acontece
    case WStype_DISCONNECTED: // caso cliente disconecte
        /*Serial.println();
        Serial.print("Desconexão de: ");
        Serial.println((client_num));*/
      break;
    case WStype_CONNECTED: // caso cliente conecte
      {
        //Serial.println();
        //Serial.print("Conexao de: ");
        //Serial.println((client_num));
        if (modo){                        // envia pro site o modo que o esp está funcionando no momento
          webSocket.sendTXT(client_num, "pr");  
        } else {
          webSocket.sendTXT(client_num, "run");  
        }
        sprintf(env, "p: %d", passo);
        webSocket.sendTXT(client_num, env);
        sprintf(env, "m: %d", max_passo);
        webSocket.sendTXT(client_num, env);
        sprintf(env, "1: %d", passos[passo][0]);
        webSocket.sendTXT(client_num, env);
        sprintf(env, "2: %d", passos[passo][1]);
        webSocket.sendTXT(client_num, env);
        sprintf(env, "3: %d", passos[passo][2]);
        webSocket.sendTXT(client_num, env);
        sprintf(env, "4: %d", passos[passo][3]);
        webSocket.sendTXT(client_num, env);
      }
      break;
    case WStype_TEXT: // caso seja recebida uma string, onde "payload = dado recebido"
      //Serial.println();
      //Serial.printf("%u: enviou: %s\n", client_num, payload);
      transmitirPayload(payload);
      if (strcmp((char *)payload, "pr") == 0){ // comparação para ver se o payload é igual program
        webSocket.sendTXT(client_num, "pr");
        modo = 1;
        pr = true;
        tmp = millis();
      } else if (strcmp((char *)payload, "run") == 0) { // comparação para ver se o payload é igual run
        webSocket.sendTXT(client_num, "run");
        modo = 0;
        u = 0;
        rn = true;
        tmp = millis();
      } else if (strcmp((char *)payload, "rs") == 0) {
        passos[passo][0] = 90;
        passos[passo][1] = 65;
        passos[passo][2] = 75;
        passos[passo][3] = 105;
        rs = true;
        tmp = millis();
      } else if (strcmp((char *)payload, "sv") == 0) {
        //Serial.println("salvando arquivos");
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
      
      if (payload[0] == 59){
        valor = (char *)payload;
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
        sprintf(env, "1: %d", passos[passo][0]);
        webSocket.sendTXT(client_num, env);
        sprintf(env, "2: %d", passos[passo][1]);
        webSocket.sendTXT(client_num, env);
        sprintf(env, "3: %d", passos[passo][2]);
        webSocket.sendTXT(client_num, env);
        sprintf(env, "4: %d", passos[passo][3]);
        webSocket.sendTXT(client_num, env);
      } else if (payload[1] == 59){
        if (payload[0] == 49){
          conversaoPayload(payload);
          passos[passo][0] = atoi(valor);
        } else if (payload[0] == 50){
          conversaoPayload(payload);
          passos[passo][1] = atoi(valor);
        } else if (payload[0] == 51){
          conversaoPayload(payload);
          passos[passo][2] = atoi(valor);
        } else if (payload[0] == 52){
          conversaoPayload(payload);
          passos[passo][3] = atoi(valor);
        }
      }

      //Serial.println(); 
      //Serial.println("\nMatriz: ");
      for (int i = 0; i < 4; i++){
//        Serial.print("\t");
        //Serial.print(passos[passo][i]);
      }
      break;
    default:
      break;
  }
}

void onIndexRequest(AsyncWebServerRequest *request) { // responsável por devolver o index.html para quando o navegador envia o header http
  IPAddress remote_ip = request->client()->remoteIP();
  request->send(SPIFFS, "/index.html", "text/html");
}

void onCSSRequest(AsyncWebServerRequest *request) { // responsável por devolver o css, quando o index.html pede
  IPAddress remote_ip = request->client()->remoteIP();
  request->send(SPIFFS, "/style.css", "text/css");
}

void onJSRequest(AsyncWebServerRequest *request) {  // responsável por devolver  o js, quando o index.html pede
  IPAddress remote_ip = request->client()->remoteIP();
  request->send(SPIFFS, "/script.js", "text/js");
}

void onProgHTML(AsyncWebServerRequest *request){
  IPAddress remote_ip = request->client()->remoteIP();
  request->send(SPIFFS, "/prog.html", "text/html");
}

void onProgCSS(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  request->send(SPIFFS, "/prog.css", "text/css");
}

void onProgJS(AsyncWebServerRequest *request) {
  IPAddress remote_ip = request->client()->remoteIP();
  request->send(SPIFFS, "/prog.js", "text/js");
}

void transmitirPayload(uint8_t * payload){
  valor = (char *)payload;
  Wire.beginTransmission(4);
  Wire.write(valor);
  Wire.endTransmission();
}

void conversaoPayload(uint8_t * payload){
  valor = (char *)payload;
  for (int i = 0; i < strlen(valor); i++){
    if (i < strlen(valor)-1) valor[i] = valor[i+2];
    else   valor[i] = '\0';
  }
}

void lerDados(){
  File file2 = SPIFFS.open("/dados.txt");
 
  if(!file2) deuErrado();

  int i = 0;
  while(file2.available()){
    msg[i] = (file2.read());
    i++;
  }
  
  msg[i] = '\0';
  file2.close();
  
  //Serial.println("valor lido: ");
  //for (int i = 0; i < strlen(msg); i++){
    //Serial.print(msg[i]);
  //}
  //Serial.println(); 
  int index = 0;
  int linha = 0;
  int coluna = 0;
      
  for (int i = 0; i < strlen(msg); i++){
    if (msg[i] == ':' || msg[i] == ';'){
      valor_msg[index] = '\0';
      passos[linha][coluna] = atoi(valor_msg);
      coluna++;
      index = 0;
      if(msg[i] == ';'){
        coluna = 0;
        linha++;
      }
    } else {
      valor_msg[index] = msg[i];
      index++;
    }
  }
  for (int i = 0; i < 10; i++){
   //Serial.println();
    for (int j = 0; j < 4; j++){
      if (passos[i][j] != -1) max_passo = i;       
      //Serial.print(passos[i][j]);
      //Serial.print("\t");
    }
  } 
}

void salvarDados(){
  File file = SPIFFS.open("/dados.txt", FILE_WRITE);

  if(!file) deuErrado();

  for (int i = 0; i < 10; i++){
    for (int j = 0; j < 4; j++){
      if(!file.print(passos[i][j])) deuErrado();
      if(j == 3) {
        if (!file.print(";")) deuErrado();
      }
      else if (!file.print(":")) deuErrado();
    }
  }
  file.close();
}

void deuErrado(){
  //Serial.println("falhou");
  while(1){
    digitalWrite(2, HIGH);
    delay(200);
    digitalWrite(2, LOW);
    delay(200);
  }
}

int cIntToStr(long lngNum, char strNum[], uint8_t tamNum) {
  long i = 0;
  long lngResto = 0, lngResultado = 0, lngDividendo = potencia(10, (tamNum - 1));

  while (lngDividendo > 0) {
    lngResultado = lngNum / lngDividendo;
    lngResto     = lngNum % lngDividendo;
    lngNum       = lngResto;

    strNum[i] = 48 + lngResultado;
    lngDividendo /= 10;
    i++;
  }
  return 0;
}

long potencia(long a, long b) {
  long resultado = a;
  for (uint8_t i = 1; i < b; i++)
      resultado *= a;
  return resultado;
}
