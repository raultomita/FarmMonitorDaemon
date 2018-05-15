import tornado
import tornadis
import redis

print("Enter redis manager")

pool = redis.ConnectionPool(host='localhost', port=6379)

def hgetall(command):
    r = redis.Redis(connection_pool=pool)
    return r.hgetall(command)

def handleResult(result):
    if isinstance(result, tornadis.TornadisException):
        # For specific reasons, tornadis nearly never raises any exception
        # they are returned as result
        print ("got exception: %s" % result)
    else:
        # result is already a python object (a string in this simple example)
        print ("Result: %s" % result)


@tornado.gen.coroutine
def main():
    while True:
        if(client.is_connected()):
            print("is connected")
            print("Read messages")
            result = yield client.pubsub_pop_message()
            handleResult(result)
                        
        # let's (re)connect (autoconnect mode), call the ping redis command
        # and wait the reply without blocking the tornado ioloop
        # Note: async_call() method on Client instance does not return anything
        # but the callback will be called later with the result.
        else:
            connRes = yield client.connect()
            if(connRes == True):
                yield client.pubsub_subscribe("commands")
            print("Is not connected")
        yield tornado.gen.sleep(1)


# Build a tornadis.Client object with some options as kwargs
# host: redis host to connect
# port: redis port to connect
# autoconnect=True: put the Client object in auto(re)connect mode
#client = tornadis.PubSubClient(host="192.168.1.200", port=6379, autoconnect=True)

# Start a tornado IOLoop, execute the coroutine and end the program
#loop = tornado.ioloop.IOLoop.instance()
#loop.run_sync(main)