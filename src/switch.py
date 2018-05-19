import baseThing
import dispatcher
import redisManager
from gpiozero import OutputDevice

switchNotification = '{ "id": "%s", "type": "switch", "display":"%s", "location":"%s", "timeStamp": "%s", "state": "%d" }'
class Switch(baseThing.Thing):
    def setLocation(self, location):
        self.location = location

    def setDisplay(self, display):
        self.display = display

    def setGpio(self, gpio):
        self.output = OutputDevice(gpio)

    def handleCommand(self, command):
        if command == self.id:
            print("toggle switch %s" % self.id)
            self.output.toggle()
            self.sendState() 
            self.sendNotification()
        elif command == self.id + ":on" or command == self.location + ":on" or command == "all:on":
            self.output.on()
            self.sendState() 
            self.sendNotification()
            print("turn on switch %s" % self.id)
        elif command == self.id + ":off" or command == self.location + ":off" or command == "all:off":
            self.output.off()
            self.sendState() 
            self.sendNotification()
            print("turn off switch %s" % self.id)
        elif command == "all:?":            
            print("query state")
            self.sendState()
    
    def initialize(self):
        self.sendState()
    
    def sendState(self):        
        dispatcher.sendCommand("%s:%d" % (self.id, int(self.output.value)))
            
    def sendNotification(self):       
        notification = switchNotification % (self.id, self.display, self.location, "na", int(self.output.value))
        redisManager.hset("devices", self.id, notification)
        redisManager.publishNotification(notification)