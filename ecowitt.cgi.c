#include <time.h>
#include <stdio.h>
#include "cgic.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <MQTTLinux.h>
#include <MQTTClient.h>


#define MQTTHOST "localhost"
#define MQTTPORT 1883
#define MQTTQOS	0
#define MQTTUSER "ecowitt"
#define MQTTPASS "ecowitt"
#define MQTTCLIENTID "eco"

typedef struct ecowittData
{
	time_t DataTime;
	double InteriorTemp;
	double InteriorHumidity;
	double Pressure;
	double ExteriorTemp;
	double ExteriorHumidity;
	double SensorTemp[8];
	double SensorHumidity[8];
} ecowittDataStruct;


int cgiMain() 
{
	ecowittDataStruct TempData;
	MQTTClient c;
	Network n;
	char mqtt_buffer[100],mqtt_readBuffer[100];
	int rc=0;	

//	if (strncmp(cgiRequestMethod,"POST",4)==0)
	{
		TempData.DataTime = time(NULL);
		cgiFormDoubleBounded( "tempinf", &TempData.InteriorTemp, -40, 160, 69.420);
        cgiFormDoubleBounded( "humidityin", &TempData.InteriorHumidity,0,100,50 );
		cgiFormDouble( "baromrelin",&TempData.Pressure,0);
		cgiFormDoubleBounded( "tempf", &TempData.ExteriorTemp,-40, 160, 69.420 );
		cgiFormDouble( "humidity", &TempData.ExteriorHumidity,0 );
		// lazy loop to avoid copy/paste
		for (int i=0;i<8;i++)
		{
			char tempf[8],humidity[10];
			sprintf(tempf,"temp%df",i+1);
			sprintf(humidity,"humidity%df",i+1);
			cgiFormDoubleBounded( tempf, &TempData.SensorTemp[i], -40, 160, 69.420 );
			cgiFormDouble( humidity, &TempData.SensorHumidity[i],0 );
		}
	cgiHeaderContentType("text/html");


		// send to mqtt
		NetworkInit(&n);
		
	rc  = NetworkConnect(&n,MQTTHOST,MQTTPORT);
		if (rc!=0)
		{
			fprintf(cgiOut,"Network Connect Failed\r\n");
		}



		MQTTClientInit(&c,&n,1000,mqtt_buffer,100,mqtt_readBuffer,100);
		MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
		data.willFlag = 0;
		data.MQTTVersion = 4;
		data.clientID.cstring = MQTTCLIENTID;
		data.username.cstring = MQTTUSER;
		data.password.cstring = MQTTPASS;
		data.keepAliveInterval=10;
		data.cleansession = 1;
		rc=	MQTTConnect(&c,&data);
		if (rc!=0)
		{
			fprintf(cgiOut,"MQTT Connect Failed: %d\r\n",rc);
		}
		MQTTMessage msg;
	//	char tmp[20];
	//	sprintf(tmp,"%f",TempData.InteriorTemp);
		msg.payload = &TempData.InteriorTemp;
		msg.payloadlen = sizeof(double);
		msg.qos = MQTTQOS;
		msg.retained = 0;
	
	rc  =	MQTTPublish(&c, "/ecowitt/tempin", &msg);
		if (rc!=0)
		{
		fprintf(cgiOut,"Published Failed  %d\r\n",rc);
		}
		MQTTDisconnect(&c);
		NetworkDisconnect(&n);
	/* Send the content type, letting the browser know this is HTML */
	/* Top of the page */
	fprintf(cgiOut, "<HTML><HEAD>\n");
	fprintf(cgiOut, "<TITLE>cgic test</TITLE></HEAD>\n");
	fprintf(cgiOut, "<BODY><H1>Accepted</H1>\n");
	/* If a submit button has already been clicked, act on the 
		submission of the form. */
	
	/* Finish up the page */
	fprintf(cgiOut, "</BODY></HTML>\n");
	
	}
	/*else
	{
fprintf(stderr,"Access was a GET");
	cgiHeaderContentType("text/html");
	fprintf(cgiOut, "<HTML><HEAD>\n<TITLE>cgic test</TITLE></HEAD>\n<BODY><H1>YOU FUCKED UP</H1>\n</BODY></HTML>\n");
	}*/
	return 0;
}