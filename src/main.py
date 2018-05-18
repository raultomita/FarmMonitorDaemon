import redisManager
import dispatcher
import threading
import time

redisThread = threading.Thread(target=redisManager.RedisClient().run)
redisThread.start()

dispatcherThread = threading.Thread(target=dispatcher.startDispatching)
dispatcherThread.start()


