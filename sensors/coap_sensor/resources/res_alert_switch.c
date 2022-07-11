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

static bool isActive = true;

static void get_intensity_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void post_switch_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_event_handler(void);


//qui costruisco la response che devo dare al client

RESOURCE(alert_switch_actuator, //--> name
"title=\"alert switch sensor: ?obs",   //---> descriptor (obs significa che è osservabile)
get_switch_handler,
NULL,
put_switch_handler,
NULL,
NULL); //--> handler invoke auto  every time the state of resource change


static void get_switch_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{

}

static void put_switch_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{

}

