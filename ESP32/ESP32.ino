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
bool pr = false;
bool rn = false;
bool rs = false;
bool b = true; // se for true, o monitor serial será habilitado
unsigned int long tempoBuf;
unsigned int tmp;
unsigned int tempo;

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
void enviarDados();
void resetarPasso();
int cIntToStr(long lngNum, char strNum[], uint8_t tamNum);
long potencia(long a, long b);

void setup() {
  Serial.begin(9600);
  if (b) Serial.printf("\nPrograma comecando\n");  

  for (int i = 0; i < 10; i++){
    for (int j = 0; j < 4; j++){
      passos[i][j] = -1;
    }
  }
  
  if(!SPIFFS.begin(true)) deuErrado();
  if (b) Serial.println("SPIFFS montado");
  lerDados();
  
  WiFi.softAP(ssid, senha); // configuração wifi
  if (b) Serial.printf("\n\nMeu endereco IP: ");
  if (b) Serial.print(WiFi.softAPIP());
  
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

  pinMode(2, OUTPUT);

  for (int i = 0; i < 2; i++){
    digitalWrite(2, HIGH);
    delay(200);
    digitalWrite(2, LOW);
    delay(200); 
  }
  
  tempoBuf = millis();
  tempo = millis();
  tmp = millis();
}

