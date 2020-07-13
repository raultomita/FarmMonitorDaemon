from redis import ConnectionPool, StrictRedis, ConnectionError
import time
import threading
import queue
import logging
import socket

import dispatcher

logger = logging.getLogger(__name__)
hostName = socket.gethostname()
needsInitialization = False
#Local initialization

localPool = ConnectionPool(host='127.0.0.1', port=6379)
localRedis = StrictRedis(connection_pool=localPool, decode_responses=True)

def readDevices():    
    
    logger.info("initialize system for %s" % hostName)
    devices = []
    cursor, members = localRedis.sscan(hostName)
        
    while True:
        for deviceId in members:
            device = localRedis.hgetall(deviceId)
            device[b'id'] = deviceId
            devices.append(device)

        if cursor == 0:
            logger.debug("Received all devices")
            break

        cursor, members = localRedis.sscan(hostName, cursor=cursor)    
    
    return devices

def readAllSwitchLocations():
    switches = []  
    cursor, members = localRedis.scan(match='switch*')

    while True:
        for deviceId in members:
            device = localRedis.hmget(deviceId, "location")
            switches.append({
                'id': deviceId.decode(),
                'location':device[0].decode()
            })            
       
        if cursor == 0:
            break

        cursor, members = localRedis.scan(match='switch*', cursor=cursor)

    return switches

#Sending and receiving comands - Connection: 192.168.1.201

commands = queue.Queue()

def enqueueNotification(notification):
    enqueueGeneral("PUBLISH", "notifications", notification)

def enqueueCommand(command):
    enqueueGeneral("PUBLISH", "commands", command)

def enqueueGeneral(*args):
    commands.put(args)

class RedisManagerThread(threading.Thread):
    def __init__(self):
        super(RedisManagerThread, self).__init__()
        self.pool = ConnectionPool(host='192.168.1.201', port=6379)
        self.redis = StrictRedis(connection_pool=self.pool, decode_responses=True)  
        self.pubSub = self.redis.pubsub()
        self.subcriptionThread = None
        self.daemon = True
        self.connectionAttempts = 1

    def commandsHandler(self, command):
        dispatcher.enqueueCommand(command['data'].decode())

    def clearQueue(self):
        while not commands.empty():
            commands.get()

    def run(self):
        if needsInitialization == True
            logger.info("reading configuration from Redis")
        
        while True:
            try:                 
                if self.subcriptionThread is None or not self.subcriptionThread.is_alive():
                    self.clearQueue()       
                    self.redis.ping()                    
                    logger.info("connected")
                    self.pubSub.subscribe(commands=self.commandsHandler)
                    self.subcriptionThread = self.pubSub.run_in_thread(daemon=True)  
                    self.connectionAttempts = 1
                    dispatcher.enqueueCommand("all:?")
                else:
                    logger.debug("Listening for commands")                    
                    command4Send = commands.get(timeout=15)
                    logger.debug("execute command:")
                    logger.debug(command4Send)
                    self.redis.execute_command(*command4Send)                                          
                    logger.debug("command executed")
            except queue.Empty:
                pass
            except ConnectionError:
                logger.warning("Connection error, sleep for %d seconds" % self.connectionAttempts)
                time.sleep(self.connectionAttempts)
                if self.connectionAttempts < 32:
                    self.connectionAttempts *= 2