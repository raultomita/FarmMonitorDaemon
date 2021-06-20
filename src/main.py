import logging

logLevel = logging.WARNING
logger = logging.getLogger(__name__)

import dataManager
import dispatcher
import threading
import time

import sys
import re

#sys.argv holds the command line args of pyhton exe. First arg is the script name itself.

if len(sys.argv) >= 2 and re.fullmatch("^[0-9]{1,3}(\.[0-9]{1,3}){3}$", sys.argv[1]) != None:
    dataManager.redisIP = sys.argv[1]
else:
    logger.error("The IP for Redis service is not sent as the first argument of the script (or has an incorrect format)")
    sys.exit()

if len(sys.argv) > 2:
    if sys.argv[2] == '-d':
        logLevel = logging.DEBUG
    elif sys.argv[2] == '-i':
        logLevel = logging.INFO
    elif sys.argv[2] == '-init':
        logLevel = logging.DEBUG
        dataManager.needsInitialization = True

logging.basicConfig(format='%(levelname)-8s:%(module)-15s: %(message)s', level=logLevel)

redisThread = dataManager.RedisManagerThread()
redisThread.start()

dispatcherThread = dispatcher.DispatcherThread()
dispatcherThread.start()

try:
    import remote
    remoteControlThread = remote.RemoteControlThread()
    remoteControlThread.start()    
except ImportError as error:
    logger.warning("Remote cannot be loaded due to: %s" % error) 

while True:
    logger.debug(time.process_time())
    logger.debug("Active threads %d with redis queue size %d and dispatcher queue size %d" % (threading.active_count(), dataManager.commands.qsize(), dispatcher.receivedCommandsQueue.qsize()))
    time.sleep(1)
    dispatcher.enqueueCommand("timer")



