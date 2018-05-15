import queue
import redisManager
import groupToggleButton
 
commandsQueue = queue.Queue()
group = groupToggleButton.GroupToggleButton()
group.handleCommand('switch15:1')
