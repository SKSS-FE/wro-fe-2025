#include "distance_sensor.h"
#include <Arduino.h>

DistanceSensor::DistanceSensor(uint8_t trigPin, uint8_t echoPin)
    : trigPin_(trigPin), echoPin_(echoPin) {}

void DistanceSensor::begin() {
    pinMode(trigPin_, OUTPUT);
    pinMode(echoPin_, INPUT);
}

float DistanceSensor::read() {
    long duration;
    float distance;
    digitalWrite(trigPin_, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin_, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin_, LOW);
    duration = pulseIn(echoPin_, HIGH, 30000); // timeout 30ms
    if (duration > 0) {
        distance = duration * 0.0343 / 2.0;
        return distance;
    } else {
        return -1.0;
    }
}
