import queue
import threading 
import redisManager

import switch
import stateManager

receivedCommandsQueue = queue.Queue()
stopper = threading.Event()
devices = []

#all deconding and encoding should be in this file
encoding = "utf-8"

def startDispatching():    
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
        print ("found state manager%s" % id)
        newStateManager = stateManager.StateManager()
        newStateManager.setId(id.decode(encoding))

        devices.append(newStateManager)

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