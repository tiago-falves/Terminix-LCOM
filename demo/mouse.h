#pragma once

#include <stdbool.h>
#include <stdint.h>

#define DISABLE_MOUSE_BYTE 0xA7
#define MOUSE_START_POSITION 300
#define ENABLE_MOUSE_BYTE 0xA8
#define CHECK_MOUSE_INTERFACE_BYTE 0xA8
#define WRITE_BYTE_TO_MOUSE 0xD4
#define DISABLE_MOUSE_BYTE 0xA7
#define MOUSE_IRQ 12
#define MOUSE_EVENT BIT(0)

typedef struct {
  __int16_t x;
  __int16_t y;
  __int16_t new_x;
  __int16_t new_y;
  xpm_image_t mouse_img;
} Mouse;


Mouse *create_mouse();
int mouse_subscribe(uint8_t *bit_no);
int mouse_unsubscribe();
void packet_byte_handler(int *position, uint8_t bytes[3]);
void packet_handler(struct packet *pp, uint8_t packet[3]);
int mouse_send_command(uint8_t command);
void mouse_enable_interrupts();
void mouse_disable_interrupts();
int mouse_flush_OBF();
int reset_KBC_command_byte(uint8_t byte);
int check_if_IBF_full();
void mouse_actions_analyser(struct mouse_ev * mouse_event);
void mouse_handler(int *position, uint8_t packet[3]);
void update_mouse_position(struct packet pp);
struct mouse_ev mouse_event_detector(struct packet *pp, uint8_t *event);

