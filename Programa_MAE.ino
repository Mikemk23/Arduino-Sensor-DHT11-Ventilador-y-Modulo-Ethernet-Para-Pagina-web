#include <LiquidCrystal.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <EtherCard.h>

int rs = 9;
int e = 8;
int d4 = 5;
int d5 = 4;

//LCD
int d6 = 3;
int d7 = 2;
int numero = 0;

LiquidCrystal lcd(rs, e, d4, d5, d6, d7);

//DHT11
#define Type DHT11
int dhtPin = 7;
DHT HT(dhtPin, Type);
int humidity;
float tempC;
float tempF;

// Ventilador
int fanPin = 6;
bool fanOn = false;

int dTime = 500;

//ETHERNET MODULO

// Si colocamos 1 deshabilitamos el DHCP
// Si colocamos 0 colocamos una IP estática
#define ESTATICA 1

#if ESTATICA
  // Colocamos la dirección IP al dispositivo
  static byte myip[] = { 192,168,2,17  };
  // Colocamos la dirección IP de la puerta de enlace de nuestro router
  static byte gwip[] = { 192,168,2,1 };
#endif

// Colocamos la dirección MAC, que sera único en nuestra red
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

// Enviar y recibir buffer (TCP/IP)
byte Ethernet::buffer[500]; 

const char page[] PROGMEM =
"HTTP/1.0 503 Ethernet Hanrun HR911105A - \r\n"
"Content-Type: text/html\r\n"
"Retry-After: 600\r\n"
"\r\n"
"<html>"
  "<head><title>"
    "Ejemplo #2: Ethernet Hanrun HR911105A"
  "</title></head>"
  "<body>"
    "<h3>Hola Mundo</h3>"
    "<p><em>"
      "Esto es una prueba.<br />"
      ":)."
    "</em></p>"
  "</body>"
"</html>"
;

void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);
  HT.begin();
  pinMode(fanPin, OUTPUT); // Configura el pin del ventilador como salida
  
  //Ethernet
   Serial.begin(19200);
  Serial.println("\n[Vuelva pronto]");
  
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) 
    Serial.println( "Error al acceder al controlador Ethernet");
#if ESTATICA
  ether.staticSetup(myip, gwip);
#else
  if (!ether.dhcpSetup())
    Serial.println("DHCP falló");
#endif

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);  
  ether.printIp("DNS: ", ether.dnsip);  
}


void loop() {
  humidity = HT.readHumidity();
  tempC = HT.readTemperature();
  //tempF = HT.readTemperature(true);
  Serial.print("Humedad Relativa: ");
  Serial.print(humidity);
  Serial.print("% / Temperatura: ");
  Serial.print(tempC);
  Serial.print("ºC / ");
  Serial.print(tempF);
  Serial.println("ºF");

  lcd.setCursor(0, 0);
  lcd.print("Humedad: ");
  lcd.print(humidity);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("Temp.: ");
  lcd.print(tempC);
  lcd.print("*C");

  // Verifica si la temperatura supera los 35°C
  if (tempC > 50 && !fanOn) {
    digitalWrite(fanPin, HIGH); // Enciende el ventilador
    fanOn = true; // Actualiza el estado del ventilador
  } else if (tempC <= 50 && fanOn) {
    digitalWrite(fanPin, LOW); // Apaga el ventilador
    fanOn = false; // Actualiza el estado del ventilador
  }

  delay(dTime);
  lcd.clear();
  //Ethernet
  //Se espera un paquete TCP entrante, luego se ignora el contenido
  if (ether.packetLoop(ether.packetReceive())) {
    memcpy_P(ether.tcpOffset(), page, sizeof page);
    ether.httpServerReply(sizeof page - 1);
  }
}
