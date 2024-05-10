/* Projeto Integrador
 * Univesp
 *  Polos Guarulhos, Juquitiba, Mauá, Santa Isabel - SP
2024

 *  04/2024
 * 
 */
 
 
 
// Modelo da placa==>>  Wifiduino (ESP-8266 Module)
#include <ESP8266WiFi.h>
#include <Ethernet.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
 
ESP8266WebServer server(80);
 
const int NumRedes=3;
int LinhaRede=0;
const char* WIRELESS[NumRedes][2]={
                      {"JM","Coloque a senha do Wifi Aqui!"},
                      {"PNTM","Coloque a senha do Wifi Aqui!"},
                      };
 
int erroWIFI=20;
int erroBD=0;
 
/* Time Stamp */
#include <NTPClient.h>
#include <WiFiUdp.h>
 
#define NTP_OFFSET  -3  * 60 * 60 // In seconds // -3  * 60 * 60 
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "0.pool.ntp.org"
 
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);
 
// ############ Temperatura Agua
 
// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>
 
// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2
 
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
 
// arrays to hold device address
DeviceAddress insideThermometer;
 
float tempAgua=1.1;
int aquecedorPin=13;
int estadoAquecedor=LOW;
float tempMin=27.0;// 27
float tempMax=28.0;
int espera=5000;
int LIGA=LOW;
int DESLIGA=HIGH;
String estadoAquecedorS="Aquec.Des";
int estadoAquecedorBD=LOW;
 
//############################ bOTÃO DE ALIMENTAÇÃO
 
int btAlim=5;
int estadoBtAlim;
int estadoBtAlimAnterior;
int bomba=16;
unsigned long esperaAlim=60000; //1 minuto x 10
int alimenta=LOW;
 
 
//############# Banco de dados
IPAddress server_addr(https://localhost:8080);  // IP of the MySQL *server* here
char userBD[] = "***";              // MySQL user login username
char passwordBD[] = "***";        // MySQL user login password
 
WiFiClient client; 
 
int forcaAtualizacao=HIGH;
MySQL_Connection conn((Client *)&client);
MySQL_Connection conn1((Client *)&client);
//#### UPDATE
const char* host = "vansan.com.br";
 
String atualizaBDS="1";
int contaBusca=0;
 
unsigned long tempoAtualBD=0;
unsigned long tempoAnteriorBD=900000; // 15 minutos
unsigned long tempoAtualizacaoBD=900000; // 15 minutos
 
//########################## LCD i2c
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
 
// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);
 
uint8_t bell[8]  = {0x4, 0xe, 0xe, 0xe, 0x1f, 0x0, 0x4};
uint8_t note[8]  = {0x2, 0x3, 0x2, 0xe, 0x1e, 0xc, 0x0};
uint8_t clock[8] = {0x0, 0xe, 0x15, 0x17, 0x11, 0xe, 0x0};
uint8_t heart[8] = {0x0, 0xa, 0x1f, 0x1f, 0xe, 0x4, 0x0};
uint8_t duck[8]  = {0x0, 0xc, 0x1d, 0xf, 0xf, 0x6, 0x0};
uint8_t check[8] = {0x0, 0x1 ,0x3, 0x16, 0x1c, 0x8, 0x0};
uint8_t cross[8] = {0x0, 0x1b, 0xe, 0x4, 0xe, 0x1b, 0x0};
uint8_t retarrow[8] = {  0x1, 0x1, 0x5, 0x9, 0x1f, 0x8, 0x4};
 
unsigned long tempoAnteriorLCD = 0;   
unsigned long tempoAtualLCD = millis();
const long tempoAtualizacaoLCD = 5000;   
int estadoLCD=0;
 
// ########### Server Web
 
char txthtml[800];
 
void handleRoot() {
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
 
"<html>\
  <head>\
    <meta http-equiv='refresh' content=10/>\
    <title>ESPDuino Aquario</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <h1>Valores dos Sensores</h1>\
    <h3>Aquecedor</h3>\
    %d\
    <h3>Temperatura Agua: </h3>\
    %.1f\
     </body>\
</html>",
 
   hr, min % 60, sec % 60 , estadoAquecedorBD, tempAgua
    );
  server.send ( 800, "text/html", txthtml );; 
}
 
