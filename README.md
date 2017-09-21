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
- [Hard] Taiat geam
