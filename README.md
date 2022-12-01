- [Braço robótico](#braço-robótico)
- [Hardware e Esquemas Elétricos](#hardware-e-esquemas-elétricos)
  - [ATMega328P ou Arduino](#atmega328p-ou-arduino)
  - [ESP32](#esp32)
- [Software](#software)
  - [Software do ATMega328P ou Arduino:](#software-do-atmega328p-ou-arduino)
  - [Software do ESP32:](#software-do-esp32)
  - [Aplicação Web para configuração do Robô](#aplicação-web-para-configuração-do-robô)
- [Sistema Supervisório](#sistema-supervisório)

# Braço Robótico
#### Desenvolvimento de um braço robótico para pegar objetos em ponto A e levar a ponto B

Esse braço robótico, desenvolvido pela 2° turma do curso de Sistemas Embarcados da Fatec de Jundiaí, foi apresentado como um desafio à classe, sendo uma continuação do projeto da sala anterior, uma esteira seletora.

O dispositivo em questão foi adquirido no mercado e compõe-se basicamente de uma estrutura em acrílico para a parte mecânica e uma outra eletromecânica relativa aos quatro servomotores (base, altura, avanço e garra). O seu único objetivo é ilustrar, de forma bastante prática, o funcionamento de um equipamento em escala industrial, com nível de complexidade muito próximo no tocante à sua funcionalidade.

<img src="/imagens/Imagem_do_braco.png" alt="Braço Robótico" style="height:auto; width:100%;"/>

Para o desenvolvimento do braço, foram necessários dois microcontroladores, o NodeMCU ESP32 e o ATMega328P (microcontrolador do Arduino UNO) utilizando o prototocolo de comunicação I²C entre eles, utilizando a linguagem C++ para os microcontroladores, e HTML, JavaScript e CSS para interface de controle do robô. 
___
- *Obs: Esse repositório contém duas branchs. A branch **_main_** (branch atual) contém o código para o funcionamento do braço robótico sozinho. A segunda branch **_serial_esteira_** contém o código e as adaptações necessárias para funcionar com uma segunda parte do nosso projeto, uma Esteira Seletora. Para melhor entendimento do projeto, sugerimos que leia primeiramente a descrição dessa branch, e depois ler a outra.* 
[_Clique aqui para ser abrir a branch Serial Esteira_](https://github.com/cursosistemasembarcados/braco_robotico/tree/serial_esteira)
___

Segue um vídeo do nosso projeto para melhor entendimento:

<img src = "imagens/braco_robotico.gif" width = "100%">

[Clique aqui se preferir ver no youtube](https://www.youtube.com/watch?v=XmaUbG7Ou1w)

# Hardware e Esquemas Elétricos

Como mencionado anteriormente, o projeto usa quatro servomotores. O modelo utilizado foi o MICRO SERVO TOWERPRO 9G SG90 que possui 3 fios, sendo eles para alimentação (fio vermelho), GND (fio marrom) e para PWM (fio laranja). Pode ser alimentado com tensões entre 3,0 à 7,2V. No nosso projeto, utilizamos uma fonte externa, juntamente ao ATMega328P, que envia os sinais PWM para operar os motores.

#### ATMega328P ou Arduino:

<img src="/esquemas_eletricos/pinout_ATMega328P.png" alt="PinoutATMega328p" style="height:auto; width:100%;"/>

O microcontrolador ATMega328P é o microcontrolador presente no Arduino UNO. Porém, no nosso projeto, utilizamos somente o ATMega. Mas, o arduino também pode ser utilizado no lugar dele.

O positivo da fonte deve ser conectado aos cabos vermelho dos servos e o negativo aos cabos marrons dos servos e ao negativo do ATMega328P.

No ATMega328p/Arduino:
- No pino 11(porta digital 6 do arduino): o PWM do servo da garra;
- No pino 15(porta digital 9 do arduino): o PWM do servo da base;
- No pino 16(porta digital 10 do arduino): o PWM do servo da ângulo do braço;
- No pino 17(porta digital 11 do arduino): o PWM do servo da avanço do braço;
- No pino 8 ou 22(qualquer GND do Arduino): jumper com o negativo da fonte externa, e no negativo do ESP32;
- No pino 28(porta analógica A5 do arduino): o SCL do I²C;
- No pino 27(porta analógica A4 do arduino): o SDA do I²C.

No nosso projeto, o esquema elétrico é o seguinte:
<img src="/esquemas_eletricos/Placa_de_controle_robo_ATMega328P.jpeg" alt="PinoutATMega328p" style="height:auto; width:100%;"/>

#### ESP32:

<img src="/esquemas_eletricos/ESP32_pinout.jpg" alt="Pinout ATMega328p" style="height:auto; width:100%;"/>

No ESP32, as ligações são feitas da seguinte forma:
- No Pino 22: SCL do I²C;
- No Pino 21: SDA do I²C;
- No GND: conectado ao GND do ATMega328P(pino 8 ou 22).

# Software

#### Software do ATMega328P ou Arduino:

Se o código for utilizado em um arduino, basta acessar a diretório *"/codigos/Arduino"* e fazer o *upload* do arquivo nele. Para ele não é necessário instalar nenhuma biblioteca externa ao Arduino.

Senão, será necessário instalar o *breadboard-avr* atráves da seguinte página, e seguir os passos que estão nela para fazer o microcontrolador funcionar separadamente:
- [BreadBoard-avr](https://docs.arduino.cc/built-in-examples/arduino-isp/ArduinoToBreadboard)

#### Software do ESP32:

Para o código funcionar corretamente, são necessárias três bibliotecas que não são instaladas ao configurar o *board manager* do ESP32.

Para o programa funcionar corretamente, instale o *board manager* do ESP32 e as bibliotecas que estão abaixo:
- [*esp32-arduino*](https://github.com/iotechbugs/esp32-arduino)
- [*AsyncTCP*](https://github.com/me-no-dev/AsyncTCP)
- [*ESPAsyncWebServer*](https://github.com/me-no-dev/ESPAsyncWebServer)
- *WebSocketsServer* (essa é instalável pelo próprio gerenciador de bibliotecas do Arduino, e pode ser encontrada com o seguinte nome: **WebSockets** by **Markus Sattler**)

Para o processo de configuração inicial do ESP32, é necessário um plugin(esp32-fs), que permite que se suba arquivos à memória flash do ESP32, que nesse projeto são necessários para o funcionamento da interface *Web*.
- [*esp32-fs*](https://github.com/me-no-dev/arduino-esp32fs-plugin)

Para salvar os arquivos da página de login na memória do ESP, é necessário *clicar* no botão *ESP32 Sketch Data Upload*, em "ferramentas", na *interface* do Arduino. Feito isso, para se conectar ao servidor web criado pelo ESP, basta se conectar ao seu WIFI, que podem ter nome e senhas alterados no próprio código(nas linhas 9 e 10). Para se conectar ao servidor, basta escrever *192.168.4.1* no seu navegador de escolha e assim a seguinte página deve se carregar:

<img src="/imagens/Tela_de_login.png" alt="Tela de login" style="height:auto; width:100%;"/>

___
- *Observação: o navegador, às vezes, tentará se conectar ao website usando HTTPS, e não funcionará. Neste caso, mude a url para forçar a conexão usando HTTP.*
___

Nesse projeto, apesar de implementarmos a página de login, ela não foi configurada para ser acessada com determinada senha e login. Sendo assim, basta preencher os campos com qualquer coisa que o site deixará vocẽ prosseguir à tela que está no item abaixo.

#### Aplicação *Web* para configuração do Robô

A fim de se aproximar de uma aplicação real da indústria, além da facilitar a configuração do braço robótico, foi desenvolvido uma aplicação Web, se utilizando das linguagens como HTML, CSS e JavaScript para a criação de um site para configuração dos movimentos que o dispositivo irá realizar para transporte da peça do ponto A ao B. 

Dessa maneira o operador responsável pelo manuseio do braço poderá escolher os ângulos de movimentação da base, avanço, altura e garra do braço, para os respectivos passos que o dispositivo irá fazer em cada posição, tendo a função de escolha do número de passos máximos que serão configurados. Uma vez configurado os movimentos, o braço pode entrar em modo automático. 

Vale ressaltar que para que haja a configuração efetiva entre o operador para o braço, o microcontrolador ESP32 é o responsável pela comunicação entre a aplicação *web* e os dados que serão passados para o braço robótico, por apresentar *wifi* integrado, pode criar um *Acess Point* e lidar com com conexões de uma forma relativamente parecida com o que um servidor faria.

<img src="/imagens/Tela_de_configuracao.png" alt="Tela de configuração do braço" style="height:auto; width:100%;"/>

___
- *Observação: É recomendado que na primeira vez que se iniciar o site, clicar em "reset" e em "save", para que as posições sejam salvas e evite problemas em uso futuro*
___

Acima o "estado da conexão" e o "modo de funcionamento", indicam respectivamente o estado do *websocket*(que é necessário para comunicação com o ESP32) e o modo de funcionamento atual, que pode ser "Run" e "Program".

Abaixo, são possíveis configurar os passos.

Ao final da tela, os botões no final da tela desempenham as seguintes funções:
- *Run*: coloca o robo em modo automático, onde ele executará os passos configurados pelo operador anteriormente;
- *Program*: coloca o robo em modo de configuração;
- *Save*: Salva as posições do robô, fazendo com que mesmo se o robô perca completamente a alimentação, ele lembre do que foi salvo;
- *Reset*: Volta os valores do passo atual para os valores centrais;
- *Restart*: Volta todos os valores de todos os passos aos valores centrais.

# Sistema Supervisório

Todas as funções que estiverem ocorrendo no braço robótico, idependente do modo de operação, serão posssíveis de viazualizar a partir de um sistema supervisório, que é responsável pelo monitoramento em tempo real. A transmissão de dados ocorre pelo protocolo de comunicação RS232, através da porta serial do ESP32. O Software usado no nosso caso é o Elipse E3.
___
- *Observação: a variável "bool b"(linha 24), no código do ESP32 tem que ser "false" para os dados serem transmitidos ao Elipse E3.*

- *Além disso, a variável "P1/N1/B1", do RS232(Domínio/Objetos de Servidor/Drivers e OPC/RS232) deve ser alterada à porta "COM" que o esp está usando no seu computador.*
___

Depois de tudo isso, a tela deve se aparecer com algo parecido abaixo:
<img src="/imagens/Tela_supervisorio_robo.png" alt="Tela de configuração do braço" style="height:auto; width:100%;"/>