void handleNotFound(){
//  digitalWrite(led, 1);
 
snprintf ( txthtml, 800,
 
"<html>\
  <head>\
    <meta http-equiv='refresh' content=10;url='https://localhost'>\
    <title>ESPDuino Aquario</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Atualizando Sensores</h1>\
  </body>\
</html>"
  );
  server.send ( 800, "text/html", txthtml );; 
 
 Serial.print("uri falha = ");
  Serial.println(server.uri());
  if (server.uri()=="/atualiza"){
    forcaAtualizacao=HIGH;
    Serial.print("Mudando a atualização forçada !! ");
  }
 
}
 
//###########################//###########################//###########################
void setup(){
    Serial.begin(115200);    
    // initialize the LCD
    lcd.init();
   
    // Turn on the blacklight and print a message.
    lcd.backlight();
    lcd.print("Iniciando . . .");
 
  lcd.createChar(0, bell);
  lcd.createChar(1, note);
  lcd.createChar(2, clock);
  lcd.createChar(3, heart);
  lcd.createChar(4, duck);
  lcd.createChar(5, check);
  lcd.createChar(6, cross);
  lcd.createChar(7, retarrow);
  lcd.home();
   
  lcd.setCursor(0, 1);
  lcd.print(" Eu ");
  lcd.write(3);
  lcd.print(" Arduinos!");
 
    pinMode(aquecedorPin,OUTPUT); 
    pinMode(bomba,OUTPUT);
    pinMode(btAlim,INPUT_PULLUP);
    digitalWrite(bomba,LIGA);
    digitalWrite(aquecedorPin,DESLIGA);
   ;
 
 
   
  delay(10000);
     // locate devices on the bus
      Serial.print("Locating devices...");
      sensors.begin();
      Serial.print("Found ");
      Serial.print(sensors.getDeviceCount(), DEC);
      Serial.println(" devices.");
     
      // report parasite power requirements
      Serial.print("Parasite power is: "); 
      if (sensors.isParasitePowerMode()) Serial.println("ON");
      else Serial.println("OFF");
 
      if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");
       Serial.print("Device 0 Address: ");
      printAddress(insideThermometer);
      Serial.println();
     
      // set the resolution to 9 bit (Each Dallas/Maxim device is capable of several different resolutions)
      sensors.setResolution(insideThermometer, 12);
      
      Serial.print("Device 0 Resolution: ");
      Serial.print(sensors.getResolution(insideThermometer), DEC); 
      Serial.println();
 
      //sensors.setResolution(12);
      
    // We start by connecting to a WiFi network
      Serial.println();
      Serial.println();
      Serial.print("Connecting to ");
      Serial.println(WIRELESS[LinhaRede][0]);
      lcd.clear();
      lcd.home();
      lcd.print("Conectando WIFI");
      lcd.setCursor(0, 1);
      lcd.print(WIRELESS[LinhaRede][0]);
       
      /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
         would try to act as both a client and an access-point and could cause
         network-issues with your other WiFi-devices on your WiFi-network. */
 
 
reconectaWIFI();
        
  delay(1000);
 
     
 //   server.begin();
    timeClient.begin();
     
    // ######## Servidor WEB
 
  server.on ( "/", handleRoot );
 // server.on ( "/test.svg", drawGraph );
  server.on ( "/inline", []() {
    server.send ( 200, "text/plain", "this works as well" );
  } );
  server.onNotFound ( handleNotFound );  
    server.begin();
  Serial.println ( "HTTP server started" );
 
}
 
int value = 0;
 
void loop(){
  digitalWrite(bomba,LIGA); 
   
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Sem Conexao WIFI . . .");
    erroWIFI++;
    delay(500);
      if (erroWIFI >3){
        reconectaWIFI();
      }
  }else{
    erroWIFI=0;
  }
        
