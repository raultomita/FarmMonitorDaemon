import baseThing
import logging
from gpiozero import PWMOutputDevice,OutputDevice
import re

OPENED = 3
HALFCLOSED = 2
CLOSED = 1

logger = logging.getLogger(__name__)

class CurtainController(baseThing.Thing):
    def __init__(self):
        self.matcher = re.compile('(cc[0-9]*):([oc])([0-9]{1,3})')
        self.state = 0
        self.direction = 0
        self.speed = PWMOutputDevice(12)
        self.direction = OutputDevice(26)

    def handleCommand(self, command):
        result = self.matcher.fullmatch(command)
        if result != None and result.group(1) == self.id:            
            if result.group(2) == 'o':
                self.direction.on()            
            else:
                self.direction.off()

            state = int(result.group(3))
            if state > 100:
                state = 100
            elif state < 0:
                state = 0

            self.speed.value = state / 100
            
            logger.info("set speed %s for %s" % (state, self.id))