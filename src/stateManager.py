import baseThing
import redisManager
import dispatcher
import re

class StateManager(baseThing.Thing):
    def __init__(self):
        self.switches = {}
        self.initialized = False

    def handleCommand(self, command): 
               
        result = re.search('(switch[0-9]*):([01])', command)
        if result != None and self.initialized:
            
            deviceId = result.group(1)
            state = result.group(2)
            
            print("set state %s for %s" % (state, deviceId))
            redisManager.hset("states", deviceId, state)
            switchLocation = self.switches[deviceId]
            locationState = 0
            allState = 0

            states = redisManager.hgetall("states")
           
            for key, location in self.switches.items():
                if key.encode() in states and states[key.encode()] == b'1':
                    allState = 1
                    if location == switchLocation :
                        locationState = 1

            dispatcher.sendCommand("%s:%d" %(switchLocation, locationState))
            dispatcher.sendCommand('all:%d' % allState)


    def initialize(self):
        switchLocations = redisManager.readAllSwitchLocations()
        for key, value in switchLocations.items():
            self.switches[key] = value        

        print("states: ")
        print(self.switches)
        self.initialized = True
        dispatcher.sendCommand("all:?")
