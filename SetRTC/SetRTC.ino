#include <Ds1302.h>

// DS1302 RTC instance
Ds1302 rtc(5, 7, 6);

uint8_t parseDigits(char* str, uint8_t count)
{
    uint8_t val = 0;
    while(count-- > 0) val = (val * 10) + (*str++ - '0');
    return val;
}

const static char* WeekDays[] =
{
    "Not valid",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
    "Sunday"
};

void setup()
{
    // initialize the RTC
    rtc.init();

    Serial.begin(115200);
    Serial.println("Input the date and time in UTC (YYMMDDWhhmmss): ");
}


void loop()
{
    static char buffer[13];
    static uint8_t char_idx = 0;

    if (char_idx == 13)
    {
        // structure to manage date-time
        Ds1302::DateTime dt;

        dt.year = parseDigits(buffer, 2);
        dt.month = parseDigits(buffer + 2, 2);
        dt.day = parseDigits(buffer + 4, 2);
        dt.dow = parseDigits(buffer + 6, 1);
        dt.hour = parseDigits(buffer + 7, 2);
        dt.minute = parseDigits(buffer + 9, 2);
        dt.second = parseDigits(buffer + 11, 2);

        // set the date and time
        rtc.setDateTime(&dt);

        char_idx = 0;

        Ds1302::DateTime now;
        rtc.getDateTime(&now);

        Serial.print("20");
        Serial.print(now.year);    // 00-99
        Serial.print('-');
        if (now.month < 10) Serial.print('0');
        Serial.print(now.month);   // 01-12
        Serial.print('-');
        if (now.day < 10) Serial.print('0');
        Serial.print(now.day);     // 01-31
        Serial.print(' ');
        Serial.print(WeekDays[now.dow]); // 1-7
        Serial.print(' ');
        if (now.hour < 10) Serial.print('0');
        Serial.print(now.hour);    // 00-23
        Serial.print(':');
        if (now.minute < 10) Serial.print('0');
        Serial.print(now.minute);  // 00-59
        Serial.print(':');
        if (now.second < 10) Serial.print('0');
        Serial.print(now.second);  // 00-59
        Serial.println(" UTC");
    }

    if (Serial.available())
    {
        buffer[char_idx++] = Serial.read();
    }
}
