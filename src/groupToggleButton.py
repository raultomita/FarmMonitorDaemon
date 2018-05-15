import baseThing
import redisManager

class GroupToggleButton(baseThing.Thing):
    reactWhen = ['switch15:1', 'switch15:0']
    def handleCommand(self, command):
        if(command in self.reactWhen):
            result = redisManager.hgetall("switchStates")
            print(result["switch15"])

