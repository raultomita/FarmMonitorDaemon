import baseThing
import redisManager

class Switch(baseThing.Thing):

    def setLocation(self, location):
        self.location = location

    def setDisplay(self, display):
        self.display = display

    def setGpio(self, gpio):
        self.gpio = gpio

    def handleCommand(self, command):
        if command == self.id:
            print("toggle switch %s wtho gpio %d" % (self.id, self.gpio))
        elif command == self.id + ":on" or command == self.location + ":on":
            print("turn on switch %s wtho gpio %d" % (self.id, self.gpio))
        elif command == self.id + ":off" or command == self.location + ":off":
            print("turn off switch %s wtho gpio %d" % (self.id, self.gpio))
        