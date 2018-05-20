import redisConn
import dispatcher
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

logging.basicConfig(format='%(levelname)-8s:%(module)-15s: %(message)s', level=logLevel)
logger = logging.getLogger(__name__)

redisThread = redisConn.RedisManagerThread()
redisThread.start()

dispatcherThread = dispatcher.DispatcherThread()
dispatcherThread.start()

while True:
    logger.debug(time.clock())
    logger.debug("Active threads %d with queue size %d" % (threading.active_count(), redisConn.commands.qsize()))
    time.sleep(1)



