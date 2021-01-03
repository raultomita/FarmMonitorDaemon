import baseThing
import threading 
import dispatcher
import logging
from gpiozero import PWMOutputDevice,OutputDevice,DistanceSensor,LineSensor
import re
import VL53L1X
import uuid

MAX_DISTANCE = 3200
MIN_DISTANCE = 300
DOORDISTANCE = 2000

OPENED = 1
BEFORE_MIDDLE = 2
AFTER_MIDDLE = 3
BLOCKED = 4
CLOSED = 5

NONE = 0

OPENING = 1
CLOSING = 2

logger = logging.getLogger(__name__)

class CurtainEngine():
    def __init__(self):
        self.speed = PWMOutputDevice(18)
        self.direction = OutputDevice(17)
        self.speed.value = 0
    
    def open(self):
        self.direction.off()
        self.speed.value= 0.1
    
    def close(self):
        self.direction.on()
        self.speed.value= 0.1
    
    def stop(self):
        self.speed.value= 0

    def state(self):
        if self.speed.value > 0 and self.direction.value == 1:
            return CLOSING
        elif self.speed.value > 0 and self.direction.value == 0:
            return OPENING        
        return NONE

    def isRunning(self):
        return self.speed.value > 0

class CurtainSensor():
    def __init__(self, address):        
        self.sensor = VL53L1X.VL53L1X(i2c_bus=1, i2c_address=0x29) 
        self.sensor.open()   
        self.sensor.change_address(new_address = address)
        self.sensor.open()        

    def getDistance(self):
        self.sensor.start_ranging(1) #1 short 120 cm, 2 medium 200 cm, 3 large 400 cm 
        distance = self.sensor.get_distance()
        self.sensor.stop_ranging()  

        return distance
        
class Curtain():
    def __init__(self):   
        self.middle = CurtainSensor(address=0x20)          
        #self.right = CurtainSensor(address=0x21)       
        #self.left = CurtainSensor(address=0x22)       

        self.doorSensor = LineSensor(4)

    def state(self):       
        middleDistance = self.middle.getDistance()        
        rightDistance =1234    
        leftDistance = 5
        logger.info("Curtain right is %f and middle %f and door %f", rightDistance, middleDistance, self.doorSensor.value)


        if rightDistance > 400:
            return OPENED
        elif middleDistance >= 400:
            return BEFORE_MIDDLE
        # elif leftSensorDistance < 400 and doorSensor.value == 1:
        #     return CLOSED
        # elif leftSensorDistance < 400 and doorSensor.value != 1:
        #     return BLOCKED
        elif middleDistance < 400 and self.doorSensor.value == 1:
            return AFTER_MIDDLE
        elif middleDistance < 400 and self.doorSensor.value >= 1:
            return BLOCKED
        
        return NONE

class CurtainController(baseThing.Thing):
    def __init__(self):
        self.matcher = re.compile("(curtain[0-9]*):([0-9]{1,3})")       
        self.engine = CurtainEngine()        
        self.curtain = Curtain()

        self.state = NONE

    def handleCommand(self, command):
        if command == "timer":
            self.handleTimerCommand()
        elif command == "%s:open" % self.id:
             self.handleOpenCommand()
        elif command == "%s:close" % self.id:
            self.handleCloseCommand()
        elif command == "%s:stop" % self.id:
            logger.debug("curtain stopped %s" % self.id)
            self.engine.stop()
            self.state = self.curtain.state()

    def initialize(self):  
        # testare cand se ia curentul, sau crapa serviciul     
        self.state = self.curtain.state()

    def handleTimerCommand(self):    
        logger.debug("curtain time command")    
        self.state = self.curtain.state()
        if self.engine.isRunning() and self.curtain.state() != self.state:
            self.engine.stop()
            self.state = self.curtain.state()
        
    def handleOpenCommand(self):
        logger.info("curtain opening %s" % self.id)
        self.engine.open()
    def handleCloseCommand(self):
        logger.info("curtain closing %s" % self.id)
        self.engine.close()