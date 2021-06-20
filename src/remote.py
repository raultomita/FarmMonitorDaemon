import dispatcher
import logging
import threading
from lirc import RawConnection
from evdev import *

logger = logging.getLogger(__name__)
conn = RawConnection()
logger.info(conn)

class RemoteControlThread(threading.Thread):
    def __init__(self):
        super(RemoteControlThread, self).__init__()
        self.daemon = True

    def run(self):
       logger.info("Starting remote control thrread")
       while True:
           try:
               keypress = conn.readline(.0001)
           except:
               keypress=""
                      
           if (keypress != "" and keypress != None):
               data = keypress.split()

               if (data[1] != "00"):
                   keypress = ""
               else:
                   keypress = data[2]
               
               logger.info("Key pressed: %s", keypress)               
               
               if keypress == "KEY_POWER":                   
                   dispatcher.enqueueCommand("switch23")
               elif keypress == "KEY_PREVIOUS":
                   dispatcher.sendCommand("switch22")
               elif keypress == "KEY_NEXT":
                   dispatcher.sendCommand("switch21")
               elif keypress == "KEY_PLAY":
                   dispatcher.sendCommand("switch6:on")
                   dispatcher.sendCommand("switch8:on")

                   dispatcher.sendCommand("switch5:off")
                   dispatcher.sendCommand("switch7:off")
                   dispatcher.sendCommand("switch1:off")
                   dispatcher.sendCommand("switch11:off")
                   dispatcher.sendCommand("switch13:off")
                   dispatcher.sendCommand("switch22:off")
                   dispatcher.sendCommand("switch20:off")
                   dispatcher.sendCommand("switch21:off")
               elif keypress == "KEY_STOP":
                   dispatcher.sendCommand("switch6:off")
                   dispatcher.sendCommand("switch8:off")
                   dispatcher.sendCommand("switch5:off")
                   dispatcher.sendCommand("switch7:off")
                   dispatcher.sendCommand("switch1:off")
                   dispatcher.sendCommand("switch11:off")
                   dispatcher.sendCommand("switch13:off")
                   dispatcher.sendCommand("switch22:off")
                   dispatcher.sendCommand("switch21:off")
                   dispatcher.sendCommand("switch20:off")
