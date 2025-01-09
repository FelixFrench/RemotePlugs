#ifndef NET_CREDS_H
#define NET_CREDS_H

class NetCreds {
public:
    static const char* ssid;
    static const char* password;
};

const char* NetCreds::ssid = "peanutbutter";
const char* NetCreds::password = "01010101";

#endif