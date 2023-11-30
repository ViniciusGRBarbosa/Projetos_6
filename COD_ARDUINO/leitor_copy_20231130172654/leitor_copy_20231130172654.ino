/*
 *********************************************************************************************************
 ESTE CODIGO TEM A INTENÇÃO DE MOSTRAR UMA APLICAÇÃO USANDO UM ESP8266-12,
 E UM SHIELD RFID-RC522, NA PRIMEIRA VEZ APOS CARREGAR O CODIGO, ABRA O MONITOR SERIAL,
 CONFIGURE O MESMO PARA A VELOCIDADE DE 9600 BAUD RATE,E PRESSIONE O BOTÃO RESET DA SUA PLACA,
 NO MONITOR SERIAL VOCÊ IRA OBSERVAR A MENSAGEM: APROXIME SEU CARTAO (TAG).
 AO APROXIMAR SEU CARTAO,COMO ELE AINDA NAO ESTA CADASTRADO ELE NÃO ACIONARA O GPIO,
 MAS IMPRIMIRA A TAG DO MESMO, DESTA FORMA VOCE AGORA PODE COLOCAR NO CODIGO A TAG
 QUE FOI IMPRESSA NO MONITOR SERIAL DO SEU CARTÃO, OU CHAVEIRO, INSIRA A TAG NA LINHA: 
 if (conteudo.substring(1) == "AQUI SUA TAG" || "AQUI OUTRA SE QUISER") //UID 1 - 2....
 E CARREGUE O CODIGO COM SUA TAG, OU COM AS TAGS SE FOR MAIS UMA, 
 AGORA DEPOIS DE CARREGAR E PRESSIONAR O BOTÃO RESET, APROXIME O CARTÃO, E O LED
 ACENDERA POR 3 SEGUNDOS, VOCÊ PODE DEFINIR ESSE TEMPO COMO QUISER, NESTA SAIDA
 PODE USAR UM RELE PARA ABRIR UM SOLENOIDE DE UMA FECHADURA POR EXEMPLO.

 ESTE CODIGO FONTE É LIVRE E SEGUE OS PRINCIPIOS GNU, PODE SER ADAPTADO, NODIFICADO
 PARCIAL OU INTEIRAMENTE SEM PERVIO AVISO POR QUALQUER PESSOA.

 COMIGO FUNCIONOU PERFEITAMENTE SEM PROBLEMA ALGUM, MAS NÃO GARANTO SEU FUNCIONAMENTO,
 FICA POR CONTA E RISCO DE QUEM QUISER USAR, DUVIDAS FAVOR ME CONTATAR NO E-MAIL ABAIXO.

 COMIGO FUNCIONOU PERFEITAMENTE SEM PROBLEMA ALGUM, MAS NÃO GARANTO SEU FUNCIONAMENTO,
 FICA POR CIONTA E RISCO DE QUEM QUISER USAR, DUVIDAS FAZER ME CONTATAR NO E-MAIL ABAIXO.

 contato@carloskwiek.com.br 

 Carlos kwiek, engenheiro eletronico amante de eletronica e programação.
 www.carloskwiek.com.br 
 ************************************************************************************************************
 *************************PINAGEM****************************************************************************
  
  RFID-RC522   Wemos          Nodemcu
RST/Reset RST  D3  [1*]        D3 [1*]      GPIO-0  [1*]
SPI SS  SDA[3] D8  [2*]        D8 [2*]      GPIO-15 [2*]
SPI MOSI MOSI  D7              D7           GPIO-13
SPI MISO MISO  D6              D6           GPIO-12
SPI SCK SCK    D5              D5           GPIO-14

[1*] (1, 2) Configuracao tipica definindo como RST_PIN no sketch / programa.
[2*] (1, 2) Configuracao, tipica definindo como SS_PIN no sketch / programa.
[3*] O pino SDA pode ser rotulado como SS em algumas placas de MFRC522 / mais antigas, significam a mesma coisa.
******************************************************************************************************************
 */


#include <SPI.h>
#include <MFRC522.h>

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

#define RST_PIN    D3    
#define SS_PIN     D8   
#define LED        D1

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

#define WIFI_SSID "Redmi Note 8"
#define WIFI_PASSWORD "12345678"

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "AIzaSyB5lDsyy7p2f2QsSgVJeAitdP5AqPy9qnM"

/* 3. Define the RTDB URL */
#define DATABASE_URL "https://bancochaves-99730-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "projetoint7@gmail.com"
#define USER_PASSWORD "Senha@123456"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;

void setup() 
{
  Serial.begin(9600);   // Inicia a serial
  SPI.begin();      // Inicia  SPI bus
  mfrc522.PCD_Init();   // Inicia MFRC522
  Serial.println("Aproxime o seu cartao do leitor...");
  Serial.println();
  digitalWrite(D1,LOW);
  digitalWrite(D2,LOW);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  digitalWrite(D4,HIGH);
  digitalWrite(D0,HIGH);
  pinMode(D4, OUTPUT);
  pinMode(D0, OUTPUT);
  delay(1000);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Or use legacy authenticate method
  // config.database_url = DATABASE_URL;
  // config.signer.tokens.legacy_token = "<database secret>";

  // To connect without auth in Test Mode, see Authentications/TestMode/TestMode.ino

  //////////////////////////////////////////////////////////////////////////////////////////////
  // Please make sure the device free Heap is not lower than 80 k for ESP32 and 10 k for ESP8266,
  // otherwise the SSL connection will fail.
  //////////////////////////////////////////////////////////////////////////////////////////////

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  // Limit the size of response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);

  Firebase.begin(&config, &auth);

  // The WiFi credentials are required for Pico W
  // due to it does not have reconnect feature.
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
  config.wifi.clearAP();
  config.wifi.addAP(WIFI_SSID, WIFI_PASSWORD);
#endif

  Firebase.setDoubleDigits(5);

  config.timeout.serverResponse = 10 * 1000;

}

