import queue
import threading 

commandsQueue = queue.Queue()
stopper = threading.Event()

def startDispatching():
    while True and not stopper.is_set() :
        print("Listen for items in queue")
        command = commandsQueue.get()
        print("Dequeue")
        print(command)
        