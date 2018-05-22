import baseThing
import datetime
import logging
import dispatcher

logger = logging.getLogger(__name__)

class AutomaticTrigger(baseThing.Thing):
    def __init__(self):
        self.startedAt = None
        self.commandSent = False

    def setTargetDeviceId(self, deviceId):
        self.targetDeviceId = deviceId       
    
    def setListenOn(self, deviceId):
        self.turnedOn = f"{deviceId}:1"
        self.turnedOff = f"{deviceId}:0"
   
    def handleCommand(self, command):
        if command == self.turnedOn:
            self.schedule()
        elif command == self.turnedOff:
            self.turnOff()
        elif command == "timer" and self.isScheduled():
            self.turnOn()
    
    def schedule(self):
        if self.startedAt == None:
            self.commandSent = False
            self.startedAt = datetime.datetime.now()
            logger.debug(f"Schedule for {self.id} started at {self.startedAt}")

    def isScheduled(self):
        return self.startedAt != None and not self.commandSent

    def turnOff(self):
        self.startedAt = None
        dispatcher.sendCommand(f"{self.targetDeviceId}:off")
        logger.debug(F"send command for turning off for {self.targetDeviceId}")

    def turnOn(self):
        elapsedSeconds = (datetime.datetime.now() - self.startedAt).total_seconds()
        if elapsedSeconds > 210:
            dispatcher.sendCommand(f"{self.targetDeviceId}:on")
            logger.debug(F"send command for turning on for {self.targetDeviceId}")
            self.commandSent = True
        else:
            logger.debug(F"There is time {elapsedSeconds} or command is sent {self.commandSent}")
            
                