timeClient.update();
String formattedTime = timeClient.getFormattedTime();
 
//textoEnvia=formattedTime.substring(0,5);
  Serial.print("Relogio: ");
  Serial.print(formattedTime);  
 
 
// ################  Temperatura Agua
      sensors.requestTemperatures(); // Send the command to get temperatures
      tempAgua=sensors.getTempC(insideThermometer);
      Serial.print(" |T. Agua: ");
      Serial.print(tempAgua);
 

// ############ Atualizando LCD
    
  tempoAtualLCD = millis();
  if (tempoAtualLCD - tempoAnteriorLCD >= tempoAtualizacaoLCD) {
      tempoAnteriorLCD=tempoAtualLCD;
      lcd.clear();
      lcd.home();
      lcd.write(2);
     // lcd.print(" ");
      lcd.print(formattedTime.substring(0,5));
    if(estadoLCD==HIGH){
        lcd.print("  Ambiente");
        lcd.setCursor(0, 1);
        //lcd.print("Temp ");
        lcd.print(temperatura);
        lcd.print((char)223);
        lcd.print("C");
        lcd.print(" Umidade ");
        lcd.print(umidade);
        lcd.print("%");
        estadoLCD=LOW;
    }else{
        lcd.print("   Aquario");
        lcd.setCursor(0, 1);
      //  lcd.print("Temp ");
        String tempAguaS=String(tempAgua);
        lcd.print(tempAguaS.substring(0,4));
        lcd.print((char)223);
        lcd.print("C "); 
        lcd.print(estadoAquecedorS);
        estadoLCD=HIGH;
    }
  }
     
// ############ AQUECEDOR
 
    if (tempAgua<=tempMin){
       estadoAquecedor=LIGA;
       estadoAquecedorBD=HIGH;
       estadoAquecedorS="Aquec.Lig"; 
    //   Serial.println(" |Aquecedor Ligado "); 
      }   
      if (tempAgua>=tempMax){
         estadoAquecedor=DESLIGA;
         estadoAquecedorBD=LOW;
         estadoAquecedorS="Aquec.Des"; 
     //    Serial.println(" |Aquecedor Desligado ");
      }
      Serial.print(" |");
      Serial.print(estadoAquecedorS);
     digitalWrite(aquecedorPin,estadoAquecedor);
 
//#########  Botão de alimentação
  while (digitalRead(btAlim)==!HIGH){
    alimenta=HIGH;
    Serial.println(" Botao Alimentacao pressionado  ");
    delay(50);
    digitalWrite(bomba,DESLIGA);
    digitalWrite(lamp,LIGA);
     lcd.backlight();
     luzBt=HIGH;
     tempoInicioLamp=millis();
  }
 
  if (alimenta==HIGH){
      alimenta=LOW;
      digitalWrite(lamp,LIGA);
      for (int x=20 ; x>0 ; x--){
        lcd.clear();
        lcd.home();  
        lcd.print("Alimentacao ");
        lcd.print(x); 
        lcd.print("min");
        lcd.setCursor(0, 1);
        timeClient.update();
        String formattedTime = timeClient.getFormattedTime();
        lcd.write(2);
     // lcd.print(" ");
        lcd.print(formattedTime.substring(0,5));
        lcd.print(" ");
        for(int y=x ; y>0 ; y--){     
            lcd.write(3);
        }
        digitalWrite(aquecedorPin,DESLIGA);
        digitalWrite(bomba,DESLIGA);
        //Serial.println("\n\n\n");
        Serial.print("##Pausa para Alimentacao## >> ");
        Serial.print(x);       
        Serial.println("  Minutos  ##  ");   
        if (x==10){ // liga a bomba por 10 segundos
          digitalWrite(bomba,LIGA);
          delay(10000); // liga a bomba por 10 segundos
          digitalWrite(bomba,DESLIGA);
        }
       // Serial.println("\n\n\n"); 
        delay(esperaAlim);
      }
      Serial.print("### Religando ###");      
      Serial.println("\n\n\n");
      digitalWrite(bomba,LIGA);    
       
      if (estadoAquecedor == LIGA)
         digitalWrite(aquecedorPin,LIGA);
      delay(espera*2);
      digitalWrite(lamp,DESLIGA);       
    } 
 

  }
 
 
