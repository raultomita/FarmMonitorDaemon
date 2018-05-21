import queue
import threading 
import redisConn
import logging

import switch
import toggleButton
import led
import stateManager

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

        logger.debug("Before handling commands there are %d" % receivedCommandsQueue.qsize())

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
        
        logger.info("Added %d devices from %d received", len(self.devices), len(devices))

    def addDevice(self, rawDevice):
        logger.debug(rawDevice)
        newDevice = None
        
        if rawDevice[b"type"] == b"switch":
            newDevice = switch.Switch()            
            newDevice.setLocation(rawDevice[b"location"].decode())
            newDevice.setDisplay(rawDevice[b"display"].decode())
            newDevice.setGpio(int(rawDevice[b"gpio"]))

        elif rawDevice[b"type"] == b"toggleButton":           
            newDevice = toggleButton.ToggleButton()            
            newDevice.setGpio(int(rawDevice[b"gpio"]))
            if b"commands4On" in rawDevice:
                newDevice.setCombinedReactTo(rawDevice[b"targetDeviceId"].decode(), rawDevice[b"commands4On"].decode())
            else:
                newDevice.setReactTo(rawDevice[b"targetDeviceId"].decode())

        elif rawDevice[b"type"] == b"led":            
            newDevice = led.Led()
            newDevice.setReactTo(rawDevice[b"listenTo"].decode())
            newDevice.setGpio(int(rawDevice[b"gpio"]))

            if b"gpioOff" in rawDevice:
                newDevice.setGpioOff(int(rawDevice[b"gpioOff"]))
            
        elif rawDevice[b"type"] == b"stateManager":            
            newDevice = stateManager.StateManager()

        if newDevice != None:
            logger.info("Adding %s %s", rawDevice[b'type'], rawDevice[b'id'])
            newDevice.setId(rawDevice[b'id'].decode())
            self.devices.append(newDevice)


    def initializeSystem(self):
        logger.debug("Start initialization")
        for device in self.devices:
            device.initialize()   
    