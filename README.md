# Stingray & MuMuDVB

Download Raspbian image and format SD card
```
cd ~/Downloads
ls -lha | grep jessie
unzip 2017-01-11-raspbian-jessie-lite.zip
sudo dd bs=1m if=2017-01-11-raspbian-jessie-lite.img of=/dev/rdisk2
```
Boot raspberry pi, connect hdmi monitor and keyboard, once boot log with:  
user: pi  
password: raspberry  
Introduce the following commands:
```
raspi-config --expand-rootfs
sudo reboot


```
Enter sudo raspi-config in a terminal window.  
Select Interfacing Options.  
Navigate to and select SSH.  
Choose Yes.  
Select Ok.  
Choose Finish.  

Install Requirements
```
lsb_release
sudo apt-get update
sudo apt-get upgrade
sudo apt-get -y install git
sudo git clone https://github.com/jordy33/stin_mumudvb.git
sudo apt-get -y install build-essential
sudo apt-get -y install dvb-apps
```
Compile tune-s2
```
cd tune-s2
sudo make
cd ..
```
Compile udproxy
```
cd udpxy
sudo make
sudo make install
```
Install Firmware:
```
cd ~/prof7500
sudo cp dvb-usb-p7500.fw /lib/firmware/.
sudo reboot
```
To support 8psk for prof7500:
Download the folowing file with sftp and then get:
```
/lib/modules/<currentversion>/kernel/drivers/media/dvb-frontends/stv0900.ko
```
Modify the file with [Hex Editor](https://mh-nexus.de/en/hxd/) and then Upload the file to the same location

```
Open a copy of the stv0900.ko file in your preferred hex editor and search for:  
  
73 f5 c0 00 73 f3 c0 00  
  
Change the two c0 bytes to 00 . (Example Below):

73 f5 00 00 73 f3 00 00 . 
  
Save and replace the stv0900.ko file wherever it was . 
Then reboot
```
Check the adapter is running with the following command:
```
dmesg | grep dvb
```

If Everything is ok . Issue the command ~./m

To see udpxy status
http://substitute-with-raspberry-pi-ip-address:4022/status

The hear the stingray music channels move your antenna to 89w 

To listen via http open stingray-http.3mu from 3mu directory and then with an editor (notepad in windows, textedit in OSX) and substitute ip adress 192.168.1.66 with your raspberry pi address. Save the file. Then open stingray-http.3mu with vlc,kodi,etc... in wherever device you might to use (android device,ios device, windows desktop,osx desktop,etc) that is connected in the same net of the raspberry using utp cable or wifi.

If you want listen with kodi in your raspberry open stingray-multicast.3mu

### Mumudvb
Download debian package for raspberry pi [here](http://mumudvb.net/download/)
Then un pack debian package:
```
dpkg -i mumudvb*.deb
```
Usage
```
mumudvb [options] -c config_file
mumudvb [options] --config config_file
```
Posible Options are:
```
-d, --debug
        Don't deamonize and print messages on the standard output.

-s, --signal
        Print signal strenght every 5 seconds

-t, --traffic
        Print the traffic of the channels every 10 seconds

-l, --list-cards
        List the DVB cards and exit

--card
        The DVB card to use (overrided by the configuration file)

--server_id
        The server id (for autoconfiguration, overrided by the configuration file)

-h, --help
        Show help

-v
        More verbose (add for more)

-q
        More quiet (add for less)

--dumpfile
        Debug option : Dump the stream into the specified file
```

