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
#define CONN_TRY_INTERVAL 1
#define REG_TRY_INTERVAL 1
#define SENSOR_TYPE "alert_actuator"
#define SIMULATION_INTERVAL 30
#define TIMEOUT_INTERVAL 30

#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678
static struct simple_udp_connection udp_conn;


/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

/* RESOURCES */
double intensity = 5.0;

PROCESS(alert_server, "Server for the alert actuator");
AUTOSTART_PROCESSES(&alert_server);

static void udp_rx_callback(struct simple_udp_connection *c, const uip_ipaddr_t *sender_addr,
         uint16_t sender_port, const uip_ipaddr_t *receiver_addr, uint16_t receiver_port, const uint8_t *data, uint16_t datalen){
  LOG_INFO("Received response %s ", data);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");
}

//*************************** GLOBAL VARIABLES *****************************//
char* service_url = "/registration";

static bool connected = false;
static bool registered = false;
bool pressed = false;


static struct etimer wait_connectivity;
static struct etimer wait_registration;
static struct etimer simulation;
static struct etimer timeout_timer;


extern coap_resource_t alert_actuator;
extern coap_resource_t  alert_switch_actuator;

//*************************** UTILITY FUNCTIONS *****************************//
static void check_connection()
{
    if (NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr))
    {
         LOG_INFO("BR reachable");
        // TODO: notificare in qualche modo che si è connessi
        connected = true;
        LOG_INFO("Sending request %u to ", message_number);
        LOG_INFO_6ADDR(&dest_ipaddr);
        LOG_INFO_("\n");
        char buf[300];
        sprintf(buf, "Message %d from node %d", message_number, node_id);
        message_number++;
        simple_udp_sendto(&udp_conn, buf, strlen(buf) + 1, &dest_ipaddr);
    }
    else
    {
       LOG_INFO("BR not reachable\n");
       etimer_reset(&wait_connectivity);
    }
}

void client_chunk_handler(coap_message_t *response)
{
    const uint8_t* chunk;

    if (response == NULL)
    {
        LOG_INFO("Request timed out\n");
        etimer_set(&wait_registration, CLOCK_SECOND * REG_TRY_INTERVAL);
        return;
    }

    int len = coap_get_payload(response, &chunk);

    if(strncmp((char*)chunk, "Success", len) == 0)
        registered = true;
    else
        etimer_set(&wait_registration, CLOCK_SECOND * REG_TRY_INTERVAL);
}



//*************************** THREAD *****************************//
PROCESS_THREAD(alert_server, ev, data)
{

    button_hal_button_t *btn;

    static struct etimer et;

    uip_ipaddr_t dest_ipaddr;
    static unsigned int message_number = 0;

    PROCESS_BEGIN();
    etimer_set(&et, 2*CLOCK_SECOND);

    btn = button_hal_get_by_index(0);

    static coap_endpoint_t server_ep;
    static coap_message_t request[1]; // This way the packet can be treated as pointer as usual

    etimer_set(&wait_connectivity, CLOCK_SECOND * CONN_TRY_INTERVAL);

    simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL, UDP_SERVER_PORT, udp_rx_callback);

    while (!connected) {
        PROCESS_WAIT_UNTIL(etimer_expired(&wait_connectivity));
        check_connection();
    }
    LOG_INFO("CONNECTED\n");

    while (!registered) {
        LOG_INFO("Sending registration message\n");

        coap_endpoint_parse(SERVER_EP, strlen(SERVER_EP), &server_ep);

        coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
        coap_set_header_uri_path(request, service_url);
        //nel payload abbiamo solo il tipo di sensore = alert_actuator
        char msg[300];
        char* sensor_type = "";
        sprintf(sensor_type, "{\"Resource\":\"%s}", SENSOR_TYPE);
        strcpy(msg, sensor_type);
        coap_set_payload(request, (uint8_t*) msg, strlen(msg));
        COAP_BLOCKING_REQUEST(&server_ep, request, client_chunk_handler);
        registered = true;

        // wait for the timer to expire
        PROCESS_WAIT_UNTIL(etimer_expired(&wait_registration));
        if(registered){
            if(etimer_expired(&et)){
                leds_toggle(LEDS_GREEN);
                etimer_restart(&et);
            }
        }
    }
    LOG_INFO("REGISTERED\nStarting alert server");

    // RESOURCES ACTIVATION
    coap_activate_resource(&alert_actuator, "alert_actuator");
    coap_activate_resource(&alert_switch_actuator, "alert_switch_actuator");

    // SIMULATION
    etimer_set(&simulation, CLOCK_SECOND * SIMULATION_INTERVAL);
    LOG_INFO("Simulation\n");

    while (1) {
        PROCESS_WAIT_EVENT();

        if (ev == PROCESS_EVENT_TIMER && data == &simulation) {
            alert_actuator.trigger();
            etimer_set(&simulation, CLOCK_SECOND * SIMULATION_INTERVAL);
        }

        if ((ev == button_hal_press_event) && !pressed) {
            //registered la variabile per capire se si è registrata al border router
            if (registered) {
                printf("Button pressed\n");
                btn = (button_hal_button_t *)data;
                printf("Release event (%s)\n", BUTTON_HAL_GET_DESCRIPTION(btn));
                pressed = !pressed;
                etimer_set(&timeout_timer,TIMEOUT_INTERVAL*CLOCK_SECOND);
            }
        }
        if (pressed) {
            if (registered) {
                // Client has put info, stop the running timer (if any)
                etimer_stop(&timeout_timer);
                printf("Running timer stopped!\n");
                alert_switch_actuator.trigger();
                pressed = false;

            }
        }

    }

    PROCESS_END();
}
