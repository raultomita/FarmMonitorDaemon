class Thing:    
    def __init__(self):
        self.name = "NoNma"
        self.location = "NoLocation"
        self.display = "NoDisplay"

    def setName(self, name):
        self.name = name
    
    def setLocation(self, location):
        self.location = location

    def setDisplay(self, display):
        self.display = display

    def handleCommand(self, command):
        pass