#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <mqtt.h>

void MQTTSetup(const char* address, const char* port);
void MQTTRefresher();
void MQTTSendPlayhead();
extern "C" {
    void MQTTCallback(void** unused, struct mqtt_response_publish *published);
}
