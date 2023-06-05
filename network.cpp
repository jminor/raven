#include "network.h"
#include "app.h"

#include "libs/MQTT-C/examples/templates/posix_sockets.h"

static int sockfd = -1;
static struct mqtt_client client;
static uint8_t sendbuf[2048]; // sendbuf should be large enough to hold multiple whole mqtt messages
static uint8_t recvbuf[1024]; // recvbuf should be large enough any whole mqtt message expected to be received
static const char* client_id = NULL; // Create an anonymous session

static const char* playhead_topic = "opentimelineio/raven/playhead";

void MQTTSetup(const char* address, const char* port)
{
    Log("Connecting to MQTT broker %s:%s", address, port);
    
    sockfd = open_nb_socket(address, port);
    if (sockfd == -1) {
        Log("ERROR: Failed to open socket: %s", strerror(errno));
        return;
    }

    mqtt_init(&client, sockfd, sendbuf, sizeof(sendbuf), recvbuf, sizeof(recvbuf), MQTTCallback);
    // Ensure we have a clean session
    uint8_t connect_flags = MQTT_CONNECT_CLEAN_SESSION;
    // Send connection request to the broker.
    mqtt_connect(&client, client_id, NULL, NULL, 0, NULL, NULL, connect_flags, 400);

    // check that we don't have any errors
    if (client.error != MQTT_OK) {
        Log("ERROR: %s\n", mqtt_error_str(client.error));
        return;
    }

    // Subscribe to this topic
    mqtt_subscribe(&client, playhead_topic, 0);
}

void MQTTRefresher()
{
    if (sockfd == -1) return;
    mqtt_sync(&client);
}

void MQTTSendPlayhead()
{
    if (sockfd == -1) return;

    // print a message
    char application_message[256];
    snprintf(application_message, sizeof(application_message), "%f", appState.playhead.to_seconds());
    Log("MQTT OUT: %s %s", playhead_topic, application_message);

    // publish the time
    mqtt_publish(&client, playhead_topic, application_message, strlen(application_message) + 1, MQTT_PUBLISH_QOS_0);

    // check for errors
    if (client.error != MQTT_OK) {
        Log("ERROR: %s\n", mqtt_error_str(client.error));
    }
}

extern "C" {
void MQTTCallback(void** unused, struct mqtt_response_publish *published)
{
    // note that published->topic_name is NOT null-terminated (here we'll use %.*s precision)
    Log("MQTT  IN: %.*s %s", published->topic_name_size, published->topic_name, (const char*) published->application_message);

    size_t len = strlen(playhead_topic);
    if (published->topic_name_size == len && !strncmp((const char*)published->topic_name, playhead_topic, len)) {
        double seconds = atof((const char*)published->application_message);
        SeekPlayhead(seconds, false);
    }
}
}
