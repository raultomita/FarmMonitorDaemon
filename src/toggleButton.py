import baseThing
import dispatcher

class ToggleButton(baseThing.Thing):
    def __init__(self):
        self.onCommands = []
        self.state = 0

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
        if command == reactTo + ":0":
            self.state = 0
        elif command == reactTo + ":1":
            self.state = 1

    def handleInteruption(self):
        if self.state == 0:
            for onCommand in self.onCommands:
                dispatcher.sendCommand(onCommand)
        else:
            dispatcher.sendCommand(self.offCommand)
