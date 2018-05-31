import baseThing
import dispatcher
import redisConn
import logging
from datetime import datetime
from gpiozero import OutputDevice

logger = logging.getLogger(__name__)

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
            logger.debug("toggle switch %s" % self.id)
            self.output.toggle()
            self.sendState() 
        elif command == self.id + ":on" or command == self.location + ":on" or command == "all:on":
            self.output.on()
            self.sendState() 
            logger.debug("turn on switch %s" % self.id)
        elif command == self.id + ":off" or command == self.location + ":off" or command == "all:off":
            self.output.off()
            self.sendState() 
            logger.debug("turn off switch %s" % self.id)
        elif command == "all:?" or command == self.id + ":?" or command == self.location + ":?":           
            logger.debug("query state")
            self.sendState()
    
    def sendState(self):        
        dispatcher.sendCommand("%s:%d" % (self.id, int(self.output.value)))
        notification = switchNotification % (self.id, self.display, self.location, datetime.now().isoformat(), int(self.output.value))
        redisConn.enqueueGeneral('HSET', 'devices', self.id, notification)
        redisConn.enqueueNotification(notification)        