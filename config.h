#ifndef __CONFIG_H__
#define __CONFIG_H__

// DEFINE
#define USERAGENT "pestiborso sensor"
#define TIMETOUPDATE 10000 // frequency of update - every 20 seconds

// cosm parameters
#define COSMHOST "api.cosm.com"
#define COSMPATH "/v2/feeds/yourfeedid.csv"
#define COSMAPIKEY "yourapikey"

// server parameters
#define SERVERHOST "yourserverhost"
#define SERVERPATH "/feed/"
#define SERVERAPIKEY "yourotherapikey"

// network parameters
#define NETWORK "ssid"   
#define NETWORKPASS "pass1234" 

#define LIGHTPIN 0
#define HUMIDTYPIN 1
#endif
