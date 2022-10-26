//Para o código foi utilizado a lógica booleana, que por padrão tem como 1- True e 0- False, porém aqui a lógica foi invertida.
//Isso acontece na transmissão de dados Digitais para o ESP32

//Bibliotecas incluidas
#include <WiFi.h> //https://www.usinainfo.com.br/blog/esp32-wifi-comunicacao-com-a-internet/
#include "DHT.h" //https://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-sensor-arduino-ide/
#include <ESP32Servo.h>

   
//Definições - pinos e tipos de sensores
//pinos analogicos - todos do outro lado, e o pino 2,4,15
#define DHTPIN 23 //Digital
#define DHTTYPE DHT11 //Necessário especificar para a bilbioteca "DHT.h"

//Servo motor
#define SERVOPIN 22 //Digital - Precisa suportar PWM

//Sensor de chuva
#define CHUVAPIN_D 21 //Digital
#define CHUVAPIN_A 34 //Analogico

//Sensor de humidade do solo
#define SOLOPIN_D 20 //Digital - não tem utilidade aqui
#define SOLOPIN_A 34 //Analogico


//Classes (Dar nome aos bois)
DHT dht(DHTPIN, DHTTYPE);
WiFiServer server(80);
Servo myservo;


//vars - interrupção
hw_timer_t * timer = NULL; //create a hardware timer
hw_timer_t * timer1 = NULL; //create a hardware timer
bool coletadht = false; //coletar dados do DHT a cada 2 segundos
bool lerservo = false;
int16_t conta = 0; //int de 16 bits


//vars - wifi
const char* ssid = "\x4D\x61\x72\x6C\x6F\x6E\x5F\x48\x61\x72\x61\x64\x61";   //Nome do wifi que deseja conectar
const char* password =  "\x72\x61\x66\x66\x61\x6D\x6F\x72\x65\x69\x72\x61\x6D\x61\x6E\x6F";  //Senha do Wifi (Fabão criptografou a senha e o nome do wifi)


//vars - dht11 - variáveis
  float h = 0.0;
  float t = 0.0;
  float hic = 0.0;


//vars - servomotor - variáveis
  int pos = 0;
  bool aciona_servo = false;
  bool toldo = false;


//vars - sensor de chuva
  bool chuva_D0 = true; //logica inversa - 1 é seco e 0 é molhado
  int chuva_A0 = 0;


//vars - sensor de umidade do solo
  int solo_A0; //VARIÁVEL QUE ARMAZENA O PERCENTUAL DE UMIDADE DO SOLO
  int analogSoloSeco = 4095; //VALOR MEDIDO COM O SOLO SECO (VOCÊ PODE FAZER TESTES E AJUSTAR ESTE VALOR)
  int analogSoloMolhado = 1100; //VALOR MEDIDO COM O SOLO MOLHADO (VOCÊ PODE FAZER TESTES E AJUSTAR ESTE VALOR)
  int percSoloSeco = 0; //MENOR PERCENTUAL DO SOLO SECO (0% - NÃO ALTERAR)
  int percSoloMolhado = 100; //MAIOR PERCENTUAL DO SOLO MOLHADO (100% - NÃO ALTERAR)


//Função da interrupção
//COLOCAR SOMENTE INCREMENTOS - TRAVA SE USAR SERIAL.PRINT OU VARIAVEIS COM FLOAT
void IRAM_ATTR INT0_DHT(){ 
  coletadht = true; //executar função umiadade();
  lerservo = true; //função servo executa a cada 2 segundos
  conta++; //mostrado na página web
}


void IRAM_ATTR INT1_SERVO(){
  if(chuva_D0 == false){
  aciona_servo = true; //executar funcao servo();
  }
}


//Função executado no inicio, somente 1 vez
void setup() {
  Serial.begin(9600); //Tira as medições analógicas

  //Interrupção****************
  timer = timerBegin(0, 80, true);
  timer1 = timerBegin(1, 80, true);
  
  timerAttachInterrupt(timer, &INT0_DHT, true);
  timerAlarmWrite(timer, 2000000, true);

  timerAttachInterrupt(timer1, &INT1_SERVO, true);
  timerAlarmWrite(timer1, 15000, true);

  timerAlarmEnable(timer); //Começa o alarme
  timerAlarmEnable(timer1);
  Serial.println("start timer");

  
  //DHT11****************
  Serial.println(F("DHTxx test!"));
  dht.begin();

  //Servomotor*********************
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);    // standard 50 hz servo
  myservo.attach(SERVOPIN, 500, 2400); 

  
  //Servidor****************
  //Serial.println();
  //Serial.print("Conectando-se a ");
  //Serial.println(ssid);
  WiFi.begin(ssid, password); //Conectando-se a rede
  WiFi.softAP("esp32", "esp32");

  //Esperando o wifi
  Serial.println("");
  Serial.println("WiFi conectada.");
  Serial.print("IP Local: ");
  Serial.println(WiFi.localIP());
  Serial.print("IP SoftAP: ");
  Serial.println(WiFi.softAPIP());
 
  server.begin();

  pinMode(21, INPUT);
}
 
