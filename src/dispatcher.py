import queue
import threading 
import dataManager
import logging
import json
from os import path

from devices.switch import Switch
from devices.toggleButton import ToggleButton
from devices.led import Led
from devices.stateManager import StateManager
from devices.automaticTrigger import AutomaticTrigger
from devices.distanceSensor import DistanceSensor
from devices.heartbeat import *
from devices.curtain import *

logger = logging.getLogger(__name__)

receivedCommandsQueue = queue.Queue()

def enqueueCommand(command):
    receivedCommandsQueue.put(command)    
    logger.info("Command received %s" % command)

def sendCommand(command):
    #throttle commands
    logger.debug("send command: %s" % command)
    enqueueCommand(command)
    dataManager.enqueueCommand(command)

class DispatcherThread(threading.Thread):
    def __init__(self):
        super(DispatcherThread, self).__init__()
        self.daemon = True
        self.devices = []

    def run(self):
        logger.debug("Before handling commands, pending ones are %d" % receivedCommandsQueue.qsize())

        while True:
            logger.debug("Listen for items in queue")
            command = receivedCommandsQueue.get()
            logger.debug("command %s dequeued" % command)
        
            self.handleCommand(command)

    def handleCommand(self, command):
        if command == "refreshSystem":
            self.addDevices()
            self.initializeSystem()
     
        for device in self.devices:
            device.handleCommand(command)

    def addDevices(self):
        if path.exists("devices.json"):
            configFile = open("devices.json","r")        
            devices = json.loads(configFile.read())        
            
            for device in devices:
                self.addDevice(device)
                    
            logger.info("Added %d devices from %d received", len(self.devices), len(devices))
            self.addSystemDevices()
        else:
            logger.warning("File devices.json is not found. Restart with -init as arg to initialize it.")

    def addDevice(self, rawDevice):
        logger.debug(rawDevice)
        newDevice = None
        
        if rawDevice["type"] == "switch":
            newDevice = Switch()            
            newDevice.setLocation(rawDevice["location"])
            newDevice.setDisplay(rawDevice["display"])
            newDevice.setGpio(int(rawDevice["gpio"]))
            newDevice.setGoogleType(rawDevice["googleType"])

        elif rawDevice["type"] == "toggleButton":           
            newDevice = ToggleButton()            
            newDevice.setGpio(int(rawDevice["gpio"]))
            
            if "commands4On" in rawDevice:
                newDevice.setCombinedReactTo(rawDevice["targetDeviceId"], rawDevice["commands4On"])
            else:
                newDevice.setReactTo(rawDevice["targetDeviceId"])
            
            if "logPressedCommand" in rawDevice:
                newDevice.setReactToLongPressed(rawDevice["logPressedCommand"])

        elif rawDevice["type"] == "led":            
            newDevice = Led()
            newDevice.setReactTo(rawDevice["listenTo"])
            newDevice.setGpio(int(rawDevice["gpio"]))

            if "gpioOff" in rawDevice:
                newDevice.setGpioOff(int(rawDevice["gpioOff"]))
      
        elif rawDevice["type"] == "automaticTrigger":
            newDevice = AutomaticTrigger()
            newDevice.setTargetDeviceId(rawDevice["targetDeviceId"])  
            newDevice.setListenOn(rawDevice["listenOnDeviceId"])  

        elif rawDevice["type"] == "distanceSensor":
            newDevice = DistanceSensor()
            newDevice.setGpio(int(rawDevice["gpio"]))
            newDevice.setReactTo(rawDevice["targetDeviceId"])
            if rawDevice["invertState"] == "1":
                newDevice.makeToggle()

        if newDevice != None:
            logger.info("Adding %s %s", rawDevice['type'], rawDevice['id'])
            newDevice.setId(rawDevice['id'])
            self.devices.append(newDevice)

    def addSystemDevices(self):
        heartbeat = Hearbeat()
        heartbeat.setId("heartbeat")        
        self.devices.append(heartbeat)
        # curtainController = CurtainController()
        # curtainController.setId("curtain")    
        # self.devices.append(curtainController)

        if dataManager.hostName == "watcher":
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
    