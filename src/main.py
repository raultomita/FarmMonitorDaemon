import redisManager
import dispatcher
import threading
import time

redisManager.readDevices()
dispatcher.initializeSystem()

redisThread = threading.Thread(target=redisManager.RedisPubSubClient().run)
redisThread.start()

dispatcherThread = threading.Thread(target=dispatcher.startDispatching)
dispatcherThread.start()


