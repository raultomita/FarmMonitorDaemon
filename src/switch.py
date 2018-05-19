import baseThing
import dispatcher

class Switch(baseThing.Thing):
    def __init__(self):
        self.state = 0

    def setLocation(self, location):
        self.location = location

    def setDisplay(self, display):
        self.display = display

    def setGpio(self, gpio):
        self.gpio = gpio

    def handleCommand(self, command):
        if command == self.id:
            print("toggle switch %s with gpio %d" % (self.id, self.gpio))
            self.state = (self.state + 1) % 2
            self.sendState() 
        elif command == self.id + ":on" or command == self.location + ":on" or command == "all:on":
            self.state = 1
            self.sendState() 
            print("turn on switch %s with gpio %d" % (self.id, self.gpio))
        elif command == self.id + ":off" or command == self.location + ":off" or command == "all:off":
            self.state = 0
            self.sendState() 
            print("turn off switch %s with gpio %d" % (self.id, self.gpio))
        elif command == "all:?":            
            print("query state")
            self.sendState()
    
    def initialize(self):                
        self.sendState()
    
    def sendState(self):
        dispatcher.sendCommand("%s:%d" % (self.id, self.state))