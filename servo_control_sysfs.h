/*
  Written by: Cole Lashley
  
  attempting to use the low level sysfs interface to program a software driven pwm

*/
#ifndef SERVO_CONTROL_SYSFS_H
#define SERVO_CONTROL_SYSFS_H
#include <iostream>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#define NUM_DIGITS_2(num) (1 + ((num) >= 10))
#define CALC_VAL_MICROS(num, min, max) ((((num) * (2000)) / ((max)-(min)))  - (min))

class PwmPin{
private:
  int pin_num;
  int ctrl_min;
  int ctrl_max;
  int num_pulses;
  
  int init_pwm_pin(int pctrl_min, int pctrl_max){
     int cur_pin, fd;
     char pin_num_buf[3]; /* buffer for writing pin_num to file) */
     char direction_file_buf[33] = "/sys/class/gpio/gpio"; 

     if(pctrl_min >= pctrl_max){
       perror("init_pwm_pin - illegal inputs");
       return -1;
     }

     this->ctrl_min = pctrl_min;
     this->ctrl_max = pctrl_max;
  

     cur_pin = this->pin_num;
  
     if(cur_pin < 0){
       perror("init_pwm_pin - illegal inputs");
       return -1;
     }

     /* this is needed to initialize the pin 
	we are telling the os we want to use this pin num */
     fd = open("/sys/class/gpio/export", O_WRONLY);
     if(fd == -1){
       perror("init_pwm_pin - unable to open /sys/class/gpio/export");
       return -1;
     }

     sprintf(pin_num_buf, "%d", cur_pin);

     /*write the pin number to the file */
     if(write(fd, pin_num_buf, NUM_DIGITS_2(cur_pin)) != NUM_DIGITS_2(cur_pin)){
       perror("init_pwm_pin - write failed");
       return -1;
     }

     close(fd);

     /*set the pin to output by writing "out" to /sys/class/gpio/gpio[pin_num]/direction*/
  
     sprintf(&direction_file_buf[20], "%d/direction", cur_pin);
  
     fd = open(direction_file_buf, O_WRONLY);
     if(fd == -1){
       perror("init_pwm_pin - unable to open direction file");
       return -1;
     }

     if(write(fd, "out", 3) != 3){
       perror("init_pwm_pin - unable to write to direction file");
       return -1;
     }

     close(fd);

     return 0;

  }

  int close_pwm_pin(){
      int fd, cur_pin;
      char pin_num_buf[3]; 

      cur_pin = this->pin_num;

      fd = open("/sys/class/gpio/unexport", O_WRONLY);
      if(fd == -1){
	perror("close_pwm_pin - Unable to open /sys/class/gpio/unexport");
	return -1;
      }

      sprintf(pin_num_buf, "%d", cur_pin);

      if(write(fd, pin_num_buf, NUM_DIGITS_2(cur_pin)) != NUM_DIGITS_2(cur_pin)){
	perror("close_pwm_pin - Unable to write to /sys/class/gpio/unexport");
	return -1;
      }

      close(fd);

      return 0;

  }

public:
  PwmPin(int ppin_num, int ctrl_min, int ctrl_max){
    this->pin_num = ppin_num;
    this->num_pulses = 50;
    init_pwm_pin(ctrl_min, ctrl_max);
  }

  void set_num_pulses(int num_pulses){
    this->num_pulses = num_pulses;
  }
  
  int write_pwm_value( int value){
    int fd, cur_pin, on_micros = 0, off_micros = 20000, i = 0;
    char value_file_buf[29] = "/sys/class/gpio/gpio";

    if(value < this->ctrl_min || value > this->ctrl_max){
      perror("write_pwm_value - invalid inputs");
      return -1;
    }

    cur_pin = this->pin_num;
  
    on_micros = 500 + CALC_VAL_MICROS(value, this->ctrl_min, this->ctrl_max);
    /*pin must be on for between 1 and 2 milliseconds, then off for the rest of the time, 
      or 20 ms */


    printf("%d\n", on_micros);
  

    off_micros = 20000 - on_micros;

    /*we must calculate the value here in micros so that we don't have to do any intermediate calculations when the servo is running, this will hopefully make the pwm more accurate */

    sprintf(&value_file_buf[20], "%d/value", cur_pin);

    printf("%d, %d\n", on_micros, off_micros);
  
    fd = open(value_file_buf, O_WRONLY);
    if(fd == -1){
      perror("write_pwm_value - unable to open value file");
      return -1;
    }

    for(i = 0;i < this->num_pulses;i++){
      /*write on */
      if(write(fd, "1", 1) != 1){
	perror("write_pwm_pin - error writing 1 to value");
	return -1;
      }
      /*wait till it needs to be turned off */
      usleep(on_micros);

      /*write off */
      if(write(fd, "0", 1) != 1){
	perror("write_pwm_pin - error writing 0 to value");
	return -1;
      }
      /* wait till it needs to be turned back on */
      usleep(off_micros);
    }
  
    close(fd);
    return 0;
  }

  ~PwmPin(){
    close_pwm_pin();
  }

  

};


#endif
