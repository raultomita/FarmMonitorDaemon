import queue
import threading 
import redisManager

import switch
import toggleButton
import led
import stateManager

receivedCommandsQueue = queue.Queue()
stopper = threading.Event()
devices = []

#all deconding and encoding should be in this file
encoding = "utf-8"

def startDispatching():  
    redisManager.readDevices()
    initializeSystem()

    while True and not stopper.is_set() :
        print("Listen for items in queue")
        command = receivedCommandsQueue.get().decode(encoding)
        print("Dequeue")
        print(command)
        handleCommand(command)

def addDevice(id, device):
    print(device)
    if device[b"type"] == b"switch":
        print("found switch %s" % id)
        
        newSwitch = switch.Switch()
        newSwitch.setId(id.decode(encoding))
        newSwitch.setLocation(device[b"location"].decode(encoding))
        newSwitch.setDisplay(device[b"display"].decode(encoding))
        newSwitch.setGpio(int(device[b"gpio"]))

        devices.append(newSwitch)

    elif device[b"type"] == b"stateManager":
        print ("found state manager %s" % id)
        newStateManager = stateManager.StateManager()
        newStateManager.setId(id.decode(encoding))

        devices.append(newStateManager)
    
    elif device[b"type"] == b"toggleButton":
        print("found toggleButton %s" % id)
        newToggleButton = toggleButton.ToggleButton()
        newToggleButton.setId(id.decode(encoding))
        newToggleButton.setGpio(int(device[b"gpio"]))
        if b"commands4On" in device:
            newToggleButton.setCombinedReactTo(device[b"targetDeviceId"].decode(), device[b"commands4On"].decode())
        else:
            newToggleButton.setReactTo(device[b"targetDeviceId"].decode())

        devices.append(newToggleButton)    
    
    elif device[b"type"] == b"led":
        print("found led %s" % id)
        newLed = led.Led()
        newLed.setId(id.decode(encoding))
        newLed.setReactTo(device[b"listenTo"].decode(encoding))
        newLed.setGpio(int(device[b"gpio"]))

        if b"gpioOff" in device:
            newLed.setGpioOff(int(device[b"gpioOff"]))
        
        devices.append(newLed)
        newLed.initialize()

def handleCommand(command):
    for device in devices:
        device.handleCommand(command)
        
def sendCommand(command):
    #throttle commands
    print("send command: %s" % command)
    success = redisManager.publishCommand(command)
    if not success:
        handleCommand(command)
    
def initializeSystem():
    for device in devices:
        device.initialize()