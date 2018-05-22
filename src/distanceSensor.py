import baseThing
import dispatcher
import logging
from gpiozero import LineSensor

logger = logging.getLogger(__name__)

class DistanceSensor(baseThing.Thing):
    def __init__(self):
        self.onlyOff = True
        self.state = 0

    def setGpio(self, gpio):
        self.line = LineSensor(gpio)   
        self.line.when_line = lambda: self.handleInterruption()        

    def setReactTo(self, command):        
        self.reactTo = command
    
    def makeToggle(self):
        self.onlyOff = False

    def handleCommand(self, command):
        if command == self.reactTo + ":0":
            self.state = 0
        elif command == self.reactTo + ":1":
            self.state = 1

    def initialize(self):
        dispatcher.sendCommand(self.reactTo + ":?")

    def handleInterruption(self):
        logger.debug("handle interruption")
        if self.onlyOff:
            dispatcher.sendCommand(f"{self.reactTo}:off")
        else:   
            if self.state == 0:
                dispatcher.sendCommand(f"{self.reactTo}:on")
            else:
                dispatcher.sendCommand(f"{self.reactTo}:off")
