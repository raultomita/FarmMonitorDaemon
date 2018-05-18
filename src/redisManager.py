import tornado
import tornadis
import asyncio
import redis
import dispatcher
import socket

pool = redis.ConnectionPool(host='127.0.0.1', port=6379)
r = redis.Redis(connection_pool=pool)

def initializeSystem():
    hostName = socket.gethostname()
    print("initialize system for %s" % hostName)
    
    cursor, members = r.sscan(hostName)
    
    while True:
        for deviceId in members:
            device = r.hgetall(deviceId)
            dispatcher.addDevice(deviceId, device)
        if cursor == 0:
            break

        cursor, members = r.sscan(hostName, cursor=cursor)

#log recconnection in redis  cache with an expiration date

class RedisPubSubClient:
    @tornado.gen.coroutine
    def handleCommands(self):
        while True and not dispatcher.stopper.is_set():
            if(self.client.is_connected()):                
                print("[Connected] Read messages")
                result = yield self.client.pubsub_pop_message()
                self.handleResult(result)
                            
            # let's (re)connect (autoconnect mode), call the ping redis command
            # and wait the reply without blocking the tornado ioloop
            # Note: async_call() method on Client instance does not return anything
            # but the callback will be called later with the result.
            else:
                connRes = yield self.client.connect()
                if(connRes == True):
                    yield self.client.pubsub_subscribe("commands")
                print("Is not connected")
            yield tornado.gen.sleep(0.5)
    
    def handleResult(self, result):
        if isinstance(result, tornadis.TornadisException):
            # For specific reasons, tornadis nearly never raises any exception
            # they are returned as result
            print ("got exception: %s" % result)
        else:
            # result is already a python object (a string in this simple example)
            print ("Result: %s" % result[2])
            dispatcher.commandsQueue.put(result[2])
    
    def run(self):
        asyncio.set_event_loop(asyncio.new_event_loop())
        self.client = tornadis.PubSubClient(host="192.168.1.201", port=6379, autoconnect=True)    
        loop = tornado.ioloop.IOLoop.instance()
        loop.run_sync(self.handleCommands)
