#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "sys/node-id.h"
#include "os/dev/button-hal.h"
#include "os/dev/serial-line.h"
#include "os/dev/leds.h"

#include "coap-engine.h"
#include "coap-blocking-api.h"

#include "sys/log.h"
#include "sys/etimer.h"

#define LOG_MODULE "NODE"
#define LOG_LEVEL LOG_LEVEL_INFO

#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define SERVER_EP "coap://[fd00::1]:5683"
#define SERVER_REGISTRATION "registration"


#define SENSOR_TYPE "motion_sensor"
#define SIMULATION_INTERVAL 30
#define TOGGLE_INTERVAL 10
#define TIMEOUT_INTERVAL 30

static struct etimer register_timer;
static struct etimer simulation;

bool registered = false;

extern coap_resource_t motion_sensor;


/*---------------------------------------------------------------------------*/
PROCESS(coap_client, "CoAP Client");
PROCESS(sensor_node, "Sensor node");
AUTOSTART_PROCESSES(&coap_client, &sensor_node);

/*---------------------------------------------------------------------------*/
void response_handler(coap_message_t *response){
    const uint8_t *chunk;
    if(response == NULL) {
        puts("Request timed out");
        return;
    }
    int len = coap_get_payload(response, &chunk);
    printf("|%.*s\n", len, (char *)chunk);
}

/*---------------------------------------------------------------------------*/
/**
 * Node behave as coap_client in order to register to the rpl_border_router.
 */
//codice coap client client
//serve solo per la connessione al border router dei sensori coap
//si illumina se si connette
PROCESS_THREAD(coap_client, ev, data){

    static coap_endpoint_t server_ep;
    static coap_message_t request[1];
    uip_ipaddr_t dest_ipaddr;

    PROCESS_BEGIN();
    coap_endpoint_parse(SERVER_EP, strlen(SERVER_EP), &server_ep);

    etimer_set(&register_timer, TOGGLE_INTERVAL * CLOCK_SECOND);

    while(1) {

        printf("Waiting connection..\n");
        PROCESS_YIELD();

        if((ev == PROCESS_EVENT_TIMER && data == &register_timer) || ev == PROCESS_EVENT_POLL) {

            if(NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)){
                printf("--Registration--\n");

                coap_init_message(request, COAP_TYPE_CON, COAP_GET, 0);
                coap_set_header_uri_path(request, (const char *)&SERVER_REGISTRATION);
                char msg[300];
                strcpy(msg, "{\"Resource\":\"motion_resource\"}");
                //strcat(msg, SENSOR_TYPE);
                //strcat(msg, "}");
                printf("MSG registration motion.c : %s\n", msg);
                coap_set_payload(request, (uint8_t*) msg, strlen(msg));
                COAP_BLOCKING_REQUEST(&server_ep, request, response_handler);
                registered = true;
                leds_toggle(LEDS_GREEN);
                break;
            }

            else{
                printf("Not rpl address yet\n");
            }
            etimer_reset(&register_timer);
        }
    }

    LOG_INFO("REGISTERED\nStarting motion server\n");
    /*coap_activate_resource(&motion_sensor, "motion_resource");
    etimer_set(&simulation, CLOCK_SECOND * SIMULATION_INTERVAL);
    LOG_INFO("Simulation\n");
    while (1) {
        PROCESS_YIELD();
        //ogni 30 secondi triggera il controllo e genera random isDetected
        if (ev == PROCESS_EVENT_TIMER && data == &simulation) {
            printf("Trigger Motion\n");
            motion_sensor.trigger();
            etimer_set(&simulation, CLOCK_SECOND * SIMULATION_INTERVAL);
        }
    }*/
    PROCESS_END();
}

PROCESS_THREAD(sensor_node, ev, data){

	PROCESS_BEGIN();
	coap_activate_resource(&motion_sensor, "motion_resource");
    etimer_set(&simulation, CLOCK_SECOND * SIMULATION_INTERVAL);
    LOG_INFO("Simulation\n");
    while (1) {
        PROCESS_YIELD();
        //ogni 30 secondi triggera il controllo e genera random isDetected
        if (ev == PROCESS_EVENT_TIMER && data == &simulation) {
            printf("Trigger Motion\n");
            motion_sensor.trigger();
            etimer_set(&simulation, CLOCK_SECOND * SIMULATION_INTERVAL);
        }
    }
	PROCESS_END();
}


