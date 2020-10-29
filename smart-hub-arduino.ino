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
	readResponse();
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

void setup()
{
	Serial.begin(9600);
	espSerial.begin(9600);
	pinMode(13, OUTPUT);

	//CONNECT TO WIFI AND START SERVER
	sendCommand("AT+RST", 5000);
	//SendCommand("AT+CWJAP=\"wifi\",\"password\"", 5000);
	String wifiSSID(WIFI_SSID);
	String wifiPassword(WIFI_PASSWORD);
	sendCommand("AT+CWJAP=\"" + wifiSSID + "\",\"" + wifiPassword + "\"", 5000);
	sendCommand("AT+CWMODE=1", 1000);
	sendCommand("AT+CIFSR", 1000);
	sendCommand("AT+CIPMUX=1", 1000);
	sendCommand("AT+CIPSERVER=1,80", 1000);

	serialPrintln(" -- SET-UP COMPLETED");
}

void loop()
{
	readIncomingRequest();
	delay(200);
}

int connectionId = -1;
void readIncomingRequest()
{
	if (espSerial.available())
	{
		if (espSerial.find("+IPD,")) //HTTP CALL
		{
			delay(10);
			connectionId = espSerial.read() - 48;
			if (espSerial.find("GET "))
			{
				if (espSerial.read() == '/')
				{
					String cmd = espSerial.readStringUntil(' ');
					serialPrint("COMMAND: ");
					serialPrintln(cmd);
				}
			}
		}
	}
}

void sendResponse()
{
}

/*
0,CONNECT

+IPD,0,143:GET /ciao HTTP/1.1
User-Agent: Wget/1.ve

*/
