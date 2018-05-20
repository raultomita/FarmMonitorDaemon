import baseThing
import redisConn
import dispatcher
import logging
import re

logger = logging.getLogger(__name__)

class StateManager(baseThing.Thing):
    def __init__(self):
        self.matcher = re.compile('(switch[0-9]*):([01])')
        self.switches = {}
        logger.debug("read all switches")
        switchLocations = redisConn.readAllSwitchLocations()
        logger.debug(switchLocations)
        for switch in switchLocations:
            self.switches[switch['id']] = {
                'location': switch['location'],
                'state': "0"
            }        

    def handleCommand(self, command):                
        result = self.matcher.fullmatch(command)
        if result != None :
            logger.debug("match %s" % command)            
            deviceId = result.group(1)
            state = result.group(2)
            
            logger.debug("set state %s for %s" % (state, deviceId))
            self.switches[deviceId]['state'] = state
            switchLocation = self.switches[deviceId]['location']
            locationState = 0
            allState = 0

            
           
            for key, switch in self.switches.items():
                if switch['state'] == '1':
                    allState = 1
                    if switchLocation == switch['location'] :
                        locationState = 1

            dispatcher.sendCommand("%s:%d" %(switchLocation, locationState))
            dispatcher.sendCommand('all:%d' % allState)


    def initialize(self):
        redisConn.enqueueCommand("all:?")
