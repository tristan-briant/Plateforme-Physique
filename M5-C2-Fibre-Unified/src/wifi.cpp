#include <WiFiClient.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include "pref-util.h"

void send_page(WiFiClient client);
void repondre(WiFiClient client);
void repondre_simple(WiFiClient client);

String RECEIVED_DATA;
String buffer;
const int BUFFER_LENGTH = 100;

char wifiAvailable();

char *ssid;
const char *password = "";

WiFiServer server(80);

const char index_html[] =
    R"rawliteral(<!DOCTYPE HTML>

<html>
<head>
    <title>Interface fibre optique </title>
    <meta name="viewport" content="width=device-width, initial-scale=1">  
  </head> 
  <body>
    <div class="main">
      <p>
        <input type="text" id="inputText" value="coucou" size="15"/>
        <input type="button" id="port" value="Envoyer" onclick="sendData()" />
      </p>
	  <p>
	  <!-- Vitesse de com.
	  <select name="speed" id="speed-select" onchange="changeSpeed()" >
			<option value="1">10</option>
			<option value="2">30</option>
			<option value="3">100</option>
			<option value="4">300</option>
			<option value="5">1000</option>
			<option value="6">3000</option>
			<option value="7">10000</option>
		</select>
		bit/s
    -->
		</p>
      <hr />
       <textarea id="outputText" name="w3review" rows="8" cols="30" disabled="true"></textarea>
      <p>
	  <input type="button" id="btclr" value="Clear" onclick="clearOutput()" />
	  <!-- <input type="button" id="port" value="getData" onclick="poolData()" /> -->
      </p>
    </div>
  </body>
<script>

document.addEventListener('DOMContentLoaded', setup, false);

function setup() {
    setInterval(poolData, 200);
}

function clearOutput(){
	const out=document.getElementById("outputText");
	out.value ="";
	console.log('output clear!');
}

function changeSpeed(){
	var requete = new XMLHttpRequest(); 
    var url = location + '?speed=';
	url+=document.getElementById("speed-select").value;
	requete.open("GET", url,true);
	requete.send("toto");
	console.log('speed changed! '+ url );
}

function sendData(){
	var requete = new XMLHttpRequest(); 
    var url = location + '?data=';
	url+=document.getElementById("inputText").value;
	requete.open("GET", url,true);
	requete.send(null);
	
	console.log('sent!');
}

function poolData() {
    var requete = new XMLHttpRequest(); 
    var url = location + "?pooldata";
    
    console.log(url) // Pour debugguer l'url formée    
    requete.open("GET", url, true); // On construit la requête
    requete.send(null); // On envoie !
    requete.onreadystatechange = function() { // on attend le retour
        if (requete.readyState == 4) { // Revenu !
            if (requete.status == 200) {// Retour s'est bien passé !
            	console.log(requete.responseText);
                donnees = JSON.parse(requete.responseText);
                console.log(donnees);
                const out=document.getElementById("outputText");
                newData = donnees["data"];
                if(out.value.length  + newData.length > 30*8)
                  out.value="";
	            out.value+=donnees["data"];
            }
        }
    };
	
	console.log('pool!');
}


</script>
</html>
)rawliteral";

void send_page(WiFiClient client)
{
  client.print(index_html);
}

void initWIFI()
{
  WiFi.softAP(
      // ssid,
      getDeviceName(),
      password);                    // You can remove the password parameter if you want the AP to be open.
  IPAddress myIP = WiFi.softAPIP(); // Get the softAP interface IP address.
  Serial.println(myIP);

  server.begin(); // Start the established Internet of Things network server.
}

void listenWIFI()
{
  // static WiFiClient client = 0;
  //  static String currentLine = "";

  WiFiClient client = server.available(); // listen for incoming clients.

  /*if (!client)
  {
    client = server.available(); // listen for incoming clients.
    if (client)
    {
      Serial.print("New Client:");
      currentLine = "";
    }
  }*/

  if (client)
  { // if you get a client.
    // Serial.println("New Client:");
    String currentLine = ""; // make a String to hold incoming data from the client.
    while (client.connected())
    {
      if (client.available())
      {                         // if there's bytes to read from the client.
        char c = client.read(); // store the read a byte.

        // Serial.write(c);
        if (c == '\n')
        {
          if (currentLine.length() == 0)
          {
            break;
          }
          else
          {
            if (currentLine.startsWith("GET /?pooldata"))
            {
              repondre(client);
            }
            else if (currentLine.startsWith("GET /?speed="))
            {
              String str = currentLine.substring(12, 15);
              int index = str.toInt();
              Serial.print(index);
              repondre_simple(client);
            }
            if (currentLine.startsWith("GET /?data="))
            {
              String str = currentLine.substring(11, -1);
              int i = str.indexOf(" ");
              String str0 = str.substring(0, i);
              str0.replace("%20", " ");
              if (buffer.length() + str0.length() < BUFFER_LENGTH)
                buffer += str0;
              Serial.println(str0);
              Serial.println(buffer);

              repondre_simple(client);
            }
            else if (currentLine.startsWith("GET / "))
            {
              send_page(client);
            }

            currentLine = "";
          }
        }
        else if (c != '\r')
        {
          currentLine += c; // add it to the end of the currentLine.
        }
      }
    }
    client.stop(); // close the connection.
  }
}

void repondre(WiFiClient client)
{
  // La fonction prend un client en argument

  if (RECEIVED_DATA.length() > 0)
  {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    // Autorise le cross origin
    // client.println("Access-Control-Allow-Origin: *");
    String str = RECEIVED_DATA;  //utilise str comme buffer sinon perte de character
    RECEIVED_DATA = "";

    client.println();
    client.println("{");
    client.print("\t\"data\": \"");
    client.print(str);
    client.println("\"");
    client.println("}");

    Serial.print(str);
    Serial.println("<-");
  }
  else
    repondre_simple(client);
}

void repondre_simple(WiFiClient client)
{
  // Simple acknowledge
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Length: 0");
}

char wifiAvailable()
{
  char c = 0;
  if (buffer.length() > 0)
  {
    c = buffer.charAt(0);
    buffer.remove(0, 1);
  }
  return c;
}

void wifiWrite(char c)
{
  RECEIVED_DATA += c;
}