//#########  Luz do Display de LCD
 
  if (valorLDR >=valorClaro){
     lcd.backlight();
  }else{
    lcd.noBacklight();
  }
 
 
 
//################### acesso no Banco para gravação
 tempoAtualBD = millis();
  if ((tempoAtualBD - tempoAnteriorBD >= tempoAtualizacaoBD)|| forcaAtualizacao==HIGH) {
      tempoAnteriorBD=tempoAtualBD;
      forcaAtualizacao=LOW;
      Serial.print("   Conectando Banco ...");
        if (conn.connect(server_addr, 3306, userBD, passwordBD)) {
          //delay(1000);
          Serial.println("   OK OK OK.");
            //char INSERT_SQL[] ="INSERT INTO vansa631_arduinodisplay.tbtexto (nome, email, texto) VALUES ('Arduino', 'arduino@gmail.com', 'Texto do arduino')";
             
            char INSERIR_TEMP1[] = "insert into vansa631_aquario.tbSensores (sensor, valorSensor,aquecedor) values ('agua', %f , %d)";
            char INSERT_SQL1[128];
            sprintf(INSERT_SQL1, INSERIR_TEMP1, tempAgua, estadoAquecedorBD);
            MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
            cur_mem->execute(INSERT_SQL1);
           
            char INSERIR_TEMP2[] = "insert into vansa631_aquario.tbSensores (sensor, valorSensor) values ('ambiente', %d)";
            char INSERT_SQL2[128];
            sprintf(INSERT_SQL2, INSERIR_TEMP2, temperatura);     
            cur_mem->execute(INSERT_SQL2);
       
       
            char INSERIR_UMID[] = "insert into vansa631_aquario.tbSensores (sensor, valorSensor) values ('umidade', %d)";
            char INSERT_SQL3[128];
            sprintf(INSERT_SQL3, INSERIR_UMID, umidade);
            cur_mem->execute(INSERT_SQL3);
 
          
            char INSERIR_LDR[] = "insert into vansa631_aquario.tbSensores (sensor, valorSensor) values ('LDR', %d)";
            char INSERT_SQL4[128];
            sprintf(INSERT_SQL4, INSERIR_LDR, valorLDR);
            cur_mem->execute(INSERT_SQL4);
             
             
             
            delete cur_mem;   
            conn.close();
            delay(500);
            erroBD=0;
       
            
// ##############  IP DDNS mais ou menos
               WiFiClient client;
              const int httpPort = 80;
              if (!client.connect(host, httpPort)) {
                Serial.println("connection failed");
                return;
              }
             
            String url = "/ip/ip2.php?";
                      url += "qc=";
                      url += formattedTime; 
             
             client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                           "Host: " + host + "\r\n" +
                           "Connection: close\r\n\r\n");
              unsigned long timeout = millis();
              while (client.available() == 0) {
                if (millis() - timeout > 5000) {
                  Serial.println(">>> Client Timeout !");
                  erroWIFI=30;
                  client.stop();
                  return;
                }
              }
       
        } else{
          Serial.println("   Conexão Falhou."); 
//          tape = textoEnvia;
          erroBD++;
            if (erroBD >5){
              delay(3000);
              reconectaWIFI();
            }    
        }
  }
             
  
  server.handleClient();
     
  delay(1000);
} // loop
 
