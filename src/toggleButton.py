import baseThing
import dispatcher
from gpiozero import Button

class ToggleButton(baseThing.Thing):
    def __init__(self):
        self.onCommands = []
        self.state = 0

    def setGpio(self, gpio):
        self.button = Button(gpio)        
        self.button. when_released = lambda: self.handleInterruption()

    def setReactTo(self, command):        
        self.reactTo = command
        self.onCommands.append(command)
        self.offCommand = command
    
    def setCombinedReactTo(self, command, onCommands):
        self.reactTo = command
        self.offCommand = command + ":off"
        for onCommand in onCommands.split(","):
            self.onCommands.append(onCommand + ":on")    
        
    def handleCommand(self, command):
        if command == self.reactTo + ":0":
            self.state = 0
        elif command == self.reactTo + ":1":
            self.state = 1

    def handleInterruption(self):
        print("handle interruption")
        if self.state == 0:
            for onCommand in self.onCommands:
                dispatcher.sendCommand(onCommand)
        else:
            dispatcher.sendCommand(self.offCommand)
