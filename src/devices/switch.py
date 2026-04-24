import baseThing
import dispatcher
import dataManager
import logging
from datetime import datetime
from gpiozero import OutputDevice

logger = logging.getLogger(__name__)

switchNotification = '{ "id": "%s", "type": "switch", "display":"%s", "location":"%s", "timeStamp": "%s", "state": "%d", "googleType": "%s" }'

class Switch(baseThing.Thing):
    def __init__(self):
        super().__init__()
        self.autoOffSeconds = 0
        self._turnedOnAt = None

    def setLocation(self, location):
        self.location = location

    def setDisplay(self, display):
        self.display = display

    def setGpio(self, gpio):
        self.output = OutputDevice(gpio)

    def setGoogleType(self, googleType):
        self.googleType = googleType

    def setAutoOff(self, seconds):
        """Set auto turn-off delay in seconds. 0 disables auto turn-off."""
        self.autoOffSeconds = seconds

    def _startAutoOff(self):
        if self.autoOffSeconds > 0 and self.output.value:
            self._turnedOnAt = datetime.now()
            logger.debug("auto-off for %s scheduled in %d seconds" % (self.id, self.autoOffSeconds))
        else:
            self._turnedOnAt = None

    def _cancelAutoOff(self):
        self._turnedOnAt = None

    def handleCommand(self, command):
        if command == "timer":
            if self._turnedOnAt is not None and (datetime.now() - self._turnedOnAt).total_seconds() >= self.autoOffSeconds:
                logger.debug("auto turn-off triggered for %s" % self.id)
                self._turnedOnAt = None
                self.output.off()
                self.sendState()
        elif command == self.id:
            logger.debug("toggle switch %s" % self.id)
            self.output.toggle()
            self.sendState()
            self._startAutoOff()
        elif command == self.id + ":on" or command == self.location + ":on" or command == "all:on":
            self.output.on()
            self.sendState()
            self._startAutoOff()
            logger.debug("turn on switch %s" % self.id)
        elif command == self.id + ":off" or command == self.location + ":off" or command == "all:off":
            self._cancelAutoOff()
            self.output.off()
            self.sendState() 
            logger.debug("turn off switch %s" % self.id)
        elif command == "all:?" or command == self.id + ":?" or command == self.location + ":?":           
            logger.debug("query state")
            self.sendState()
    
    def sendState(self):        
        dispatcher.sendCommand("%s:%d" % (self.id, int(self.output.value)))
        notification = switchNotification % (self.id, self.display, self.location, datetime.now().isoformat(), int(self.output.value), self.googleType)
        dataManager.enqueueGeneral('HSET', 'devices', self.id, notification)
        dataManager.enqueueNotification(notification)        