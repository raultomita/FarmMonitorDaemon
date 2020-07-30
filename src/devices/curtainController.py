import baseThing
import logging

OPENED = 3
HALFCLOSED = 2
CLOSED = 1

logger = logging.getLogger(__name__)

class CurtainController(baseThing.Thing):
    def __init__(self):
        self.state = 0
        self.direction = 0

    def handleCommand(self, command):