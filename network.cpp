#include "network.h"
#include "app.h"

#include "libs/MQTT-C/examples/templates/posix_sockets.h"

static int sockfd = -1;
static struct mqtt_client client;
static uint8_t sendbuf[2048]; // sendbuf should be large enough to hold multiple whole mqtt messages
static uint8_t recvbuf[1024]; // recvbuf should be large enough any whole mqtt message expected to be received

const char* client_id = NULL; // Create an anonymous session

void MQTTSetup()
{
    sockfd = open_nb_socket(appState.mqtt_address, appState.mqtt_port);
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
}

void MQTTRefresher()
{
    if (sockfd == -1) return;
    mqtt_sync(&client);
}

void MQTTSendPlayhead()
{
    if (sockfd == -1) return;
    const char* topic = appState.mqtt_topic;

    // print a message
    char application_message[256];
    snprintf(application_message, sizeof(application_message), "%f", appState.playhead.to_seconds());
    Log("Sending: %s \"%s\"", topic, application_message);

    // publish the time
    mqtt_publish(&client, topic, application_message, strlen(application_message) + 1, MQTT_PUBLISH_QOS_0);

    // check for errors
    if (client.error != MQTT_OK) {
        Log("ERROR: %s\n", mqtt_error_str(client.error));
    }
}

extern "C" {
void MQTTCallback(void** unused, struct mqtt_response_publish *published)
{

}
}
