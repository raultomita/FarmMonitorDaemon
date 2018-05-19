import baseThing
import dispatcher
from gpiozero import LED

class Led(baseThing.Thing):
    def __init__(self):
        self.ledOff = None

    def setGpio(self, gpio):
        self.led = LED(gpio)
    
    def setGpioOff(self, gpio):
        self.ledOff = LED(gpio)

    def setReactTo(self, command):
        self.command = command

    def handleCommand(self, command):
        if self.command + ":1" == command:
            print("switch led off")
            self.led.off()
            if self.ledOff != None:
                self.ledOff.on()

        elif self.command + ":0" == command:
            print("switch led on")
            self.led.on()
            if self.ledOff != None:
                self.ledOff.off()
        else:
            print("led for command %s not found" % command)

    def initialize(self):
        dispatcher.sendCommand(self.command + ":?")
