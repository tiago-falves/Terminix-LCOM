// IMPORTANT: you must include the following line in all your C files
#include "drawing.h"

#include "Game.h"
#include "lcom/vbe.h"
#include "mouse.h"

#include <lcom/lcf.h>
#include <lcom/liblm.h>
#include <lcom/proj.h>

#include <stdbool.h>
#include <stdint.h>

extern uint32_t global_counter;
extern uint8_t status_code, scan_code;
extern bool read_mouse;

// Any header files included below this line should have been created by you

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/proj/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/proj/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

int(proj_main_loop)(int argc, char *argv[]) {

  // Initialize Graphic mode
  graphic_mode_init(RGB_VIDEO_MODE);

  Terminix * terminix = create_terminix();

  int timer_irq, mouse_irq, keyboard_irq;
  int ipc_status, r;
  message msg;
  int position = 0;
  uint8_t packet[3];

  if (subscribe_periphericals(&timer_irq, &mouse_irq, &keyboard_irq)) {
    printf("Error subscribing periphericals");
    return 1;
  }

  while (scan_code != ESC_BREAK_CODE && terminix->state != 3) {

    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);

      continue;
    }
    if (is_ipc_notify(ipc_status)) { // received notification

      switch (_ENDPOINT_P(msg.m_source)) {
      case HARDWARE: /* hardware interrupt notification */

        if (msg.m_notify.interrupts & timer_irq) { /* subscribed interrupt */

          timer_int_handler();

          if (global_counter % (sys_hz() / FRAME_RATE) == 0) {
            render();
          }
        }
        if (msg.m_notify.interrupts & keyboard_irq) { /* subscribed interrupt */

          keyboard_handler();
        }
        else if (msg.m_notify.interrupts & mouse_irq) { /* subscribed interrupt */

          mouse_handler(&position, packet);
        }

        break;

      default:
        break; /* no other notifications expected: do nothing */
      }
    }
  }

  unsubscribe_periphericals();

  vg_exit();

  // video_test_rectangle(0x115,20,20,50,50, 303030);
  // mouse_test_gesture(10, 5);
  return 0;
}
