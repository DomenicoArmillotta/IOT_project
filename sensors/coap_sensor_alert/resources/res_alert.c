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

extern bool isActive = false;
static int intensity = 10;

static void get_intensity_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void post_switch_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_event_handler(void);


//qui costruisco la response che devo dare al client

EVENT_RESOURCE(alert_actuator, //--> name
"title=\"alarm actuator: ?POST/PUT\";obs;rt=\"alarm\"",
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
    if(request != NULL) {
        LOG_DBG("POST/PUT Request Sent\n");
    }

    printf("Post handler called\n");


    size_t len = 0;
    const char *state = NULL;
    int check = 1;

    if((len = coap_get_post_variable(request, "state", &state))) {
        if (atoi(state) == 1){
            leds_set(LEDS_NUM_TO_MASK(LEDS_RED));
            occupied = true;
        }

        else if(atoi(state) == 0){
            leds_set(LEDS_NUM_TO_MASK(LEDS_GREEN));
            occupied = false;
        }

        else{
            check = 0;
        }

        // occupied = atoi(state);

    }
    else{
        check = 0;
    }

    if (check){
        coap_set_status_code(response, CHANGED_2_04);
    }
    else{
        coap_set_status_code(response, BAD_REQUEST_4_00);
    }
}


static void res_event_handler(void)
{
    coap_notify_observers(&alert_actuator);
}
