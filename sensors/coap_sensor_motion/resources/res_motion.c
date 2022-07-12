#include "contiki.h"
#include "coap-engine.h"
#include <string.h>
#include "time.h"
#include "os/dev/leds.h"
#include "sys/etimer.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "motion sensor"
#define LOG_LEVEL LOG_LEVEL_DBG
#define EVENT_INTERVAL 30

static bool isDetected = false;

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_event_handler(void);

//qui costruisco la response che devo dare al client

EVENT_RESOURCE(motion_sensor, //--> name
"title=\"Motion sensor: ?POST/PUT\";obs",   //---> descriptor (obs significa che è osservabile)
res_get_handler, //--> handler
NULL,
NULL,
NULL,
res_event_handler); //--> handler invoke auto  every time the state of resource change

//qui ci sono le response che il server manda al client
static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    // Create a JSON message with the detected presence value and led value
    // In both the resources the get_handler return the current sensor values
    int length;

    char msg[300];
    // T = true
    // N = negative
    char val2 = isDetected == 1 ? 'T': 'N';
    strcpy(msg,"{\"isDetected\":\"");
    strncat(msg,&val2,1);
    strcat(msg,"\"}");
    length = strlen(msg);
    memcpy(buffer, (uint8_t *)msg, length);

    coap_set_header_content_format(response, TEXT_PLAIN);
    coap_set_header_etag(response, (uint8_t *)&length, 1);
    coap_set_payload(response, (uint8_t *)buffer, length);

}

static void res_event_handler(void)
{


    srand(time(NULL));
    int random = rand() % 2;

    bool new_isDetected = isDetected;
    if(random == 0){
        new_isDetected=!isDetected;
    }

    if(new_isDetected != isDetected){
        isDetected = new_isDetected;
        // Notify all the observers
        coap_notify_observers(&motion_sensor);
    }

}