void loop() {
  webSocket.loop(); // sem esse comando, o websocket não funcionará
  digitalWrite(2, modo);

  if (millis() - tempo >= 10){
    tempo = millis();
    for (int i = 0; i < 4; i++){ // toda essa parte serve para suavização dos movimentos
      if (x[i] < passos[passo][i]) x[i] += 1;
      if (x[i] > passos[passo][i]) x[i] -= 1;
    }
    
    if (modo == 0) {
      if (b) {
        Serial.println();
        Serial.println("valores: ");
        for (int i = 0; i < 4; i++){
          Serial.print("\t");
          Serial.print(x[i]);
        }
      }
      
      if (x[0] == passos[passo][0] && x[1] == passos[passo][1] && x[2] == passos[passo][2] && x[3] == passos[passo][3]) {
        if (passo == max_passo) { // se chegou no último movimento
          passo = 0;
        } else { // se não, passa pro próximo
          passo++;
        }
        delay(500);
        enviarDados();
      }
    }
  }
  
  if (millis() - tempoBuf >= 50 && !b) {
    tempoBuf = millis();
    char i = 0;
    char bufEnvio[] = "00000000000000000000000000000000000"; // string de envio para o supervisórios
    
    char eixo0[] = "000";
    cIntToStr(passos[passo][0], eixo0, 3);
    char eixo1[] = "000";
    cIntToStr(passos[passo][1], eixo1, 3);
    char eixo2[] = "000";
    cIntToStr(passos[passo][2], eixo2, 3);
    char eixo3[] = "000";
    cIntToStr(passos[passo][3], eixo3, 3);
    char isProgram[] = "00";
    cIntToStr(pr, isProgram, 1);
    char isRun[] = "00";
    cIntToStr(rn, isRun, 1);
    char isRs[] = "00";
    cIntToStr(rs, isRs, 1);
    char Cmax_passo[] = "00";
    cIntToStr(max_passo, Cmax_passo, 1);
    char Cpasso[] = "00";
    cIntToStr(passo, Cpasso, 1);

    // concatena e envia tudo via porta serial // 
    strcpy(bufEnvio, eixo0); //1
    strcat(bufEnvio, eixo1); //4
    strcat(bufEnvio, eixo2); //7
    strcat(bufEnvio, eixo3); //10
    strcat(bufEnvio, isProgram); // 13
    strcat(bufEnvio, isRun); // 15
    strcat(bufEnvio, isRs); // 17
    strcat(bufEnvio, Cmax_passo); // 19
    strcat(bufEnvio, Cpasso); // 21    
    if (Serial.available()){ // Rotina de envio de dados para Elipse E3
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

  if (millis() - tmp >= 2000){ // resetar variáveis que registra os estados do botões no supervisórios
    pr = false;
    rn = false;
    rs = false;
  }
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
        if (b) Serial.println();
        if (b) Serial.print("Conexao de: ");
        if (b) Serial.println((client_num));
        if (modo){                        // envia pro site o modo que o esp está funcionando no momento
          webSocket.sendTXT(client_num, "pr");  
        } else {
          webSocket.sendTXT(client_num, "run");  
        }
        enviarDados();
      }
      break;
    case WStype_TEXT: // caso seja recebida uma string, onde "payload = dado recebido"
      if (b) Serial.println();
      if (b) Serial.printf("%u: enviou: %s\n", client_num, payload);
      
      transmitirPayload(payload);  // transmite payload para o ATmega32u4
      if (strcmp((char *)payload, "pr") == 0){ // comparação para ver se o payload é igual program
        webSocket.sendTXT(client_num, "pr");
        modo = 1;
        pr = true;
        tmp = millis();
        enviarDados();
      } else if (strcmp((char *)payload, "run") == 0) { // run
        webSocket.sendTXT(client_num, "run"); // envia de volta ao cliente para o browser ter a confirmação
        modo = 0;
        passo = 0;
        rn = true;
        tmp = millis();
      } else if (strcmp((char *)payload, "rs") == 0) { // reset
        resetarDados();
        rs = true;
        tmp = millis();
      } else if (strcmp((char *)payload, "sv") == 0) { // save
        if (b) Serial.println("salvando arquivos");
        salvarDados();
      } else if (strcmp((char *)payload, "rt") == 0){ // restart
        for (int i = 0; i < 10; i++){
          for (int j = 0; j < 4; j++){
            passos[i][j] = -1;              
          }
        }
        passo = 0;
        max_passo = 0;
        resetarDados();
      }
      
      if (payload[0] == 59){ // quando o cliente aperta muda o passo ou o max_passo, é recebido no seguinte formato: ";max_passo;passo", onde ";" == 59 
        valor = (char *)payload;
        max_passo = valor[1] - '0';
        passo = valor[3] - '0';
        if (b) Serial.print("Max_passo: ");
        if (b) Serial.println(max_passo);
        if (b) Serial.print("Passo: ");
        if (b) Serial.println(passo);
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
        sprintf(env, "1: %d", passos[passo][0]); // envia os valores do passo atual para o browser para atualizar na página
        enviarDados();
      } else if (payload[1] == 59){ // quando o cliente mexe em um dos sliders, o formato enviado é: "slider;valor"
        if (payload[0] == 49){ // primeiro slider
          conversaoPayload(payload);
          passos[passo][0] = atoi(valor);
        } else if (payload[0] == 50){ // segundo slider
          conversaoPayload(payload);
          passos[passo][1] = atoi(valor);
        } else if (payload[0] == 51){ // terceiro
          conversaoPayload(payload);
          passos[passo][2] = atoi(valor);
        } else if (payload[0] == 52){ // quarto
          conversaoPayload(payload);
          passos[passo][3] = atoi(valor);
        }
      }

      if (b){
        Serial.println(); 
        Serial.println("\nMatriz: ");
        for (int i = 0; i < 4; i++){
          Serial.print("\t");
          Serial.print(passos[passo][i]);
        } 
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

void onProgHTML(AsyncWebServerRequest *request){ // prog.html
  IPAddress remote_ip = request->client()->remoteIP();
  request->send(SPIFFS, "/prog.html", "text/html");
}

void onProgCSS(AsyncWebServerRequest *request) { // prog.css
  IPAddress remote_ip = request->client()->remoteIP();
  request->send(SPIFFS, "/prog.css", "text/css");
}

void onProgJS(AsyncWebServerRequest *request) { // prog.js
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

  if (b){
    Serial.println("valor lido: ");
    for (int i = 0; i < strlen(msg); i++){
      Serial.print(msg[i]);
    }
    Serial.println();   
  }
  
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
   if (b) Serial.println();
    for (int j = 0; j < 4; j++){
      if (passos[i][j] != -1) max_passo = i;       
      if (b) Serial.print(passos[i][j]);
      if (b) Serial.print("\t");
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
  if (b) Serial.println("falhou");
  while(1){
    digitalWrite(2, HIGH);
    delay(200);
    digitalWrite(2, LOW);
    delay(200);
  }
}

void enviarDados(){ // aqui, assim que o cliente conecta, o esp32 envia o passo, o max_passo, e os valores do sliders no passo atual para o browser do cliente
  sprintf(env, "%d;%d;%d;%d;%d;%d", passo, max_passo, passos[passo][0], passos[passo][1],passos[passo][2], passos[passo][3]); 
  webSocket.sendTXT(0, env);
}

void resetarDados(){
  passos[passo][0] = 90;
  passos[passo][1] = 65;
  passos[passo][2] = 75;
  passos[passo][3] = 105;
}

int cIntToStr(long lngNum, char strNum[], uint8_t tamNum) { // converter int para String, mantendo formato de 3 casas, 90 -> 090
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
