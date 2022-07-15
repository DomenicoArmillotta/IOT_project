/*
 * Questa classe serve per la creazione della mqtt
 * viene inizializzato il client (client process)
 * vengono mandati messagi con valori random ogni 30s dal publisher alla coda (publisher process)
 * volendo si possono creare più topic
 * i dati vengono presi dall'app py in base al topic per l'elaborazione
 * USIAMO Mosquitto COME BROKER SULLA VM
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "net/routing/routing.h"
#include "mqtt.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-icmp6.h"
#include "net/ipv6/sicslowpan.h"
#include "sys/etimer.h"
#include "sys/ctimer.h"
#include "lib/sensors.h"
#include "dev/button-hal.h"
#include "dev/leds.h"
#include "os/sys/log.h"
#include "mqtt-client.h"

#include <string.h>
#include <strings.h>
/*---------------------------------------------------------------------------*/
#define LOG_MODULE "mqtt-client"
#ifdef MQTT_CLIENT_CONF_LOG_LEVEL
#define LOG_LEVEL MQTT_CLIENT_CONF_LOG_LEVEL
#else
#define LOG_LEVEL LOG_LEVEL_DBG
#endif

/*---------------------------------------------------------------------------*/
/* MQTT broker address. */
#define MQTT_CLIENT_BROKER_IP_ADDR "fd00::1"

static char *broker_ip = MQTT_CLIENT_BROKER_IP_ADDR;

// Defaukt config values
#define DEFAULT_BROKER_PORT         1883
#define DEFAULT_PUBLISH_INTERVAL    (30 * CLOCK_SECOND)

// We assume that the broker does not require authentication


/*---------------------------------------------------------------------------*/
/* Various states */
static uint8_t state;

#define STATE_INIT    	      0 // Initial state
#define STATE_NET_OK   	      1 // Network is initialized
#define STATE_CONNECTING      2 // Connecting to MQTT broker
#define STATE_CONNECTED       3  // Connection successful
#define STATE_SUBSCRIBED      4  // Topics of interest subscribed
#define STATE_DISCONNECTED    5  // Disconnected from MQTT broker

/*---------------------------------------------------------------------------*/
//creati i due processi : client e publisher
//runno i due processi
//uno per il client e uno per la pubblicazione sulla coda con valori randomici
PROCESS_NAME(mqtt_client_process);
AUTOSTART_PROCESSES(&mqtt_client_process);

/*---------------------------------------------------------------------------*/
/* Maximum TCP segment size for outgoing segments of our socket */
#define MAX_TCP_SEGMENT_SIZE    32
#define CONFIG_IP_ADDR_STR_LEN   64
/*---------------------------------------------------------------------------*/
/*
 * Buffers for Client ID and Topics.
 * Make sure they are large enough to hold the entire respective string
 */
#define BUFFER_SIZE 64

static char client_id[BUFFER_SIZE];
static char pub_topic[BUFFER_SIZE];
// static char sub_topic[BUFFER_SIZE];

static int value = 0;
static int humidity = 0;
static int light = 0;
static int gas = 0;

// Periodic timer to check the state of the MQTT client
#define STATE_MACHINE_PERIODIC (CLOCK_SECOND >> 1)
static struct etimer periodic_timer;

#define PERIODIC_TIMER 30
/*---------------------------------------------------------------------------*/
/*
 * The main MQTT buffers.
 * We will need to increase if we start publishing more data.
 */
#define APP_BUFFER_SIZE 512
static char app_buffer[APP_BUFFER_SIZE];
/*---------------------------------------------------------------------------*/
static struct mqtt_message *msg_ptr = 0;

static struct mqtt_connection conn;

/*---------------------------------------------------------------------------*/
PROCESS(mqtt_client_process, "MQTT Client");
/*---------------------------------------------------------------------------*/
static void pub_handler(const char *topic, uint16_t topic_len, const uint8_t *chunk,
            uint16_t chunk_len)
{
  printf("Pub Handler: topic='%s' (len=%u), chunk_len=%u\n", topic,
          topic_len, chunk_len);

  if(strcmp(topic, "actuator") == 0) {
    printf("Received Actuator command\n");
	printf("%s\n", chunk);
    // Do something :)
    return;
  }
}
/*---------------------------------------------------------------------------*/
//in base all'evento dell mqtt assegna l'evento allo stato
//default dal prof
static void mqtt_event(struct mqtt_connection *m, mqtt_event_t event, void *data)
{
  switch(event) {
  case MQTT_EVENT_CONNECTED: {
    printf("Application has a MQTT connection\n");

    state = STATE_CONNECTED;
    break;
  }
  case MQTT_EVENT_DISCONNECTED: {
    printf("MQTT Disconnect. Reason %u\n", *((mqtt_event_t *)data));

    state = STATE_DISCONNECTED;
    process_poll(&mqtt_client_process);
    break;
  }
  case MQTT_EVENT_PUBLISH: {
    msg_ptr = data;

    pub_handler(msg_ptr->topic, strlen(msg_ptr->topic),
                msg_ptr->payload_chunk, msg_ptr->payload_length);
    break;
  }
  case MQTT_EVENT_SUBACK: {
#if MQTT_311
    mqtt_suback_event_t *suback_event = (mqtt_suback_event_t *)data;

    if(suback_event->success) {
      printf("Application is subscribed to topic successfully\n");
    } else {
      printf("Application failed to subscribe to topic (ret code %x)\n", suback_event->return_code);
    }
#else
    printf("Application is subscribed to topic successfully\n");
#endif
    break;
  }
  case MQTT_EVENT_UNSUBACK: {
    printf("Application is unsubscribed to topic successfully\n");
    break;
  }
  case MQTT_EVENT_PUBACK: {
    printf("Publishing complete.\n");
    break;
  }
  default:
    printf("Application got a unhandled MQTT event: %i\n", event);
    break;
  }
}

