### Install scripts
 - Create folder for scripts in your home directory (eg. /home/pi/standby)
 - Download and copy scripts to newly created folder
 - Install python (if needed)
 - Install ConfigParser module (sudo pip install configparser)

### Launch standby monitor at system start
Open /etc/rc.local with your favouite editor (eg. sudo vi /etc/rc.local) :) and add following line before 'exit 0':

python /home/pi/standby/standby-monitor.py &
