#obj-m := my_pwm_driver.o
#obj-m := IRQ_test.o
obj-m := DRV8825_driver.o

#obj-m += kpwm3.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

#obj-m += kpwm.o
