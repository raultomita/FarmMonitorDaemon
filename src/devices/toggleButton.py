import baseThing
import dispatcher
import logging
import datetime
from gpiozero import Button

logger = logging.getLogger(__name__)

class ToggleButton(baseThing.Thing):
    def __init__(self):
        self.onCommands = []
        self.state = 0
        self.reactToLongPressed = None
        self.startedAt = None

    def setGpio(self, gpio):
        self.button = Button(gpio)        
        self.button.when_released = lambda: self.handleButtonReleased()
        self.button.when_pressed = 

    def setReactTo(self, command):        
        self.reactTo = command
        self.onCommands.append(command + ":on")
        self.offCommand = command + ":off"
    
    def setCombinedReactTo(self, command, onCommands):
        self.reactTo = command
        self.offCommand = command + ":off"
        for onCommand in onCommands.split(","):
            self.onCommands.append(onCommand + ":on")    

    def setReactToLongPressed(self, command):
         self.reactToLongPressed = command

    def handleCommand(self, command):
        if command == self.reactTo + ":0":
            self.state = 0
        elif command == self.reactTo + ":1":
            self.state = 1

    def initialize(self):
        dispatcher.sendCommand(self.reactTo + ":?")

    def handleButtonPressed(self):
        logger.debug("Handle button pressed")
        self.startedAt = datetime.datetime.now()

    def handleButtonReleased(self):
        if(self.reactToLongPressed != None and (datetime.datetime.now() - self.startedAt).total_seconds())
            dispatcher.sendCommand(self.reactToLongPressed)
            logger.debug("Handle long pressed")
        else:
            logger.debug("Handle short pressed")
            if self.state == 0:
                for onCommand in self.onCommands:
                    dispatcher.sendCommand(onCommand)
            else:
                dispatcher.sendCommand(self.offCommand)