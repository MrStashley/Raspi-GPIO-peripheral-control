#ifndef GPIO_UTILS_H
#define GPIO_UTILS_H
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

class GpioPin{

enum pin_dir {
    INPUT,
    OUTPUT,
};

private: 
    int pin_num;
    pin_dir dir; 

    int init_gpio_pin(int pin_num, char *dir){
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

        if(write(fd, dir, strnlen(dir, 4)) != strnlen(dir,4)){
            perror("init_gpio_pin - unable to write to direction file");
            return -1;
        }

        close(fd);

        return 0;
        
    } 

    int close_gpio_pin(){
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

    int read_gpio_pin(){
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

    int write_gpio_pin(int value){
        int fd, pin_value = -1;
        char value_file_buf[29] = "/sys/class/gpio/gpio", value_write_buf;

        value_write_buf = value + 48;

        if(pin_num < 0){
            perror("read_gpio_pin - invalid inputs");
            return -1;
        }

        sprintf(&value_file_buf[20], "%d/value", pin_num);

        fd = open(value_file_buf, O_WRONLY);
        if(fd == -1){
            perror("read_gpio_pin- unable to open value file");
            return -1;
        }

        if(read(fd, &value_write_buf, 1) != 1){
            perror("read_gpio_pin - unable to read value file");
            return -1;
        }

        close(fd);

        return 0;
    }

    int set_pin_dir(char *dir){
        direction_file_buf[33] = "/sys/class/gpio/gpio";
        int fd;

        sprintf(&direction_file_buf[20], "%d/direction", pin_num);

        fd = open(direction_file_buf, O_WRONLY);
        if(fd == -1){
            perror("set_pin_dir - unable to open direction file");
            return -1;
        }

        if(write(fd, dir, strnlen(dir, 4)) != strnlen(dir,4)){
            perror("set_pin_dir - unable to write to direction file");
            return -1;
        }

        close(fd);
        return 0;
    }

public: 
    GpioPin(int pin_num, pin_dir dir): pin_num(pin_num), dir(dir) {
        string dir_str; 
        if(dir == INPUT){
            dir_str = "in";
        } else if (dir == OUTPUT){
            dir_str = "out";
        } else {
            std::cout << "invalid direction in constructor for pin " << pin_num << std::endl;
            return;
        }

        init_gpio_pin(pin_num, dir_str.c_str());
    }

    int set_pin_mode(pin_dir dir){
        this->dir = dir;
        string dir_str;
        if(dir == INPUT){
            dir_str = "in";
        } else if (dir == OUTPUT){
            dir_str = "out";
        } else {
            std::cout << "set_pin_mode - invalid direction" << std::endl;
            return -1;
        }

        return set_pin_dir(dir_str.c_str());
    }

    int read(){
        if(dir == INPUT)
            return read_gpio_pin();
        else{
            std::cout << "read - pin not set to input mode " << std::endl;
            return -1;
        }
    }

    int write(int value){
        if(dir != OUTPUT){
            std::cout << "write_gpio_pin - pin is not in output mode " << std::endl;
            return -1;
        }
        else if(value == 0 || value == 1){
            return write_gpio_pin(value);
        } else {
            std::cout << "write_gpio_pin - can't write a non boolean value to a digital pin " << std::endl;
            return -1;
        }
    }

    ~GpioPin(){
        close_gpio_pin();
    }
};


#endif