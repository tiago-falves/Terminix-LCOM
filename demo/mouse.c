#include <lcom/lcf.h>
#include <lcom/lab3.h>
#include "i8042.h"
#include "mouse.h"
#include <stdint.h>
#include "pointer.xpm"
#include "drawing.h"
#include "Terminix.h"


uint8_t status_code, packet_byte;
int x_sum = 0,y_sum = 0, y_max_value;
static int hook_id = 2;
bool read_mouse = true;


Mouse * create_mouse(){

  Mouse * mouse = malloc(sizeof (Mouse));
  xpm_image_t  mouse_img;
  convert_xpm_img(pointer_xpm,&mouse_img);
  mouse->x = MOUSE_START_POSITION;
  mouse->y = MOUSE_START_POSITION;
  mouse->new_x = MOUSE_START_POSITION;
  mouse->new_y = MOUSE_START_POSITION;
  mouse->mouse_img = mouse_img;
  
  draw_xpm(mouse_img,mouse->x,mouse->y);
 
  return mouse;
}


void mouse_handler(int *position, uint8_t packet[3]){

  struct mouse_ev mouse_event;
  uint8_t event = 0;
  
  mouse_ih();


  if (read_mouse){
    packet_byte_handler(position,packet);
    if (*position == 3){
      *position = 0;
      struct packet pp;

      packet_handler(&pp,packet);

      update_mouse_position(pp);

      mouse_event = mouse_event_detector(&pp, &event);

      mouse_actions_analyser(&mouse_event);

    } 
  } 
}

void update_mouse_position(struct packet pp){
  Terminix * terminix = get_current_terminix();
  terminix->mouse->new_x +=  pp.delta_x;

  if(terminix->mouse->new_x < 0){
    terminix->mouse->new_x = 0;
  }
  else if(terminix->mouse->new_x + (terminix->mouse->mouse_img).width > (terminix->mode_info).XResolution){
    terminix->mouse->new_x = (terminix->mode_info).XResolution - (terminix->mouse->mouse_img).width;
  }

  terminix->mouse->new_y -=  pp.delta_y;

  if(terminix->mouse->new_y < 0){
    terminix->mouse->new_y = 0;
  }
  else if(terminix->mouse->new_y + (terminix->mouse->mouse_img).height > (terminix->mode_info).YResolution){
    terminix->mouse->new_y = (terminix->mode_info).YResolution - (terminix->mouse->mouse_img).height;
  }
}


void (mouse_ih)(){

  //Checks if it has any parity or time out error
  if ((status_code & PARITY_BIT) >> 7 | (status_code & TIME_OUT_BIT) >> 6)
  {
    //if it has, it should discard de byte
    u_int8_t disposable_byte;
    if(util_sys_inb(OUT_BUF_REG, &disposable_byte)){
      printf("Error reading disposable byte.\n");
    }
    read_mouse=false;
    printf("Parity, Timeout Error or Keyboard.\n");
  }
  else{
    read_mouse=true;
    if(util_sys_inb(OUT_BUF_REG, &packet_byte)){
      printf("Error reading scan code.\n");
    }
  }
}

 int mouse_subscribe(uint8_t *bit_no){
   *bit_no = hook_id; 
    if(sys_irqsetpolicy(MOUSE_IRQ ,IRQ_REENABLE |IRQ_EXCLUSIVE, &hook_id)){
      printf("Error subscribing notification.\n");
      return 1;
    }
  return 0;
 }

  int mouse_unsubscribe(){
    if(sys_irqrmpolicy(&hook_id)){
      printf("Error unsubscribing notification.\n");
      return 1;
    }
  return 0;
  }

   void packet_byte_handler(int *position, uint8_t bytes[3]){
    //If it is the first byte and it doens't have bit 3 set to 1 then it can't be in position 0
    if ((*position) == 0 && !(packet_byte & BIT(3))){
      return;
    }
    //Else just save the packet byte into the bytes and add 1 to the positon               
    bytes[(*position)] = packet_byte;
    (*position)+=1;
          
  }
 
  void packet_handler(struct packet *pp, uint8_t packet[3]){
    //Save the packet bytes
    pp->bytes[0] = packet[0];
    pp->bytes[1] = packet[1];
    pp->bytes[2] = packet[2];
    //Save Overflow Values
    pp->x_ov = (packet[0] & BIT(6)) >> 6;
    pp->y_ov = (packet[0] & BIT(7)) >> 7;
    //Save Mouse Button Click
    pp->lb = packet[0] & BIT(0);
    pp->rb = (packet[0] & BIT(1)) >> 1;
    pp->mb = (packet[0] & BIT(2)) >> 2;
    //Save Mouse Movement on the X axis
    if(packet[0] & MSB_X_DELTA_BIT){
      pp->delta_x = (packet[1] | EXTEND_8_TO_16);
    }else{
      pp->delta_x = packet[1];
    }
    //Save Mouse Movement on the Y axis
    if(packet[0] & MSB_Y_DELTA_BIT){
      pp->delta_y = (packet[2] | EXTEND_8_TO_16);
    }else{
      pp->delta_y = packet[2];
    }
  }

  int mouse_send_command(uint8_t command){
    uint8_t ack_byte;
    while(true){
      if(check_if_IBF_full()){
        continue;
      }
      
      //Send the Write Command
      sys_outb(KBC_COMMAND_REG, WRITE_BYTE_TO_MOUSE);
      if(check_if_IBF_full()){
        continue;
      }
      //Sends the Command Argument
      sys_outb(INPUT_BUF_REG, command);
      //Checks the ACK byte
      util_sys_inb(OUT_BUF_REG,&ack_byte);
      if (ack_byte != ACK)
      {
        continue;
      }
      return 0;
    }
    return 1;
}

