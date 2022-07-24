#include "contiki.h"
#include "coap-engine.h"
#include <string.h>
#include "time.h"
#include "os/dev/leds.h"
#include "sys/etimer.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "alert actuator"
#define LOG_LEVEL LOG_LEVEL_DBG

static bool isActive = false;
static int intensity = 10;

static void get_intensity_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void post_switch_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_event_handler(void);


//qui costruisco la response che devo dare al client

EVENT_RESOURCE(alert_actuator, //--> name
"title=\"alert sensor: ?obs",   //---> descriptor (obs significa che è osservabile)
get_intensity_handler,
post_switch_handler,
NULL,
NULL,
res_event_handler); //--> handler invoke auto  every time the state of resource change

//get per sapere lo stato
static void get_intensity_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    // Create a JSON message with the detected presence value and led value
    // In both the resources the get_handler return the current sensor values
    int length;

    char msg[300];
    // T = true
    // N = negative
    char val2 = isActive == true ? 'T': 'N';
    strcpy(msg,"{\"info\":\"");
    strncat(msg,&val2,1);
    //strcat(msg,"\" \"");
    strcat(msg,"\", \"intensity\":\"");
    char intensity_str[400];
    sprintf(intensity_str, "%d", intensity);
    //printf("intensity: %s\n", intensity_str);
    strcat(msg,intensity_str);
    strcat(msg,"\"}");
    printf("MSG alert: %s\n",msg);
    length = strlen(msg);
    memcpy(buffer, (uint8_t *)msg, length);

    coap_set_header_content_format(response, TEXT_PLAIN);
    coap_set_header_etag(response, (uint8_t *)&length, 1);
    coap_set_payload(response, (uint8_t *)buffer, length);

}


//usata per fare on/off
static void post_switch_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    LOG_INFO("Handling switch post request...\n");

    size_t len = 0;
    const uint8_t* payload = NULL;
    bool success = true;

    if((len = coap_get_payload(request, &payload)))
    {
        isActive = !isActive;
        if(isActive){
            //ogni volta che viene chiamata di seguito una ON intensifica di 10 la potenza
            /*if(intensity<100){
                intensity=intensity+10;
            }*/
            intensity = 100;
            LOG_INFO("Switch on\n");
        }
        else{
            //reset dell'intensita appena viene spento così riparte da 10
            intensity=10;
            LOG_INFO("Switch off\n");
        }
        /*if (strncmp((char*)payload, "ON", strlen("ON")) == 0)
        {
            isActive = true;
            //ogni volta che viene chiamata di seguito una ON intensifica di 10 la potenza
            if(intensity<100){
                intensity=intensity+10;
            }
            LOG_INFO("Switch on\n");
        }
        if (strncmp((char*)payload, "OFF", strlen("OFF")) == 0)
        {
            isActive = false;
            //reset dell'intensita appena viene spento così riparte da 10
            intensity=10;
            LOG_INFO("Switch off\n");
        }*/
    } else
        success = false;

    if(!success)
        coap_set_status_code(response, BAD_REQUEST_4_00);
}


static void res_event_handler(void)
{
    coap_notify_observers(&alert_actuator);
}
