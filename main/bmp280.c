#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_event_loop.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

//wifi sockets etc requirements
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

//wifi requirements
#define EXAMPLE_WIFI_SSID "troutstream"
#define EXAMPLE_WIFI_PASS "password"
#define PORT 80
#include "esp_wifi.h"
#include "../components/wifisetup.h"

//i2c and periferal requirements
#include "../components/i2c.h"
#include "../components/ssd1306.h"

//tcp_server_task and globals declare
//does server setup and waits for http_request
//    index.html - returns webpage
//    GetData GET - reads http post into rx_buffer
//                  and returns outstr

int cnt = 0;
uint8_t regdata[32];
char outstr[1024];      
char rx_buffer[1024];  
int digT1, digT2, digT3;
int digP1, digP2, digP3, digP4, digP5, digP6, digP7, digP8, digP9;
double ctemp = 20000, pres = 1000000;
#include "../components/bmp280.h"
#include "../components/tcp_server_task.h"

void app_main()
{
    ESP_ERROR_CHECK( nvs_flash_init() );
    initialise_wifi();  //in wifisetup.h
    wait_for_ip();      //in wifisetup.h

    i2c_init();         //in i2c.h
    i2cdetect();        //in i2c.h

    bmp280_cal ();
    
    //start tcp server - after request is following task run bmp280_getdata()
    xTaskCreate(tcp_server_task, "tcp_server", 8192, NULL, 4, NULL);  //seperate .h

    // added in ssd1305 old display
    ssd1305_init ();
    char display_string [100];
    double pres_local, pres_ref = 0;
    int presint, presdec, heightint = 0, heightdec = 0;
    while (1) {
        // get new data
        bmp280_getdata ();    // in components/bmp280.h
        // process data
        pres_local = pres /10;
        if (cnt < 50) pres_ref = pres_local; //number of seconds before pres cal
        //8266 don't do float print
        presint = (int) (pres_local / 100);
        presdec = ((int) pres_local) % 100;
        heightint = (int) ((28.4*(pres_ref - pres_local)) / 100);
        heightdec = ((int) (28.4*(pres_ref - pres_local))) % 100;
        if (heightdec < 0) heightdec = -1 * heightdec;
        sprintf(display_string, "4 Altimeter|||4 %4d.%02dhPa|||4  %d.%02d ft", 
                             presint, presdec, heightint, heightdec);
        printf ("%s\n", display_string);
        ssd1305_text ( display_string );
        ++cnt;
        vTaskDelay(100);     // 1sec updata
    }
}

