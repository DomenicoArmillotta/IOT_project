#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "contiki.h"
#include "sys/etimer.h"
//#include "dev/leds.h"

#include "node-id.h"
#include "net/ipv6/simple-udp.h"
#include "net/ipv6/uip.h"
#include "os/dev/leds.h"
#include "net/ipv6/uip-ds6.h"
#include "os/dev/button-hal.h"
#include "net/ipv6/uip-debug.h"
#include "routing/routing.h"

#include "coap-engine.h"
#include "coap-blocking-api.h"

#define SERVER_EP "coap://[fd00::1]:5683"
#define REG_TRY_INTERVAL 10
#define SENSOR_TYPE "alert_actuator"
#define SIMULATION_INTERVAL 30
#define TIMEOUT_INTERVAL 30

#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "NODE"
#define LOG_LEVEL LOG_LEVEL_INFO

#define SERVER_REGISTRATION "registrationAlert"

/* RESOURCES */
double intensity = 5.0;

//*************************** GLOBAL VARIABLES *****************************//

//static bool connected = false;
static bool registered = false;
bool pressed = false;


//static struct etimer wait_connectivity;
static struct etimer wait_registration;
static struct etimer simulation;
static struct etimer timeout_timer;


extern coap_resource_t alert_actuator;
extern coap_resource_t  alert_switch_actuator;

//*************************** UTILITY FUNCTIONS *****************************//


PROCESS(alert_server, "Server for the alert actuator");
PROCESS(sensor_node, "Sensor node");
AUTOSTART_PROCESSES(&alert_server, &sensor_node);

void response_handler(coap_message_t *response){
    const uint8_t *chunk;
    if(response == NULL) {
        puts("Request timed out");
        return;
    }
    int len = coap_get_payload(response, &chunk);
    printf("|%.*s\n", len, (char *)chunk);
}

//*************************** THREAD *****************************//
PROCESS_THREAD(alert_server, ev, data)
{
    //static struct etimer et;
    uip_ipaddr_t dest_ipaddr;
    static coap_endpoint_t server_ep;
    static coap_message_t request[1]; // This way the packet can be treated as pointer as usual

    PROCESS_BEGIN();

    //etimer_set(&et, 10*CLOCK_SECOND);
    coap_endpoint_parse(SERVER_EP, strlen(SERVER_EP), &server_ep);
    etimer_set(&wait_registration, CLOCK_SECOND * REG_TRY_INTERVAL);

    while (1) {
        printf("Waiting connection..\n");
        PROCESS_YIELD();

        if((ev == PROCESS_EVENT_TIMER && data == &wait_registration) || ev == PROCESS_EVENT_POLL) {

            if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)){
                printf("--Registration--\n");

                coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
                coap_set_header_uri_path(request, (const char *)&SERVER_REGISTRATION);
                char msg[300];
                strcpy(msg, "{\"Resource\":\"alert_actuator\"}");
                printf("MSG registration alert.c : %s\n", msg);
                coap_set_payload(request, (uint8_t*) msg, strlen(msg));
                COAP_BLOCKING_REQUEST(&server_ep, request, response_handler);
                registered = true;
                leds_toggle(LEDS_GREEN);
                break;
            }

            else{
                printf("Not rpl address yet\n");
            }
            etimer_reset(&wait_registration);
        }

    }
    LOG_INFO("REGISTERED\nStarting alert server\n");

    PROCESS_END();
}


PROCESS_THREAD(sensor_node, ev, data){
    button_hal_button_t *btn;
	PROCESS_BEGIN();
    // RESOURCES ACTIVATION
    coap_activate_resource(&alert_actuator, "alert_actuator");
    //coap_activate_resource(&alert_switch_actuator, "alert_switch_actuator");
    btn = button_hal_get_by_index(0);
    // SIMULATION
    etimer_set(&simulation, CLOCK_SECOND * SIMULATION_INTERVAL);
    LOG_INFO("Simulation\n");

    while (1) {
        PROCESS_WAIT_EVENT();

        if (ev == PROCESS_EVENT_TIMER && data == &simulation && pressed) {
            printf("Alarm event!\n");
            alert_actuator.trigger();
            etimer_set(&simulation, CLOCK_SECOND * SIMULATION_INTERVAL);
        }

        if ((ev == button_hal_press_event)) {
            //registered la variabile per capire se si è registrata al border router
            if (registered) {
                if(!pressed){
                    printf("Button pressed\n");
                    btn = (button_hal_button_t *)data;
                    printf("Release event (%s)\n", BUTTON_HAL_GET_DESCRIPTION(btn));
                    pressed = true;
                    alert_actuator.trigger();
                    etimer_set(&timeout_timer,TIMEOUT_INTERVAL*CLOCK_SECOND);
                }
                else{
                    etimer_stop(&timeout_timer);
                    printf("Release alarm!\n");
                    alert_actuator.trigger();
                    pressed = false;
                }
            }
        }
    }
    PROCESS_END();
}