void loop() {
  
  umidade(); //função *INTEIRA* executa somente em uma condição baseada na interrupção
             //poderia estar inteira na interrupção, mas não é por causa de limitações.
             
  servomotor(); 
  
  solo();  
  
  chuva(); 

  configwifi(); 
}

//WIFI Desativa os pinos ADC2, usar os pinos ADC1 os que fica perto do VP/EN
void solo(){ //Calibragem - valor seco (min) 4095 - valor molhado (max) - ~1100
 analogReadResolution(12);
 solo_A0 = analogRead(SOLOPIN_A);
 solo_A0 = constrain(solo_A0,analogSoloMolhado,analogSoloSeco); //MANTÉM valorLido DENTRO DO INTERVALO (ENTRE analogSoloMolhado E analogSoloSeco)
 solo_A0 = map(solo_A0,analogSoloMolhado,analogSoloSeco,percSoloMolhado,percSoloSeco); //EXECUTA A FUNÇÃO "map" DE ACORDO COM OS PARÂMETROS PASSADOS
}

void configwifi(){
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //Serial.write(c);
        if (c == '\n') {
          if (currentLine.length() == 0) {        
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            
            client.print("<HEAD>"); //https://forum.arduino.cc/t/auto-refresh-on-browser/101300/3
            client.print("<meta http-equiv=\"refresh\" content=\"2\">"); //"2\" para 2 segundos.
            client.print("<TITLE />Projeto Treinee</title>");
            client.print("</head>");
            client.println();
            if(h == 0){
              client.print("INICIANDO... AGUARDE!!!<br><b>Se demorar muito, deu ruim. Tire e coloque de volta os componentes.</b><br><br>");
            }
            client.print("IP de <b>ACESSO LOCAL:</b> ");
            client.print(WiFi.localIP());
            client.print("<br>IP SoftAP (Conectar na rede do ESP):");
            client.print(WiFi.softAPIP());

            
            client.print("<br><br>Sensor de Tempratura e umidade DHT11<br>Umidade (%):");
            client.print(h);
            client.print("<br>Temperatura (C):");
            client.print(t);
            client.print("<br>Indice de calor (C):");
            client.print(hic);

            
            client.print("<br><br>Sensor de chuva + servomotor:");
            client.print("<br>Ta chovendo?");
              if(chuva_D0 == true){ //nao ta chovendo - Lógica booliana invertida
                client.print(" Nao (Toldo Aberto)");
              }
              else if(chuva_D0 == false) //ta chovendo
              {
                client.print(" Sim (Toldo fechado)");  
              }

            client.print("<br><br>Sensor de umiade do solo:");
            client.print("<br>Umidade (%): ");
            client.print(solo_A0); 

            
            client.print("<br><br>Pagina atualizada ");
            client.print(conta);
            client.print(" vezes<br><br>--Fabao esteve aqui<br>--Fabao eh o melhor de todos");
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    client.stop();
    Serial.println("Client Disconnected.");
  }
}

void umidade(){
    if(coletadht == true){
      
      h = dht.readHumidity();  // Mede a unidade do ambiente
      t = dht.readTemperature(); //Mede a temperatura do ambiente (em °C)

      hic = dht.computeHeatIndex(t, h, false); //Calcula o índice de calor

      Serial.print("H (Solo):");
      Serial.print(solo_A0);
      Serial.print(F(" Humidity: "));
      Serial.print(h);
      Serial.print(F("%  Temperature: "));
      Serial.print(t);
      Serial.print(F("°C "));
      Serial.print(F("Heat index: "));
      Serial.print(hic);
      Serial.println(F("°C "));
 
      coletadht = false;
    }
}

void servomotor(){ //chuva: false - chovedo
    if (chuva_D0 == false && lerservo == true)
    {      
     if (pos < 180)
     {
       pos++; myservo.write(pos); //Está chovendo!!!
     }

     if(pos == 180)
     {
         lerservo = false;
     }
    }
    
      if (chuva_D0 == true && lerservo == true) //chuva: true - nao ta chovendo
  {
       aciona_servo = false;
       if (pos > 0)
       {
         pos--; myservo.write(pos); //Parou ede chover!!
       }

       if(pos == 0)
       {
         lerservo = false;
       }
  }
}


void chuva(){ //Calibragem - valor seco (min) 4095 - valor molhado (max) - ~1100
  chuva_D0 = digitalRead(CHUVAPIN_D); //<- essa variavel aciona o servomotor
}
