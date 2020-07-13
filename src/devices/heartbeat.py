import baseThing
import dataManager
import logging
import re

from datetime import datetime

logger = logging.getLogger(__name__)

class Hearbeat(baseThing.Thing):    
    def handleCommand(self, command):
        if command == "heartbeat:?":            
            dataManager.enqueueCommand(f"heartbeat:{dataManager.hostName}")
            logger.info(f"heartbeat:{dataManager.hostName} sent")

class HeartbeatMonitor(baseThing.Thing):
    def __init__(self):
         self.matcher = re.compile('heartbeat:([a-z]*)')
         self.sentAt = datetime.min

    def handleCommand(self, command):
        result = self.matcher.fullmatch(command)
        if result != None :
            logger.debug("heartbeat matched")
            hostname = result.group(1)
            dataManager.enqueueGeneral('HSET', 'heartbeat', hostname, datetime.now().strftime("%d.%m.%y %H:%M:%S"))

        elif command == "timer" and (datetime.now() - self.sentAt).total_seconds() > 420:
            self.sentAt = datetime.now()            
            dataManager.enqueueCommand(f"heartbeat:?")
            logger.info(f"heartbeat request send sent")