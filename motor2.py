# call pigpio deamon by sudo pigpiod
# end the deamon by sudo killall pigpiod
from time import sleep 
import pigpio

DIR = 20
STP = 21 
SWITCH = 16

pi = pigpio.pi() #Connect to pigpiod daemon

pi.set_mode(DIR,pigpio.OUTPUT)
pi.set_mode(STP, pigpio.OUTPUT)

pi.set_mode(SWITCH,pigpio.INPUT) # set up input switch
pi.set_pull_up_down(SWITCH,pigpio.PUD_UP)

MODE = (18,23,24)
RES = {'Full': (0,0,0),
        'Half':(1,0,0),
        '1/4': (0,1,0),
        '1/8': (1,1,0),
        '1/16': (0,0,1),
        '1/32': (1,0,1)
        }

for i in range(3):
    pi.write(MODE[i], RES['Full'][i])

#set duty cycle and freq
pi.set_PWM_dutycycle(STP,128)
pi.set_PWM_frequency(STP,500)

try: 
    while True:
        pi.write(DIR,pi.read(SWITCH)) # set direction
        sleep(.1)
except KeyboardInterrupt:
    print("\n Stopping PIGPIO and exiting...")
finally:
    pi.set_PWM_dutycycle(STP,0) # PWM off 
    pi.stop()
