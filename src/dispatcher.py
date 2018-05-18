import queue
import threading 

import switch

commandsQueue = queue.Queue()
stopper = threading.Event()
devices = []

#all deconding and encoding should be in this file
encoding = "utf-8"

def startDispatching():    
    while True and not stopper.is_set() :
        print("Listen for items in queue")
        command = commandsQueue.get().decode(encoding)
        print("Dequeue")
        print(command)
        for device in devices:
            device.handleCommand(command)
        
def addDevice(id, device):
    if device[b"type"] == b"switch":
        print("found switch %s" % id)
        
        newSwitch = switch.Switch()
        newSwitch.setId(id.decode(encoding))
        newSwitch.setLocation(device[b"location"].decode(encoding))
        newSwitch.setDisplay(device[b"display"].decode(encoding))
        newSwitch.setGpio(int(device[b"gpio"]))

        devices.append(newSwitch)
    print(device)