static bool have_connectivity()
{
  if(uip_ds6_get_global(ADDR_PREFERRED) == NULL || uip_ds6_defrt_choose() == NULL) {
    return false;
  }
  else
  {
    return true;
  }
}

/*---------------------------------------------------------------------------*/
//SUBSCRIBER
//posso avere anche piu topic ma in questo caso ho solo weather
PROCESS_THREAD(mqtt_client_process, ev, data)
{
  PROCESS_BEGIN();
  //mqtt_status_t status;
  //char broker_address[CONFIG_IP_ADDR_STR_LEN];

  printf("MQTT Client Process\n");

  // Initialize the ClientID as MAC address -->come prof
  snprintf(client_id, BUFFER_SIZE, "%02x%02x%02x%02x%02x%02x",
                     linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
                     linkaddr_node_addr.u8[2], linkaddr_node_addr.u8[5],
                     linkaddr_node_addr.u8[6], linkaddr_node_addr.u8[7]);

  // Broker registration --> come prof
  mqtt_register(&conn, &mqtt_client_process, client_id, mqtt_event,
                  MAX_TCP_SEGMENT_SIZE);

  state=STATE_INIT;
  printf("Intitial state\n");
  // Initialize periodic timer to check the status
  etimer_set(&periodic_timer, STATE_MACHINE_PERIODIC);

  /* Main loop */
  while(1) {

    PROCESS_YIELD();

    if((ev == PROCESS_EVENT_TIMER && data == &periodic_timer) || ev == PROCESS_EVENT_POLL){
      printf("Event!\n");
      if(state==STATE_INIT){
         printf("Test Connectivity!\n");
         if(have_connectivity()==true){
            printf("Connectivity verified!");
            state = STATE_NET_OK;
         }
      }

      if(state == STATE_NET_OK){
          // Connect to MQTT server
          LOG_INFO("Connecting to MQTT server\n");
          printf("Connecting!\n");
          //memcpy(broker_address, broker_ip, strlen(broker_ip));

          //mqtt_connect(&conn, broker_address, DEFAULT_BROKER_PORT,
          //             (DEFAULT_PUBLISH_INTERVAL * 3) / CLOCK_SECOND,
          //             MQTT_CLEAN_SESSION_ON);
          mqtt_connect(&conn, broker_ip, DEFAULT_BROKER_PORT,
                       (DEFAULT_PUBLISH_INTERVAL * 3) / CLOCK_SECOND,
                       MQTT_CLEAN_SESSION_ON);
          state = STATE_CONNECTING;
      }

      if(state==STATE_CONNECTED){
          //non faccio l'iscrizione al topic perche lo faccio in py con l'app
          state = STATE_SUBSCRIBED;
      }

      if (state == STATE_SUBSCRIBED) {
        // Publish something , specify tag of topic
        sprintf(pub_topic, "%s", "info");

        sprintf(app_buffer, "{\"temp\":%d,\"humidity\":%d,\"light\":%d,,\"gas\":%d}", value, humidity,light,gas);

        value = rand() % 36;
        humidity = rand() % 101;
        light = rand() % 3;
        gas = rand() % 100;

        printf("Message: %s\n",app_buffer);
        //code to publish the message
        mqtt_publish(&conn, NULL, pub_topic, (uint8_t *)app_buffer,
             strlen(app_buffer), MQTT_QOS_LEVEL_0, MQTT_RETAIN_OFF);
      }
      else if ( state == STATE_DISCONNECTED ){
  		LOG_ERR("Disconnected form MQTT broker\n");
  		// Recover from error
      }
	  etimer_set(&periodic_timer, STATE_MACHINE_PERIODIC);
	  printf("End Loop\n");
    }

  }

  PROCESS_END();
}
