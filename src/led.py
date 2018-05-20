import baseThing
import dispatcher
import logging
from gpiozero import LED

logger = logging.getLogger(__name__)

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
            logger.debug("switch led off")
            self.led.off()
            if self.ledOff != None:
                self.ledOff.on()

        elif self.command + ":0" == command:
            logger.debug("switch led on")
            self.led.on()
            if self.ledOff != None:
                self.ledOff.off()