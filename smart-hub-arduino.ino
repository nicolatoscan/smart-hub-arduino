#include <SoftwareSerial.h>
#include "./env.c"

#define LOG 1

SoftwareSerial espSerial(7, 6);

void serialPrintln(String s)
{
	if (LOG)
	{
		Serial.println(s);
	}
}
void serialPrint(String s)
{
	if (LOG)
	{
		Serial.print(s);
	}
}

void sendCommand(String s, int d)
{
	serialPrint("arduino $ ");
	serialPrintln(s);
	espSerial.println(s);
	delay(d);
}

void readResponse()
{
	while (espSerial.available())
	{
		serialPrintln(espSerial.readString());
		delay(10);
	}
	serialPrintln("");
}

void sendCommandAndReadResponse(String s, int d) {
	sendCommand(s, d);
	readResponse();
}

void setup()
{
	Serial.begin(9600);
	espSerial.begin(9600);
	pinMode(13, OUTPUT);

	//CONNECT TO WIFI AND START SERVER
	sendCommandAndReadResponse("AT+RST", 5000);
	//SendCommand("AT+CWJAP=\"wifi\",\"password\"", 5000);
	String wifiSSID(WIFI_SSID);
	String wifiPassword(WIFI_PASSWORD);
	sendCommandAndReadResponse("AT+CWJAP=\"" + wifiSSID + "\",\"" + wifiPassword + "\"", 5000);
	sendCommandAndReadResponse("AT+CWMODE=1", 1000);
	sendCommandAndReadResponse("AT+CIFSR", 1000);
	sendCommandAndReadResponse("AT+CIPMUX=1", 1000);
	sendCommandAndReadResponse("AT+CIPSERVER=1,80", 1000);

	serialPrintln(" -- SET-UP COMPLETED");
}

void loop()
{
	readIncomingRequest();
	delay(200);
}

String connectionId;
void readIncomingRequest()
{
	if (espSerial.available())
	{
		if (espSerial.find("+IPD,")) //HTTP CALL
		{
			delay(10);
			connectionId = espSerial.readStringUntil(',');
			if (espSerial.find("GET "))
			{
				if (espSerial.read() == '/')
				{
					String cmd = espSerial.readStringUntil(' ');
					serialPrint("COMMAND: ");
					serialPrintln(cmd);
					sendResponse(cmd);
				}
			}
		}
	}
}

void sendResponse(String cmd)
{
	String json = "{\"cmd\": \"" + cmd + "\"}";

	String res = "HTTP/1.1 200 OK\r\n";
	res += "Access-Control-Allow-Origin: *\r\n";
	res += "Connection: keep-alive\r\n";
	res += "Content-Type: application/json\r\n";
	res += "\r\n" + json;

	//SEND RESPONSE
	sendCommand("AT+CIPSEND=" + connectionId + "," + res.length(), 10);
	sendCommand(res, 10);

	//CLOSE CONNECTION
	String closeCommand = "AT+CIPCLOSE=" + connectionId; // close the socket connection
	sendCommand(closeCommand, 10);
}

/*
0,CONNECT

+IPD,0,143:GET /ciao HTTP/1.1
User-Agent: Wget/1.ve

*/
