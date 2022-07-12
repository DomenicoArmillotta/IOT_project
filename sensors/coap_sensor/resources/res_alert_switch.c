#include "contiki.h"
#include "coap-engine.h"
#include <string.h>
#include "time.h"
#include "os/dev/leds.h"
#include "sys/etimer.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "alert switch actuator"
#define LOG_LEVEL LOG_LEVEL_DBG

static bool isPressed = true;

static void get_switch_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_event_handler(void);


//qui costruisco la response che devo dare al client

EVENT_RESOURCE(alert_switch_actuator, //--> name
"title=\"alert switch sensor: ?obs",   //---> descriptor (obs significa che è osservabile)
get_switch_handler,
NULL,
NULL,
NULL,
res_event_handler); //--> handler invoke auto  every time the state of resource change


static void get_switch_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    int length;

        char msg[300];
        // T = true
        // N = negative
        char val = isPressed == 1 ? 'T': 'N';
        strcpy(msg,"{\"info\":\"");
        strncat(msg,&val,1);
        strcat(msg,"\"}");
        length = strlen(msg);
        memcpy(buffer, (uint8_t *)msg, length);

        coap_set_header_content_format(response, TEXT_PLAIN);
        coap_set_header_etag(response, (uint8_t *)&length, 1);
        coap_set_payload(response, (uint8_t *)buffer, length);
}

static void res_event_handler(void)
{
    //if (isPressed)
    coap_notify_observers(&alert_switch_actuator);
}
