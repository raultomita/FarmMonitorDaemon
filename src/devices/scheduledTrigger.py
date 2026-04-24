import baseThing
import dispatcher
import logging
from datetime import datetime, timedelta

logger = logging.getLogger(__name__)

class ScheduledTrigger(baseThing.Thing):
    """Turns a target switch ON at a CRON schedule for a set duration, then OFF.
    If the switch is already ON when the schedule fires (manual override),
    no auto-off is scheduled — the user's intent to keep it on is respected.
    If the switch is manually turned OFF during the window, auto-off is cancelled.
    """

    def __init__(self):
        super().__init__()
        self.targetDeviceId = None
        self.cronMinute = set()
        self.cronHour = set()
        self.cronDow = set()  # 0=Sun, 1=Mon, ..., 6=Sat (CRON standard)
        self.durationSeconds = 7200
        self.turnedOnBySchedule = False
        self.scheduledOffAt = None
        self.switchState = False
        self.lastTriggeredMinute = None

    def setTargetDeviceId(self, deviceId):
        self.targetDeviceId = deviceId

    def setDuration(self, seconds):
        self.durationSeconds = seconds

    def setCron(self, cronExpression):
        """Parse a CRON expression: minute hour day-of-month month day-of-week.
        Only minute, hour, and day-of-week are evaluated.
        Day-of-week uses CRON convention: 0=Sunday, 1=Monday, ..., 6=Saturday.
        Supports: * (any), N (exact), N,N (list), N-N (range).
        """
        parts = cronExpression.strip().split()
        if len(parts) != 5:
            raise ValueError(f"Invalid CRON expression: {cronExpression}")

        self.cronMinute = self._parseCronField(parts[0], 0, 59)
        self.cronHour = self._parseCronField(parts[1], 0, 23)
        self.cronDow = self._parseCronField(parts[4], 0, 6)

    def _parseCronField(self, field, minVal, maxVal):
        if field == '*':
            return set(range(minVal, maxVal + 1))

        values = set()
        for part in field.split(','):
            if '-' in part:
                start, end = part.split('-', 1)
                values.update(range(int(start), int(end) + 1))
            else:
                values.add(int(part))
        return values

    def handleCommand(self, command):
        if command == f"{self.targetDeviceId}:1":
            self.switchState = True
        elif command == f"{self.targetDeviceId}:0":
            self.switchState = False
            if self.turnedOnBySchedule:
                logger.debug(f"Switch {self.targetDeviceId} turned off during scheduled window, cancelling auto-off")
                self.turnedOnBySchedule = False
                self.scheduledOffAt = None
        elif command == "timer":
            self._checkSchedule()
            self._checkAutoOff()

    def _checkSchedule(self):
        now = datetime.now()
        triggerKey = (now.year, now.month, now.day, now.hour, now.minute)

        if triggerKey == self.lastTriggeredMinute:
            return

        cronDow = now.isoweekday() % 7  # isoweekday: Mon=1..Sun=7 → CRON: Sun=0..Sat=6

        if now.minute in self.cronMinute and now.hour in self.cronHour and cronDow in self.cronDow:
            self.lastTriggeredMinute = triggerKey

            if self.switchState:
                logger.debug(f"Schedule triggered for {self.targetDeviceId} but switch is already on (manual override, skipping)")
                return

            logger.debug(f"Schedule turning on {self.targetDeviceId} for {self.durationSeconds}s")
            dispatcher.sendCommand(f"{self.targetDeviceId}:on")
            self.turnedOnBySchedule = True
            self.scheduledOffAt = now + timedelta(seconds=self.durationSeconds)

    def _checkAutoOff(self):
        if self.turnedOnBySchedule and self.scheduledOffAt and datetime.now() >= self.scheduledOffAt:
            logger.debug(f"Scheduled auto-off for {self.targetDeviceId}")
            dispatcher.sendCommand(f"{self.targetDeviceId}:off")
            self.turnedOnBySchedule = False
            self.scheduledOffAt = None
