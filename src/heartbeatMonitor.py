import baseThing
import redisConn
import logger
from datetime import datetime

logger = logging.getLogger(__name__)

class HeartbeatMonitor(baseThing.Thing):
    def __init__(self):
         self.matcher = re.compile('heartbeat:([a-z]*)')

    def handleCommand(self, command):
        result = self.matcher.fullmatch(command)
        if result != None :
            logger.debug("heartbeat matched")
            hostname = result.group(1)
            redisConn.enqueueGeneral('HSET', 'heartbeat', hostname, datetime.now().strftime("%d.%m.%y %H:%M:%S"))