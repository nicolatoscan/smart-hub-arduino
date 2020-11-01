#include <SoftwareSerial.h>
#include "./env.c"

#define LOG 1

bool releStatus1 = true;
bool releStatus2 = true;
const int relePin1 = PIN3;
const int relePin2 = PIN2;

SoftwareSerial espSerial(7, 6);
void setup()
{
	Serial.begin(115200);
	espSerial.begin(115200);
	pinMode(13, OUTPUT);
	pinMode(relePin1, OUTPUT);
	pinMode(relePin2, OUTPUT);
	digitalWrite(relePin1, HIGH);
	digitalWrite(relePin2, HIGH);
	sendCommandAndReadResponse("AT+RST", 10000);
	// sendCommandAndReadResponse("AT+CIOBAUD=9600", 1000);
	//CONNECT TO WIFI AND START SERVER
	//SendCommand("AT+CWJAP=\"wifi\",\"password\"", 5000);
	String wifiSSID(WIFI_SSID);
	String wifiPassword(WIFI_PASSWORD);
	sendCommandAndReadResponse("AT+CWJAP=\"" + wifiSSID + "\",\"" + wifiPassword + "\"", 10000);
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

void sendCommandAndReadResponse(String s, int d)
{
	sendCommand(s, d);
	readResponse();
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
					handleCommand(cmd);
					sendResponse(cmd);
				}
			}
		}
	}
}

void handleCommand(String cmd)
{
	if (cmd == "reley1")
	{
		releStatus1 = !releStatus1;
		digitalWrite(relePin1, releStatus1 ? HIGH : LOW);
	}
	else if (cmd == "reley2")
	{
		releStatus2 = !releStatus2;
		digitalWrite(relePin2, releStatus2 ? HIGH : LOW);
	}
}

void sendResponse(String cmd)
{
	String json = "{ \"cmd\":\"" + cmd + "\", \"reley1\":\"" + (releStatus1 ? "1" : "0") + "\", \"reley2\":\"" + (releStatus2 ? "1" : "0") + "\"}\n";
	String res = "HTTP/1.1 200 OK\n";
	res += "Content-Type: application/json\n";
	res += "Connection: close\n";
	res += "\n" + json;

	//SEND RESPONSE
	sendCommandAndReadResponse("AT+CIPSEND=" + connectionId + "," + res.length(), 10);
	sendCommandAndReadResponse(res, 10);

	//CLOSE CONNECTION
	String closeCommand = "AT+CIPCLOSE=" + connectionId; // close the socket connection
	sendCommandAndReadResponse(closeCommand, 10);
}

/*
0,CONNECT

+IPD,0,143:GET /ciao HTTP/1.1
User-Agent: Wget/1.ve

*/