void mouse_enable_interrupts(){
  sys_irqenable(&hook_id);
}

void mouse_disable_interrupts(){
  sys_irqdisable(&hook_id);
}

int mouse_flush_OBF(){
  uint8_t byte_To_flush;
    //Reads the status code
    if(util_sys_inb(STAT_REG, &status_code)){
      printf("Error reading status code."); 
      return 1;                 
    }
    
   //Checks if the OBF is full and if it is, cleans it
   if(status_code & OBF_BIT) {
     if(util_sys_inb(OUT_BUF_REG, &byte_To_flush)){
      printf("Error reading scan code.");
      return 1;
    }
  } 
   return 0;
}

int reset_KBC_command_byte(uint8_t byte){
    while(true){
      if(check_if_IBF_full()){
        continue;
      }
      
      //Send the Command
      sys_outb(KBC_COMMAND_REG, WRITE_COMMAND_BYTE);
      if(check_if_IBF_full()){
        continue;
      }
      //Sends the Command Byte
      sys_outb(INPUT_BUF_REG, byte);     
      return 0;
    }
    return 1;
}

int check_if_IBF_full(){
  
  uint8_t status;
  util_sys_inb(STAT_REG, &status);
  //The IBF is full if the IBF_BIT is set to 1 in the status
  if(status & IBF_BIT){
    return 1;
  }
  return 0;
}

 void mouse_actions_analyser(struct mouse_ev * mouse_event){

   Terminix * terminix = get_current_terminix();


   if (mouse_event->type == LB_PRESSED){

     if(terminix->hero->bullet == NULL){
      terminix->hero->bullet = shoot(terminix->hero);
     }
   }
   if (mouse_event->type == RB_PRESSED){
     
   }
   if (mouse_event->type == LB_RELEASED){
     
   }
   if (mouse_event->type == RB_RELEASED){
     
   }
   if (mouse_event->type == MOUSE_MOV){
     
   }
 }

struct mouse_ev mouse_event_detector(struct packet *pp, uint8_t *event)
{

  static struct packet previous_packet;
  struct mouse_ev mouse_event;
  enum mouse_ev_t action;

  //Press LB
  if (previous_packet.lb == 0 && pp->lb == 1 && pp->rb == 0 && pp->mb == 0)
  {
    action = LB_PRESSED;
    mouse_event.type = action;
    mouse_event.delta_x = 0;
    mouse_event.delta_y = 0;
    *event |= MOUSE_EVENT;
  }
    
  //Press RB
  else if (previous_packet.rb == 0 && pp->rb == 1 && pp->lb == 0 && pp->mb == 0)
  {
    action = RB_PRESSED;
    mouse_event.type = action;
    mouse_event.delta_x = 0;
    mouse_event.delta_y = 0;
    *event |= MOUSE_EVENT;

  }
  //Release LB
  else if (previous_packet.lb == 1 && pp->lb == 0 && pp->rb == 0 && pp->mb == 0)
  {
    action = LB_RELEASED;
    mouse_event.type = action;
    mouse_event.delta_x = 0;
    mouse_event.delta_y = 0;
    *event |= MOUSE_EVENT;

  }
  //Release RB
  else if (previous_packet.rb == 1 && pp->rb == 0 && pp->lb == 0 && pp->mb == 0)
  {
    action = RB_RELEASED;
    mouse_event.type = action;
    mouse_event.delta_x = 0;
    mouse_event.delta_y = 0;
    *event |= MOUSE_EVENT;
  }
  //Move mouse
  else if (pp->delta_x != 0 || pp->delta_y != 0)
  {
    action = MOUSE_MOV;
    mouse_event.type = action;
    mouse_event.delta_x = pp->delta_x;
    mouse_event.delta_y = pp->delta_y;
    *event |= MOUSE_EVENT;
  }
  //Unused mouse events
  else
  {
    action = BUTTON_EV;
    mouse_event.type = action;
    mouse_event.delta_x = 0;
    mouse_event.delta_y = 0;
  }

  previous_packet = *pp;
  return mouse_event;
}