void loop() 
{

  if (Firebase.ready() && (millis() - sendDataPrevMillis > 100 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();

    /*Serial.printf("Set bool... %s\n", Firebase.RTDB.setBool(&fbdo, F("/test/bool"), count % 2 == 0) ? "ok" : fbdo.errorReason().c_str());

    Serial.printf("Get bool... %s\n", Firebase.RTDB.getBool(&fbdo, FPSTR("/test/bool")) ? fbdo.to<bool>() ? "true" : "false" : fbdo.errorReason().c_str());

    bool bVal;
    Serial.printf("Get bool ref... %s\n", Firebase.RTDB.getBool(&fbdo, F("/test/bool"), &bVal) ? bVal ? "true" : "false" : fbdo.errorReason().c_str());

    Serial.printf("Set int... %s\n", Firebase.RTDB.setInt(&fbdo, F("/test/int"), count) ? "ok" : fbdo.errorReason().c_str());

    Serial.printf("Get int... %s\n", Firebase.RTDB.getInt(&fbdo, F("/test/int")) ? String(fbdo.to<int>()).c_str() : fbdo.errorReason().c_str());

    int iVal = 0;
    Serial.printf("Get int ref... %s\n", Firebase.RTDB.getInt(&fbdo, F("/test/int"), &iVal) ? String(iVal).c_str() : fbdo.errorReason().c_str());

    Serial.printf("Set float... %s\n", Firebase.RTDB.setFloat(&fbdo, F("/test/float"), count + 10.2) ? "ok" : fbdo.errorReason().c_str());

    Serial.printf("Get float... %s\n", Firebase.RTDB.getFloat(&fbdo, F("/test/float")) ? String(fbdo.to<float>()).c_str() : fbdo.errorReason().c_str());

    Serial.printf("Set double... %s\n", Firebase.RTDB.setDouble(&fbdo, F("/test/double"), count + 35.517549723765) ? "ok" : fbdo.errorReason().c_str());

    Serial.printf("Get double... %s\n", Firebase.RTDB.getDouble(&fbdo, F("/test/double")) ? String(fbdo.to<double>()).c_str() : fbdo.errorReason().c_str());

    Serial.printf("Set string... %s\n", Firebase.RTDB.setString(&fbdo, F("/test/string"), F("Hello World!")) ? "ok" : fbdo.errorReason().c_str());

    Serial.printf("Get string... %s\n", Firebase.RTDB.getString(&fbdo, F("/test/string")) ? fbdo.to<const char *>() : fbdo.errorReason().c_str());

    // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create_Edit_Parse.ino
    FirebaseJson json;

    if (count == 0)
    {
      json.set("value/round/" + String(count), F("cool!"));
      json.set(F("value/ts/.sv"), F("timestamp"));
      Serial.printf("Set json... %s\n", Firebase.RTDB.set(&fbdo, F("/test/json"), &json) ? "ok" : fbdo.errorReason().c_str());
    }
    else
    {
      json.add(String(count), F("smart!"));
      Serial.printf("Update node... %s\n", Firebase.RTDB.updateNode(&fbdo, F("/test/json/value/round"), &json) ? "ok" : fbdo.errorReason().c_str());
    }*/

    int iVal;

    Firebase.RTDB.getInt(&fbdo, F("/abertura"), &iVal);

    //Serial.print(status);
    //Serial.print(" ");
    //Serial.println(iVal);

    if(iVal){
      digitalWrite(D0, LOW);
    }else if(!iVal){
      digitalWrite(D0, HIGH);
    }
    count++;
  }

  // Procura por cartao RFID
  if (!mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Seleciona o cartao RFID
  if (!mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  //Mostra UID na serial
  Serial.print("UID da tag :");
  String conteudo= "";
  byte letra;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Mensagem : ");
  conteudo.toUpperCase();

  if (conteudo == " D7 AD D1 4E") //UID 1 - Cartao
  {
    Serial.println("Liberado !");
    Serial.println();
    digitalWrite(D1, HIGH);     // LIGA LED OU/ ativa rele, abre trava solenoide
    delay(3000);              // DELAY /espera 3 segundos
    digitalWrite(D1, LOW);  // DESlIGA LED OU /desativa rele, fecha  trava solenoide
    if(digitalRead(D0) == HIGH){
      digitalWrite(D0,LOW);
      Firebase.RTDB.setInt(&fbdo, F("/abertura"), 1);
    } // DESlIGA LED OU /desativa rele, fecha  trava solenoide
    else{
      digitalWrite(D0,HIGH);
      Firebase.RTDB.setInt(&fbdo, F("/abertura"), 0);
    }
  } else
  {
    Serial.println("Não liberado !");
    Serial.println();
    digitalWrite(D2, HIGH);     // LIGA LED OU/ ativa rele, abre trava solenoide
    delay(3000);              // DELAY /espera 3 segundos
    digitalWrite(D2, LOW); 
    digitalWrite(D0, HIGH); // DESlIGA LED OU /desativa rele, fecha  trava solenoide
    Firebase.RTDB.setInt(&fbdo, F("/abertura"), 0);
  }
}

