import queue
import threading 
import redisConn
import logging

from devices.switch import Switch
from devices.toggleButton import ToggleButton
from devices.led import Led
from devices.stateManager import StateManager
from devices.automaticTrigger import AutomaticTrigger
from devices.distanceSensor import DistanceSensor
from devices.heartbeat import *

logger = logging.getLogger(__name__)

receivedCommandsQueue = queue.Queue()

def enqueueCommand(command):
    receivedCommandsQueue.put(command)    
    logger.info("Command received %s" % command)

def sendCommand(command):
    #throttle commands
    logger.debug("send command: %s" % command)
    enqueueCommand(command)
    redisConn.enqueueCommand(command)

class DispatcherThread(threading.Thread):
    def __init__(self):
        super(DispatcherThread, self).__init__()
        self.daemon = True
        self.devices = []

    def run(self):
        self.addDevices()
        self.initializeSystem()

        logger.debug("Before handling commands they are %d" % receivedCommandsQueue.qsize())

        while True:
            logger.debug("Listen for items in queue")
            command = receivedCommandsQueue.get()
            logger.debug("command %s dequeued" % command)
        
            self.handleCommand(command)

    def handleCommand(self, command):
        for device in self.devices:
            device.handleCommand(command)

    def addDevices(self):
        devices = redisConn.readDevices()            
        for device in devices:
            self.addDevice(device)
        
        self.addSystemDevices()

        logger.info("Added %d devices from %d received", len(self.devices), len(devices))

    def addDevice(self, rawDevice):
        logger.debug(rawDevice)
        newDevice = None
        
        if rawDevice[b"type"] == b"switch":
            newDevice = Switch()            
            newDevice.setLocation(rawDevice[b"location"].decode())
            newDevice.setDisplay(rawDevice[b"display"].decode())
            newDevice.setGpio(int(rawDevice[b"gpio"]))

        elif rawDevice[b"type"] == b"toggleButton":           
            newDevice = ToggleButton()            
            newDevice.setGpio(int(rawDevice[b"gpio"]))
            if b"commands4On" in rawDevice:
                newDevice.setCombinedReactTo(rawDevice[b"targetDeviceId"].decode(), rawDevice[b"commands4On"].decode())
            else:
                newDevice.setReactTo(rawDevice[b"targetDeviceId"].decode())

        elif rawDevice[b"type"] == b"led":            
            newDevice = Led()
            newDevice.setReactTo(rawDevice[b"listenTo"].decode())
            newDevice.setGpio(int(rawDevice[b"gpio"]))

            if b"gpioOff" in rawDevice:
                newDevice.setGpioOff(int(rawDevice[b"gpioOff"]))
      
        elif rawDevice[b"type"] == b"automaticTrigger":
            newDevice = AutomaticTrigger()
            newDevice.setTargetDeviceId(rawDevice[b"targetDeviceId"].decode())  
            newDevice.setListenOn(rawDevice[b"listenOnDeviceId"].decode())  

        elif rawDevice[b"type"] == b"distanceSensor":
            newDevice = DistanceSensor()
            newDevice.setGpio(int(rawDevice[b"gpio"]))
            newDevice.setReactTo(rawDevice[b"targetDeviceId"].decode())
            if rawDevice[b"invertState"] == b"1":
                newDevice.makeToggle()

        if newDevice != None:
            logger.info("Adding %s %s", rawDevice[b'type'], rawDevice[b'id'])
            newDevice.setId(rawDevice[b'id'].decode())
            self.devices.append(newDevice)

    def addSystemDevices(self):
        heartbeat = Hearbeat()
        heartbeat.setId("heartbeat")
        self.devices.append(heartbeat)

        if redisConn.hostName == "watcher":
            stateMan = StateManager()
            stateMan.setId("stateManager")            
            self.devices.append(stateMan)

            heartbeatMon = HeartbeatMonitor()
            heartbeatMon.setId("heartbeatMonitor")
            self.devices.append(heartbeatMon)

            logger.debug("Due to being a master node, stateManager and and heartbeetMonitor are added.")

    def initializeSystem(self):
        logger.debug("Start initialization")
        for device in self.devices:
            device.initialize()   
    