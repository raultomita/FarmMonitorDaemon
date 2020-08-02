import baseThing
import threading 
import dispatcher
import logging
from gpiozero import PWMOutputDevice,OutputDevice,DistanceSensor
import re
import VL53L1X
import uuid

MAX_DISTANCE = 3200
MIN_DISTANCE = 300
DOORDISTANCE = 2000

logger = logging.getLogger(__name__)

class CurtainController(baseThing.Thing):
    def __init__(self):
        self.matcher = re.compile("(cc[0-9]*):([0-9]{1,3})")
        self.currentDistance = 0       
        self.stamp = str(uuid.uuid4())

        self.speedController = PWMOutputDevice(24)
        self.directionController = OutputDevice(0)
        self.distanceSensor = VL53L1X.VL53L1X(i2c_bus=1, i2c_address=0x29)
        self.distanceSensor.open()

    def handleCommand(self, command):        
        self.handleExecuteCommand(command)
        self.handleTimerCommand(command)        
        self.handleInProgressCommand(command)

    def initialize(self):       
       self.readDistance()

    def readDistance(self):
        self.distanceSensor.start_ranging(3)  
        self.currentDistance = self.distanceSensor.get_distance()
        self.distanceSensor.stop_ranging()
        logger.info("distance for %s is %s" % (self.id, self.currentDistance)) 
    
    def handleTimerCommand(self, command):
        if command == "timer" and self.speedController.value == 0:   
            previousDistance = self.currentDistance
            self.readDistance()
            moveDetected = abs(previousDistance - self.currentDistance) > 30
            
            # if moveDetected and previousDistance > self.currentDistance:
            #      dispatcher.enqueueCommand("%s:100" % self.id)
            # elif moveDetected and previousDistance < self.currentDistance:
            #      dispatcher.enqueueCommand("%s:0" % self.id)
    
    def handleExecuteCommand(self, command):
        result = self.matcher.fullmatch(command)
        if result != None and result.group(1) == self.id:            
            self.readDistance()
            percent = int(result.group(2))
            if percent > 100:
                percent = 100
            elif percent < 0:
                percent = 0

            self.destination = MAX_DISTANCE * percent/100 
            self.stamp = str(uuid.uuid4())
            if self.destination > self.currentDistance:                
                dispatcher.enqueueCommand("%s:%s-close" % (self.id, self.stamp))            
            else:
                dispatcher.enqueueCommand("%s:%s-open" % (self.id, self.stamp))
    
    def handleInProgressCommand(self, command):
        if command == "%s:stop" % self.id:
            self.speedController.value = 0
            self.readDistance() 
            self.stamp = str(uuid.uuid4())
            logger.info("curtain stop") 
            
        elif command == "%s:%s-close" % (self.id, self.stamp):
            self.directionController.on()
            self.speedController.value = 1
            self.readDistance()
            logger.info("curtain started") 
            if self.currentDistance > self.destination:            
                self.speedController.value = 0
                self.readDistance()
            else:
                dispatcher.enqueueCommand(command)            

        elif command == "%s:%s-open" % (self.id, self.stamp):
            self.directionController.off()
            self.speedController.value = 1
            self.readDistance()
            logger.info("curtain started")
            if self.currentDistance < self.destination:            
                self.speedController.value = 0
                self.readDistance() 
            else:
                dispatcher.enqueueCommand(command)        
        
    