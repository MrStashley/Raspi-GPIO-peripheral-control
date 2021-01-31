/*

Written by: Cole Lashley

attempting to read the data pins on a rotary encoder and use it for control of another part

 */

#include <iostream>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include "servo_control_sysfs.h"



int init_gpio_pin(int pin_num){
  char pin_num_buf[3], direction_file_buf[33] = "/sys/class/gpio/gpio";
  int fd;
  
  if(pin_num < 0){
    perror("init_gpio_pin - invalid inputs");
    return -1;
  }

  fd = open("/sys/class/gpio/export", O_WRONLY);
  if(fd == -1){
    perror("init_gpio_pin - unable to open /sys/class/gpio/export");
    return -1;
  }

  sprintf(pin_num_buf, "%d", pin_num);

  if(write(fd, pin_num_buf, NUM_DIGITS_2(pin_num)) != NUM_DIGITS_2(pin_num)){
      perror("init_gpio_pin - unable to write to export");
      return -1;
  }

  close(fd);

  sprintf(&direction_file_buf[20], "%d/direction", pin_num);

  fd = open(direction_file_buf, O_WRONLY);
  if(fd == -1){
    perror("init_gpio_pin - unable to open direction file");
    return -1;
  }

  if(write(fd, "in", 2) != 2){
    perror("init_gpio_pin - unable to write to direction file");
    return -1;
  }

  close(fd);

  return 0;
  
}

int read_gpio_pin(int pin_num){
  int fd, pin_value = -1;
  char value_file_buf[29] = "/sys/class/gpio/gpio", value_read_buf;

  if(pin_num < 0){
    perror("read_gpio_pin - invalid inputs");
    return -1;
  }

  sprintf(&value_file_buf[20], "%d/value", pin_num);

  fd = open(value_file_buf, O_RDONLY);
  if(fd == -1){
    perror("read_gpio_pin- unable to open value file");
    return -1;
  }

  if(read(fd, &value_read_buf, 1) != 1){
    perror("read_gpio_pin - unable to read value file");
    return -1;
  }

  close(fd);

  return ((int)value_read_buf) - 48;
  
}

int close_gpio_pin(int pin_num){
  int fd;
  char pin_num_buf[3];

  fd = open("/sys/class/gpio/unexport", O_WRONLY);
  if(fd == -1){
    perror("close_gpio_pin - unable to open /sys/class/gpio/unexport");
    return -1;
  }

  sprintf(pin_num_buf, "%d", pin_num);

  if(write(fd, pin_num_buf, NUM_DIGITS_2(pin_num)) != NUM_DIGITS_2(pin_num)){
    perror("close_gpio_pin - unable to write to /sys/class/gpio/unexport");
    return -1;
  }

  close(fd);
  return 0;

}


void handle_sigint(int signal){
  close_gpio_pin(23);
  close_gpio_pin(24);
  exit(0);
}

void write_servo_val(PwmPin *servo_control, int servo_val){  
  if(servo_val < 0)
    servo_val = 0;
  else if(servo_val > 20)
    servo_val = 20;
  
  servo_control->write_pwm_value(servo_val);

}

int main(){
  int pin_val_23 = 1, pin_val_24 = 1;

  int servo_val = 0;

  init_gpio_pin(23);
  init_gpio_pin(24);

  PwmPin servo_control(17, 0, 20);
  servo_control.set_num_pulses(10);

  signal(SIGINT, handle_sigint);
	 
  while(1){
    int old_pin_val_23 = pin_val_23;
    int old_pin_val_24 = pin_val_24;

    pin_val_23 = read_gpio_pin(23);
    pin_val_24 = read_gpio_pin(24);

    if(pin_val_23 != old_pin_val_23){
      std::cout << "turned left: " << pin_val_23 << std::endl;
      servo_val++;
      write_servo_val(&servo_control, servo_val);
      while((pin_val_23 = read_gpio_pin(23)) != 1 || (pin_val_24 = read_gpio_pin(24)) != 1){}
    }

    if(pin_val_24 != old_pin_val_24){
      std::cout << "turned right: " << pin_val_24 << std::endl;
      servo_val--;
      write_servo_val(&servo_control, servo_val);
      while((pin_val_23 = read_gpio_pin(23)) != 1 || (pin_val_24 = read_gpio_pin(24)) != 1){}
    }


    if(servo_val < 0)
      servo_val = 0;
    else if(servo_val > 20)
      servo_val = 20;


  }

}
