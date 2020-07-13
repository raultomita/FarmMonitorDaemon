import redisConn
import dispatcher
import remote
import threading
import time
import logging
import sys

logLevel = logging.WARNING

if len(sys.argv) > 1:
    if sys.argv[1] == '-d':
        logLevel = logging.DEBUG
    elif sys.argv[1] == '-i':
        logLevel = logging.INFO
    elif sys.argv[1] == '-init':
        logLevel = logging.DEBUG
        redisCon.needsInitialization = True

logging.basicConfig(format='%(levelname)-8s:%(module)-15s: %(message)s', level=logLevel)
logger = logging.getLogger(__name__)

redisThread = redisConn.RedisManagerThread()
redisThread.start()

dispatcherThread = dispatcher.DispatcherThread()
dispatcherThread.start()

remoteControlThread = remote.RemoteControlThread()
remoteControlThread.start()

while True:
    logger.debug(time.clock())
    logger.debug("Active threads %d with redis queue size %d and dispatcher queue size %d" % (threading.active_count(), redisConn.commands.qsize(), dispatcher.receivedCommandsQueue.qsize()))
    time.sleep(2)
    dispatcher.enqueueCommand("timer")



