import dispatcher
import logging
import threading
from evdev import *

logger = logging.getLogger(__name__)
dev = InputDevice('/dev/input/event0')
logger.info(dev)
logger.info(dev.capabilities(verbose=True))
class RemoteControlThread(threading.Thread):
    def __init__(self):
        super(RemoteControlThread, self).__init__()
        self.daemon = True

    def run(self):
       logger.info("Starting remote control thrread")
    
       for event in dev.read_loop():
           
           if event.type == ecodes.EV_KEY:
               rcEvent = categorize(event)
             
               if rcEvent.keystate == rcEvent.key_down:
                   if rcEvent.keycode == "KEY_RED":
                       dispatcher.enqueueCommand("switch23")
                   elif rcEvent.keycode == "KEY_PREVIOUS":
                       dispatcher.sendCommand("switch22")
                   elif rcEvent.keycode == "KEY_NEXT":
                       dispatcher.sendCommand("switch21")
                   elif rcEvent.keycode == "KEY_PLAY":
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
                   elif rcEvent.keycode == "KEY_STOP":
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