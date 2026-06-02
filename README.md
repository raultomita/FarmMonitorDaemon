# FarmMonitorDaemon

Step 1: Create A Unit File

Open a sample unit file using the command as shown below:

sudo vi /lib/systemd/system/farmMonitor.service

Add in the following text :

 [Unit]
 Description=My Sample Service
 After=multi-user.target

 [Service]
 Type=idle
 ExecStart=/usr/bin/python /home/pi/sample.py

 [Install]
 WantedBy=multi-user.target

This defines a new service called “Sample Service” and we are requesting that it is launched once the multi-user environment is available. The “ExecStart” parameter is used to specify the command we want to run. The “Type” is set to “idle” to ensure that the ExecStart command is run only when everything else has loaded. Note that the paths are absolute and define the complete location of Python as well as the location of our Python script.

In order to store the script’s text output in a log file you can change the ExecStart line to:

ExecStart=/usr/bin/python /home/pi/sample.py > /home/pi/sample.log 2>&1
The permission on the unit file needs to be set to 644 :

sudo chmod 644 /lib/systemd/system/sample.service

Step 2: Configure systemd

Now the unit file has been defined we can tell systemd to start it during the boot sequence :

sudo systemctl daemon-reload
sudo systemctl enable sample.service
Reboot the Pi and your custom service should run:

sudo reboot

##TODO

- [Daemon] Integrare TankLevel

- [Daemon] Call dummy catre web si redis pentru performanta
- [Daemon] Integrare docker
- [Web] Implentare controale (TankLevel)
- [Web] Autoconnect webSocket
- [Deploy] Docker pentru Redis

## Azure Cosmos DB Cloud Integration

The daemon can optionally report device states and heartbeats to Azure Cosmos DB, and retrieve commands from the cloud.

### Prerequisites

Install the `requests` library:

```bash
pip install requests
```

### Environment Variables

Set the following environment variables before starting the daemon (or add them to the systemd unit file via `Environment=`):

| Variable | Description | Example |
|---|---|---|
| `COSMOS_ENDPOINT` | Cosmos DB account endpoint URL | `https://myaccount.documents.azure.com:443/` |
| `COSMOS_KEY` | Primary or secondary account key (base64) | `abc123==` |
| `COSMOS_DATABASE` | Database name | `farmmonitor` |

If these variables are not set, the cloud sync thread starts but immediately exits — the daemon works normally without cloud connectivity.

### Cosmos DB Container Schemas

Create the following containers in the `farmmonitor` database:

#### `device-states` (partition key: `/hostname`)
Upserted on every switch state change.
```json
{
  "id": "home1_switch3",
  "hostname": "home1",
  "type": "switch",
  "display": "Light 1",
  "location": "living",
  "timeStamp": "2026-05-31T20:00:00.000",
  "state": 1,
  "googleType": "LIGHT"
}
```

#### `heartbeats` (partition key: `/hostname`)
Upserted each time the node's heartbeat is confirmed.
```json
{
  "id": "home1",
  "hostname": "home1",
  "lastSeen": "31.05.26 20:00:00"
}
```

#### `commands` (partition key: `/targetHost`)
Written by an external app; the daemon polls every 30 s, dispatches pending commands, then marks them processed.
```json
{
  "id": "unique-command-uuid",
  "targetHost": "home1",
  "command": "switch3:on",
  "timestamp": "2026-05-31T20:00:00Z",
  "status": "pending"
}
```
Commands whose `timestamp` is older than the device's last known state change are **silently skipped** (marked `"processed"`) to avoid replaying outdated instructions.

### Systemd Service with Cosmos DB Variables

Add `Environment=` lines to `/lib/systemd/system/farmMonitor.service`:

```ini
[Service]
Type=idle
Environment="COSMOS_ENDPOINT=https://myaccount.documents.azure.com:443/"
Environment="COSMOS_KEY=<your-key>"
Environment="COSMOS_DATABASE=farmmonitor"
ExecStart=/usr/bin/python /home/pi/src/main.py 192.168.1.x
```

