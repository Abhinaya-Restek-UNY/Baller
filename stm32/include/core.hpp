
#include "main.h"
#include "motor.h"
#include "serial_hub.h"

#define ENCODER_PACKET_ID 1
#define TIME_PACKET_ID 3
#define MOTOR_PACKET_ID 4

typedef struct __attribute__((packed)) {
	uint64_t delta;
	uint64_t timestamp;
	uint8_t unit_index;
} packet_time;

typedef struct __attribute__((packed)) {
	uint64_t timestamp;
	int32_t deltaA;
	int32_t deltaB;
	int32_t deltaC;
} packet_encoder;

typedef struct __attribute__((packed)) {
	int16_t front_left;
	int16_t front_right;
	int16_t back_left;
	int16_t back_right;
} packet_motor;

uint64_t micros64(void);

void serial_hub_motor_direction_cb(void *ctx, uint8_t *data, fsize_t size);

void serial_hub_write_cb(void *ctx, uint8_t *data, fsize_t size);

#ifdef __cplusplus
extern "C" {
#endif
void setup();

void loop();
#ifdef __cplusplus
}
#endif
