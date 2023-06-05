#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
// #include <pthread.h>

#include <mqtt.h>

void MQTTSetup();
void MQTTRefresher();
void MQTTSendPlayhead();
extern "C" {
    void MQTTCallback(void** unused, struct mqtt_response_publish *published);
}
