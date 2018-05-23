import baseThing
import redisConn
import logging
import re

from datetime import datetime

logger = logging.getLogger(__name__)

class Hearbeat(baseThing.Thing):    
    def handleCommand(self, command):
        if command == "heartbeat:?":            
            redisConn.enqueueCommand(f"heartbeat:{redisConn.hostName}")
            logger.info(f"heartbeat:{redisConn.hostName} sent")

class HeartbeatMonitor(baseThing.Thing):
    def __init__(self):
         self.matcher = re.compile('heartbeat:([a-z]*)')
         self.sentAt = datetime.min

    def handleCommand(self, command):
        result = self.matcher.fullmatch(command)
        if result != None :
            logger.debug("heartbeat matched")
            hostname = result.group(1)
            redisConn.enqueueGeneral('HSET', 'heartbeat', hostname, datetime.now().strftime("%d.%m.%y %H:%M:%S"))

        elif command == "timer" and (datetime.now() - self.sentAt).total_seconds() > 420:
            self.sentAt = datetime.now()            
            redisConn.enqueueCommand(f"heartbeat:?")
            logger.info(f"heartbeat request send sent")