
/*--------------------------------------------------------------
 .
  Author:       W.A. Smith, http://startingelectronics.org
  Modified by:  Paul Lamb
--------------------------------------------------------------*/
#include <LiquidCrystal.h> //Load Liquid Crystal Library
#include <SPI.h>
#include <Ethernet.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#define ONE_WIRE_BUS 6
#define trigPin 14 //Sensor Echo pin connected to Arduino pin 14
#define echoPin 15 //Sensor Trip pin connected to Arduino pin 15
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float tempF = 0;
float tempC = 0;
LiquidCrystal LCD(3,2,16,17,18,19);  //Create Liquid Crystal Object called LCD
// MAC address from Ethernet shield sticker under board
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,1,70); // IP address, may need to change depending on network
EthernetServer server(504);  // create a server at port 80

void setup()
{
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    Ethernet.begin(mac, ip);  // initialize Ethernet device
    LCD.begin(16,2); //Tell Arduino to start your 16 column 2 row LCD
    server.begin();           // start to listen for clients
    sensors.begin();
}

void loop()
{
    long percent;
    float duration, distance;
    sensors.requestTemperatures();
    tempC = sensors.getTempCByIndex(0);
    tempF = sensors.toFahrenheit(tempC);
    EthernetClient client = server.available();  // try to get client
    LCD.clear();
    digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 73.9;
  if(distance>36)distance=36;
  percent = 100-(4*(distance-6));//Offset is assumed to be 6"
  LCD.setCursor(0,0);
  LCD.print("Salt Level");  //Print Message on First Row
  LCD.setCursor(11,0);  //Set cursor to 11th column of first row
  LCD.print("     "); //Clear percentage
  LCD.setCursor(11,0);
  LCD.print(percent); //Print percentage
  LCD.print("%");
  LCD.setCursor(0,1);   //Set Cursor again to first column of second row
  LCD.print("Temp = "); 
  LCD.print(tempF);
  delay(250); //pause to let things settle
    if (client) {  // got client?
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {   // client data available to read
                char c = client.read(); // read 1 byte (character) from client
                // last line of client request is blank and ends with \n
                // respond to client only after last line received
                if (c == '\n' && currentLineIsBlank) {
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: close");
                    client.println();
                    // send web page
                    client.println("<!DOCTYPE html>");
                    client.println("<html>");
                    client.println("<head>");
                    client.println("<title>Arduino Web & Temp Page</title>");
                    client.println("</head>");
                    client.println("<body>");
                    client.print("<font face='arial'>");
                    client.print("<font color='blue'size='6'>");
                    client.print("<p>Hi from the pumphouse!</p>");
                    client.print("The inside temperature is ");
                    client.print(tempF);
                    client.println(" <sup>o</sup>F");
                    client.print("<p></p>");
                    client.print("The salt level is at ");
                    client.print(percent);
                    client.print("%");
                    client.print("<meta http-equiv=\"refresh\" content=\"2\">");
                    client.println("</body>");
                    client.println("</html>");
                    break;
                }
                // every line of text received from the client ends with \r\n
                if (c == '\n') {
                    // last character on line of received text
                    // starting new line with next character read
                    currentLineIsBlank = true;
                } 
                else if (c != '\r') {
                    // a text character was received from client
                    currentLineIsBlank = false;
                }
            } // end if (client.available())
        } // end while (client.connected())
        delay(1);      // give the web browser time to receive the data
        client.stop(); // close the connection
    } // end if (client)
}
