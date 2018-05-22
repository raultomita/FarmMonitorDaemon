import baseThing
import redisConn
from datetime import datetime

class Hearbeat(baseThing.Thing):
    def __init__(self):
        self.sentAt = datetime.min

    def handleCommand(self, command):
        if command = "timer" and (datetime.now() - self.sentAt).total_seconds() > 420:
            self.sentAt = datetime.now()
            redisConn.enqueueCommand(f"heartbeat:{redisConn.hostname}")

