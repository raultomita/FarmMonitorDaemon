sudo apt-get update
cd FarmMonitorDaemon
git pull
cd ..

sudo apt-get -y install build-essential tk-dev libncurses5-dev libncursesw5-dev libreadline6-dev libdb5.3-dev libgdbm-dev libsqlite3-dev libssl-dev libbz2-dev libexpat1-dev liblzma-dev zlib1g-dev
tar -zxvf python3.6-release-build.tar.gz
cd Python-3.6.5
sudo make altinstall

cd ..
sudo rm -r Python-3.6.5
rm python3.6-release-build.tar.gz
sudo apt-get -y --purge remove build-essential tk-dev
sudo apt-get -y --purge remove libncurses5-dev libncursesw5-dev libreadline6-dev
sudo apt-get -y --purge remove libdb5.3-dev libgdbm-dev libsqlite3-dev libssl-dev
sudo apt-get -y --purge remove libbz2-dev libexpat1-dev liblzma-dev zlib1g-dev
sudo apt-get -y autoremove
sudo apt-get clean

sudo pip3.6 install --upgrade pip
sudo pip3.6 install redis
sudo pip3.6 install gpiozero
sudo pip3.6 install RPi.GPIO
