#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <linux/interrupt.h>                    // to add IRQ_interrupt requests 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TSY");

#define STP         21 
#define m0_pin      18
#define m1_pin      23
#define m2_pin      24
#define DIR         20 
#define gpioButton  16

static int duty_cycle   = 50;                   //%
static int freq         = 500;                  //Hz
static int enable       = 1;                    // echo to parameter file to turn off
static bool m0          = 0;                    // motor mode
static bool m1          = 0;
static bool m2          = 0;

static unsigned int irqNumber;          
static unsigned int numberPresses = 0; 
static int DIR_switch = 0;

static irq_handler_t  gpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs);

module_param(duty_cycle, int, 0644);           //expose thru sysfs 
module_param(freq, int, 0644);
module_param(enable, int, 0644);
module_param(m0, bool, 0644);
module_param(m1, bool, 0644);
module_param(m2, bool, 0644);


static __init int pwm_gpio_init(void){
    int result =0;

    printk(KERN_INFO "PWM module starting %s. \n", __FUNCTION__);
    gpio_request(STP, "STP");
    gpio_direction_output(STP,0);
    gpio_request(m1_pin, "m1_pin");
    gpio_direction_output(m1_pin,0);
    gpio_request(m0_pin, "m0_pin");
    gpio_direction_output(m0_pin,0);
    gpio_request(m2_pin, "m2_pin");
    gpio_direction_output(m2_pin,0);
    gpio_request(DIR, "DIR");
    gpio_direction_output(DIR,DIR_switch);

    gpio_request(gpioButton, "sysfs"); 
    gpio_direction_input(gpioButton);
    gpio_set_debounce(gpioButton, 400);
    gpio_export(gpioButton, false);

    irqNumber = gpio_to_irq(gpioButton);
    printk(KERN_INFO "GPIO_TEST: The button is mapped to IRQ: %d\n", irqNumber);

    // request an interrupt line
     result = request_irq(irqNumber,            
                        (irq_handler_t) gpio_irq_handler, 
                        IRQF_TRIGGER_RISING,   // Interrupt on rising edge 
                        "gpio_handler",    
                        NULL);                
    return result;
}

static __exit void pwm_gpio_exit(void){
    printk(KERN_INFO "PWM module stopping %s. \n", __FUNCTION__);
    gpio_free(STP);
    gpio_free(m0_pin);
    gpio_free(m1_pin);
    gpio_free(m2_pin);
    gpio_free(DIR);
    free_irq(irqNumber,NULL);
    gpio_unexport(gpioButton);
    gpio_free(gpioButton);
}

static int pwm_run_init(void){
    int tusec_On;                           // on period in micro sec
    int tusec_Off;                          // off period in micro sec
    printk(KERN_ALERT "PWM ON: %dMHz and %d percent. \n", freq, duty_cycle);

    gpio_set_value(m0_pin, m0);
    gpio_set_value(m1_pin, m1);
    gpio_set_value(m2_pin, m2);
    gpio_set_value(DIR, 0);

    while (enable)
    {
        tusec_On = (1000000*duty_cycle)/(freq*100);
        tusec_Off = (1000000*(100-duty_cycle))/(freq*100);
        gpio_set_value(STP, 1);
        usleep_range(tusec_On, tusec_On);
        gpio_set_value(STP, 0);
        usleep_range(tusec_Off, tusec_Off);
    }

    return 0;
}

static irq_handler_t gpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs){
   numberPresses++;
   DIR_switch = gpio_get_value(gpioButton);
   printk(KERN_INFO "GPIO_TEST: Interrupt! (button state is %d) and number of press is %d\n", gpio_get_value(gpioButton), numberPresses);                    
   return (irq_handler_t) IRQ_HANDLED;      // Announce that the IRQ has been handled correctly
}

static void pwm_run_exit(void){
    printk(KERN_ALERT "PWM OFF. \n");
}

/*******Module Init and Exit Func***********/

static __init int pwm_driver_init(void){
    printk(KERN_INFO "my_pwm_driver is loaded. \n");
    pwm_gpio_init();
    pwm_run_init();
    return 0;
}

static __exit void pwm_driver_exit(void){
    printk(KERN_INFO "my_pwm_driver is unloaded. \n");
    pwm_run_exit();
    pwm_gpio_exit();
}

module_init(pwm_driver_init);
module_exit(pwm_driver_exit);


//sudo tail -f /var/log/syslog
//echo 0 | sudo tee /sys/module/my_pwm_driver/parameters/enable