import baseThing

class Led(baseThing.Thing):
    def setGpio(self, gpio):
        self.gpio = gpio
    
    def setReactTo(self, command):
        self.onCommand = command + ":1"
        self.offCommand = command + ":0"

    def handleCommand(Self, command):
        if self.onCommand = command:
            pass
        elif self.offCommand = command:
            pass