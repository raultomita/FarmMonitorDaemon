import asyncio
from evdev import *

dev = InputDevice('/dev/input/event1')
print(dev)

async def helper(dev):
     async for event in dev.async_read_loop():
        if event.type == ecodes.EV_KEY:
            print(categorize(event))

def getKey():
    for event in dev.read_loop():
        if event.type == ecodes.EV_KEY:
            yield categorize(event)

#keygenerator = getKey()

# or you could consume the generator directly in a for loop
#for c in getKey():
#   print(c)
loop = asyncio.get_event_loop()
loop.run_until_complete(helper(dev))
print("test")
