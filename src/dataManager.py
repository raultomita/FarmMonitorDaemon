from redis import ConnectionPool, Redis, ConnectionError
import time
import threading
import queue
import logging
import socket
import json

import dispatcher

logger = logging.getLogger(__name__)
hostName = socket.gethostname()
redisIP = ""

needsInitialization = False

def initializeSystem(redis):    
    logger.info("Initialize system for %s" % hostName)
    retrieveDevicesForThisHost(redis)
    retrieveAllSwitchLocations(redis)
    logger.info("System initialized and JSON file created")

def retrieveDevicesForThisHost(redis)    
    devices = []
    cursor, members = redis.sscan(hostName)
    devicesJson = ''

    while True:
        for deviceId in members:
            logger.debug("Querying for device %s" % deviceId)
            rawDevice = redis.hgetall(deviceId)
            device = { 'id': deviceId.decode()}
            for prop in rawDevice:
                device[prop.decode()] = rawDevice[prop].decode()

            devices.append(device)

        if cursor == 0:
            logger.debug("Received all devices")
            break

        cursor, members = redis.sscan(hostName, cursor=cursor)    
    
    devicesJson = json.dumps(devices, sort_keys=True, indent=4)
    logger.debug(devicesJson)
    configFile = open("devices.json","w")
    configFile.write(devicesJson)
    configFile.close()    

def retrieveAllSwitchLocations(redis):
    switches = []  
    locationsJson = ''
    cursor, members = redis.scan(match='switch*')

    while True:
        for deviceId in members:
            device = redis.hmget(deviceId, "location")
            switches.append({
                'id': deviceId.decode(),
                'location':device[0].decode()
            })            
       
        if cursor == 0:
            break

        cursor, members = redis.scan(match='switch*', cursor=cursor)

    locationsJson = json.dumps(switches, sort_keys=True, indent=4)
    logger.debug(locationsJson)
    configFile = open("switchLocations.json","w")
    configFile.write(locationsJson)
    configFile.close()  

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
        self.pool = ConnectionPool(host=redisIP, port=6379)
        self.redis = Redis(connection_pool=self.pool, decode_responses=True)  
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
        if needsInitialization == True:
            initializeSystem(self.redis)

        dispatcher.enqueueCommand("refreshSystem")

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