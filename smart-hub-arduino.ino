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
	readResponse(s);
}

void readResponse(String name)
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
	char connectCommand[80];
	strcpy(connectCommand, "AT+CWJAP=\"");
	strcat(connectCommand, WIFI_SSID);
	strcat(connectCommand, "\",\"");
	strcat(connectCommand, WIFI_PASSWORD);
	strcat(connectCommand, "\"");

	sendCommand(connectCommand, 5000);
	sendCommand("AT+CWMODE=1", 1000);
	sendCommand("AT+CIFSR", 1000);
	sendCommand("AT+CIPMUX=1", 1000);
	sendCommand("AT+CIPSERVER=1,80", 1000);

	serialPrintln(" -- SET-UP COMPLETED");
	
}

void loop()
{
}