void reconectaWIFI(){
  WiFi.begin(WIRELESS[LinhaRede][0], WIRELESS[LinhaRede][1]);
  delay(2000);
    if (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
         //erroWIFI++;  
       if (erroWIFI >3){
          LinhaRede++;
         if (LinhaRede >= NumRedes){
            LinhaRede=0;
            }      
            Serial.println("DESCONECTANDO o WIFI.");
            WiFi.disconnect();
            delay(5000);
            WiFi.begin(WIRELESS[LinhaRede][0], WIRELESS[LinhaRede][1]);
            Serial.print("Reconectando o WIFI. . . .  ");
            Serial.println(WIRELESS[LinhaRede][0]);
            lcd.clear();
            lcd.home();
            lcd.print("Conectando WIFI");
            lcd.setCursor(0, 1);
            lcd.print(WIRELESS[LinhaRede][0]);
            delay(5000);
            erroWIFI=0;
            erroBD=0;
             
          }
      }       
        if (WiFi.status() == WL_CONNECTED) {
//            WiFi.config(ip, gateway, subnet, dns1);
            Serial.println("");
            Serial.println("WiFi connected");  
            Serial.print("IP address: ");
            Serial.print(WiFi.localIP());
            Serial.print("   Netmask: ");
            Serial.print(WiFi.subnetMask());
            Serial.print("  Gateway: ");
            Serial.println(WiFi.gatewayIP());
            lcd.clear();
            lcd.home();
            lcd.print(WIRELESS[LinhaRede][0]);
            lcd.setCursor(0, 1);
            lcd.print(WiFi.localIP());
            delay(10000);            
        }     
}
 
// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
<!DOCTYPE html>
<html>
  <head>
    <link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css" integrity="sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO" crossorigin="anonymous">
  <meta charset=utf-8">
	<title> Aquário IoT - Projeto Integrador VI </title>
	<meta http-equiv="refresh" content=300>
   <script type="text/javascript" src="Imagens\ambiente.png"></script>
  
  </head>
  <body>
<!DOCTYPE html>
<html>
  <head>
  <meta charset=utf-8">
   
  </head>
  <body>
	 <table class="columns">
      <tr>
        <td>
 ﻿<!DOCTYPE html>
  
 <br>
		
   <script type="text/javascript" src="https://www.gstatic.com/charts/loader.js"></script>
   <script type="text/javascript">
      google.charts.load('current', {'packages':['gauge']});
      google.charts.setOnLoadCallback(drawChartAgua);
	 
      function drawChartAgua() {

        var dataAgua = google.visualization.arrayToDataTable([
          ['Label', 'Value'],
          ['Água', 27.4],
        ]);

        var optionsAgua = {
          width: 400, height: 120,
          redFrom: 29, redTo: 40,
          yellowFrom:15, yellowTo: 26,
          greenFrom:26, greenTo:29,
          max:40,
          min:10,
          minorTicks: 10,
		  yellowColor:'#0099ff'
		  
        };

        var chartAgua = new google.visualization.Gauge(document.getElementById('chart_divAgua'));

        chartAgua.draw(dataAgua, optionsAgua);
      }

        
    </script>



		
    <td><a href="Imagens\agua.png"><div id="chart_divAgua" style="width: 120px; height: 120px;"></div></a> </td>

		
  


<br>


<br><br>
	<center><small> Projeto Integrador VI</small><br></center>     
    <center><small>Última atualização:  04/05/2024 - 20:02 </small><br></center>
<br><br>

<center><a href="painel2.html" class="btn btn-primary btn-lg active" role="button" aria-pressed="tue">Desligar filtro</a>
    <a href="painel3.html" class="btn btn-primary btn-lg active" role="button" aria-pressed="tue">Atualizar</a></center>

<script src="https://code.jquery.com/jquery-3.3.1.slim.min.js" integrity="sha384-q8i/X+965DzO0rT7abK41JStQIAqVgRVzpbzo5smXKp4YfRvH+8abtTE1Pi6jizo" crossorigin="anonymous"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.14.3/umd/popper.min.js" integrity="sha384-ZMP7rVo3mIykV+2+9J3UJ46jBk0WLaUAdn689aCwoqbBJiSnjAK/l8WvCWPIPm49" crossorigin="anonymous"></script>
<script src="https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/js/bootstrap.min.js" integrity="sha384-ChfqqxuZUCnJSK3+MXmPNIyE6ZbWh2IMqE241rYiqJxyMiZ6OW/JmZQ5stwEULTy" crossorigin="anonymous"></script>

  </body>
</html>


