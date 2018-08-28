### Install scripts
 - Create folder for scripts in your home directory (eg. /home/pi/standby)
 - Download and copy scripts to newly created folder
 - Install python (if needed)
 - Install ConfigParser module (sudo pip install configparser)

```sh
cd /home/pi
mkdir standby
cd ./standby
wget https://raw.githubusercontent.com/SebuZet/Standby/master/scripts/standby-monitor.py
wget https://raw.githubusercontent.com/SebuZet/Standby/master/scripts/standby-shutdown.py
wget https://raw.githubusercontent.com/SebuZet/Standby/master/scripts/standby.ini
sudo pip install configparser
```

### Launch standby monitor at system start
Open /etc/rc.local with your favouite editor (eg. sudo vi /etc/rc.local) :) and add following line before 'exit 0':

python /home/pi/standby/standby-monitor.py &
