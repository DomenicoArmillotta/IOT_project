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

static bool isActive = true;
static bool intensity = 10;

static void get_intensity_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void put_intensity_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void post_switch_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);


//qui costruisco la response che devo dare al client

RESOURCE(alert_actuator, //--> name
"title=\"alert sensor: ?obs",   //---> descriptor (obs significa che è osservabile)
get_intensity_handler,
post_switch_handler,
put_intensity_handler,
NULL,
res_event_handler); //--> handler invoke auto  every time the state of resource change

//get per sapere lo stato
static void get_intensity_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    LOG_INFO("Handling alarm intensity get request...\n");
    char* msg;

    int length = snprintf(NULL, 0,"%lf", intensity) + 1;
    msg = (char*)malloc((length)*sizeof(char));
    snprintf(msg, length, "%lf", intensity);



    // prepare buffer
    size_t len = strlen(msg);
    memcpy(buffer, (const void *)msg, length);

    // COAP FUNCTIONS
    coap_set_header_content_format(response, TEXT_PLAIN);
    coap_set_header_etag(response, (uint8_t *)&len, 1);
    coap_set_payload(response, buffer, len);

}

static void put_intensity_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    LOG_INFO("Handling intensity put request...\n");

    size_t len = 0;
    const uint8_t* payload = NULL;
    bool success = true;

    if((len = coap_get_payload(request, &payload)))
    {
        char* chunk = strtok((char*)payload, " ");

        chunk = strtok(NULL, " ");
        char* eptr;
        intensity = strtod(chunk, &eptr);
        printf("new_value: %f\n", intensity);

        printf("Alarm intensity changed to %lf\n", intensity);
    } else
        success = false;

    if(!success)
        coap_set_status_code(response, BAD_REQUEST_4_00);
}


static void post_switch_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    LOG_INFO("Handling switch post request...\n");

    size_t len = 0;
    const uint8_t* payload = NULL;
    bool success = true;

    if((len = coap_get_payload(request, &payload)))
    {
        if (strncmp((char*)payload, "ON", strlen("ON")) == 0)
        {
            isActive = true;
            LOG_INFO("Switch on\n");
        }
        if (strncmp((char*)payload, "OFF", strlen("OFF")) == 0)
        {
            isActive = false;
            LOG_INFO("Switch off\n");
        }
    } else
        success = false;

    if(!success)
        coap_set_status_code(response, BAD_REQUEST_4_00);
}


static void res_event_handler(void)
{
    if (isActive)
        coap_notify_observers(&alert_actuator);

}
