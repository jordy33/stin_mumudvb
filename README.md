tingray & MuMuDVB

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
sudo apt-get install libdvbcsa1
sudo apt-get install dvb-apps
sudo dpkg -i mumudvb*.deb
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
Tune a channel and scan pids
```
sudo tune-s2 4100 H 29856 -lnb CBAND -system DVB-S2
sudo scan -c -o pids
```
Output:
```
SN1A                     (0x0001) 01: PCR == V   V 0x0070 A 0x0072 (ENG) 0x0073 (ENG) 0x0074 (ENG)
SN1B                     (0x0002) 01: PCR == V   V 0x007a A 0x007c (ENG) 0x007d (ENG) 0x007e (ENG)
SN1C                     (0x0003) 01: PCR == V   V 0x0084 A 0x0086 (ENG) 0x0087 (ENG) 0x0088 (ENG)
SN1D                     (0x0004) 01: PCR == V   V 0x008e A 0x0090 (ENG) 0x0091 (ENG) 0x0092 (ENG)
```
### Example:  
Go to satellite 91w   
```
./rotor-control -m 2 -n 18
```
Create arts.txt and insert the following:
```
autoconfiguration=full
lnb_type=standard
lnb_lof_standard=5150
lnb_lof_low=0
freq=4066
modulation=8PSK
delivery_system=DVBS2
pol=v
srate=6051
```
In terminal put the following:
```
mumudvb -d --config arts.txt
```
Open vlc and open network:
```
udp://239.100.0.0:1234
```
sudo cp SoftCam.Key /var/keys/.
```
sudo mkdir /var/keys

cd oscam
cp SoftCam.key /var/keys/SoftCam.Key 
sudo cp oscam.conf /usr/local/etc
sudo ./oscam

```

### Radio Example

Go to satellite 105w   
```
./rotor-control -m 2 -n 11
```
Tune Channel
```
./tune-s2 3707 V 3333 -lnb CBAND -system DVB-S
```
Scan Channel
```
scan-s2 -c
```
Results
```
NFL1] NFL Draft;NFL1:1442:HM2:S0.0W:3333:0:4360:0:0:1:9:3:0
KESN;:1442:HM2:S0.0W:3333:0:5355:0:0:2:9:3:0
CONCACAF WCQ;DESPN:1442:HM2:S0.0W:3333:0:4846:0:0:3:9:3:0
FUTBOL PICANTE;DESPN:1442:HM2:S0.0W:3333:0:4207:0:0:4:9:3:0
CONCACAF OLYMPIC QUALIFIER;DESPN:1442:HM2:S0.0W:3333:0:4139:0:0:5:9:3:0
[CBB1] College Basketball;CBB1:1442:HM2:S0.0W:3333:0:5559:0:0:6:9:3:0
[CBB2] College Basketball;CBB2:1442:HM2:S0.0W:3333:0:5560:0:0:7:9:3:0
[AUX2] NBA Special;AUX2:1442:HM2:S0.0W:3333:0:4371:0:0:8:9:3:0
[NBA1] NBA PlayOffs;NBA1:1442:HM2:S0.0W:3333:0:5561:0:0:9:9:3:0
[CFB1] College Football;CFB1:1442:HM2:S0.0W:3333:0:4289:0:0:10:9:3:0
[CFB2] College Football;CFB2:1442:HM2:S0.0W:3333:0:4315:0:0:11:9:3:0
INTERNATIONAL FRIENDLIES;DESPN:1442:HM2:S0.0W:3333:0:4319:0:0:12:9:3:0
GERMAN CUP;DESPN:1442:HM2:S0.0W:3333:0:4140:0:0:13:9:3:0
[NFL1] Football Sun PxP;NFL1:1442:HM2:S0.0W:3333:0:5442:0:0:14:9:3:0
USA NATIONAL TEAM;DESPN:1442:HM2:S0.0W:3333:0:4206:0:0:15:9:3:0
EUROPA LEAGUE;DESPN:1442:HM2:S0.0W:3333:0:4285:0:0:16:9:3:0
DEP NFL MNF;DESPN:1442:HM2:S0.0W:3333:0:4217:0:0:17:9:3:0
[GOLF] British Open Golf;GOLF:1442:HM2:S0.0W:3333:0:5213:0:0:18:9:3:0
SOUTH AMERICAN QUALIFIERS;DESPN:1442:HM2:S0.0W:3333:0:5214:0:0:19:9:3:0
[AUX2] World Cup Hockey;AUX2:1442:HM2:S0.0W:3333:0:5259:0:0:20:9:3:0
Euro Cup Finals;AUX1:1442:HM2:S0.0W:3333:0:4141:0:0:21:9:3:0
WMVP;:1442:HM2:S0.0W:3333:0:4119:0:0:22:9:3:0
Freddie and Fitzsimmons;ESPN:1442:HM2:S0.0W:3333:0:4231:0:0:29:9:3:0
ESPN All Night;ESPN:1442:HM2:S0.0W:3333:0:4231:0:0:31:9:3:0
First & Last;ESPN:1442:HM2:S0.0W:3333:0:4231:0:0:36:9:3:0
Spain and Fitz;ESPN:1442:HM2:S0.0W:3333:0:4231:0:0:37:9:3:0
[MLB1] MLB Regular;MLB1:1442:HM2:S0.0W:3333:0:4113:0:0:40:9:3:0
[MLB2] MLB Regular;MLB2:1442:HM2:S0.0W:3333:0:4122:0:0:41:9:3:0
[AUX3] MLB Division Series;AUX3:1442:HM2:S0.0W:3333:0:4123:0:0:42:9:3:0
[AUX4] MLB Division Series;AUX4:1442:HM2:S0.0W:3333:0:4124:0:0:43:9:3:0
LA LIGA COPA DEL REY;DESPN:1442:HM2:S0.0W:3333:0:4361:0:0:44:9:3:0
[NBA1] NBA Conference;NBA1:1442:HM2:S0.0W:3333:0:4125:0:0:45:9:3:0
[NBA1] NBA Finals;NBA1:1442:HM2:S0.0W:3333:0:4322:0:0:46:9:3:0
BOXEO DE CAMPEONES;DESPN:1442:HM2:S0.0W:3333:0:5513:0:0:47:9:3:0
ESPN News Streaming Cover;COVER:1442:HM2:S0.0W:3333:0:4288:0:0:48:9:3:0
[MLB1] MLB- All Star;MLB1:1442:HM2:S0.0W:3333:0:4316:0:0:49:9:3:0
Saturday GameDay;ESPN:1442:HM2:S0.0W:3333:0:4231:0:0:50:9:3:0
Saturday GameNight;ESPN:1442:HM2:S0.0W:3333:0:4231:0:0:51:9:3:0
[NBA1] NBA Draft;NBA1:1442:HM2:S0.0W:3333:0:5195:0:0:54:9:3:0
Saturday AM Sports;ESPN:1442:HM2:S0.0W:3333:0:4231:0:0:68:9:3:0
Sunday GameDay;ESPN:1442:HM2:S0.0W:3333:0:4231:0:0:77:9:3:0
Sunday AM Sports;ESPN:1442:HM2:S0.0W:3333:0:4231:0:0:93:9:3:0
Sunday GameNight;ESPN:1442:HM2:S0.0W:3333:0:4231:0:0:94:9:3:0
[MLB2] MLB Division Series;MLB2:1442:HM2:S0.0W:3333:0:5246:0:0:99:9:3:0
[AUX1] NBA All Star Game;AUX1:1442:HM2:S0.0W:3333:0:5254:0:0:100:9:3:0
Dan LeBatard with Stugotz;ESPN:1442:HM2:S0.0W:3333:0:4231:0:0:101:9:3:0
DEP NBA GAMES;DESPN:1442:HM2:S0.0W:3333:0:4283:0:0:102:9:3:0
The Will Cain Show;ESPN:1442:HM2:S0.0W:3333:0:4231:0:0:103:9:3:0
Golic and Wingo;ESPN:1442:HM2:S0.0W:3333:0:4231:0:0:104:9:3:0
Stephen A. Smith;ESPN:1442:HM2:S0.0W:3333:0:4231:0:0:105:9:3:0
AUDI CUP;DESPN:1442:HM2:S0.0W:3333:0:4199:0:0:106:9:3:0
UEFA SUPER CUP;DESPN:1442:HM2:S0.0W:3333:0:4200:0:0:107:9:3:0
ESPN Shortform;ESPNF:1442:HM2:S0.0W:3333:0:4722:0:0:108:9:3:0
DEP MLB SUNDAY NIGHT BASEBALL;DESPN:1442:HM2:S0.0W:3333:0:4282:0:0:109:9:3:0
MLS - GAME OF THE WEEK;DESPN:1442:HM2:S0.0W:3333:0:4281:0:0:110:9:3:0
CONCACAF CHAMPIONS LEAGUE;DESPN:1442:HM2:S0.0W:3333:0:4284:0:0:111:9:3:0
PANAM;DESPN:1442:HM2:S0.0W:3333:0:4201:0:0:112:9:3:0
Test Channel;TC:1442:HM2:S0.0W:3333:0:4096:0:0:113:9:3:0
TV-ESPN-1;ESPN1:1442:HM2:S0.0W:3333:0:5562:0:0:119:9:3:0
TV-ESPN-2;ESPN2:1442:HM2:S0.0W:3333:0:5563:0:0:120:9:3:0
WEPN;WEPN:1442:HM2:S0.0W:3333:0:5568:0:0:125:9:3:0
KSPN;:1442:HM2:S0.0W:3333:0:5570:0:0:127:9:3:0
Emergency;:1442:HM2:S0.0W:3333:0:5571:0:0:128:9:3:0
UEFA WCQ;DESPN:1442:HM2:S0.0W:3333:0:5337:0:0:129:9:3:0
TV-ESPNews;ESPHD:1442:HM2:S0.0W:3333:0:4288:0:0:132:9:3:0
SILENCE;:1442:HM2:S0.0W:3333:0:5229:0:0:140:9:3:0
DEPORTES MF OVERNIGHT;DESPN:1442:HM2:S0.0W:3333:0:4207:0:0:141:9:3:0
LA LIGA SUPER COPA;DESPN:1442:HM2:S0.0W:3333:0:5592:0:0:142:9:3:0
FIRMA ESPN;DESPN:1442:HM2:S0.0W:3333:0:4207:0:0:143:9:3:0
RAZA DEPORTIVA;DESPN:1442:HM2:S0.0W:3333:0:4207:0:0:144:9:3:0
ZONA ESPN;DESPN:1442:HM2:S0.0W:3333:0:4207:0:0:146:9:3:0
COPA MX;DESPN:1442:HM2:S0.0W:3333:0:5593:0:0:147:9:3:0
JORGE RAMOS Y SU BANDA;DESPN:1442:HM2:S0.0W:3333:0:4207:0:0:148:9:3:0
DESTINO FUTBOL;DESPN:1442:HM2:S0.0W:3333:0:4207:0:0:149:9:3:0
ZONA ESPN 2ND EDICION;DESPN:1442:HM2:S0.0W:3333:0:4207:0:0:150:9:3:0
DEPORTES SAT 12M-6A;DESPN:1442:HM2:S0.0W:3333:0:4207:0:0:151:9:3:0
DEPORTES SAT 6A-12MID;DESPN:1442:HM2:S0.0W:3333:0:4207:0:0:152:9:3:0
DEPORTES SUN 12M-6A;DESPN:1442:HM2:S0.0W:3333:0:4207:0:0:153:9:3:0
DEPORTES SUN 6A-12MID;DESPN:1442:HM2:S0.0W:3333:0:4207:0:0:154:9:3:0
[MLB2] MLB Specials;MLB2:1442:HM2:S0.0W:3333:0:5572:0:0:155:9:3:0
LIGA MX 2;DESPN:1442:HM2:S0.0W:3333:0:5573:0:0:156:9:3:0
DEP MLB ALL STAR GAME;DESPN:1442:HM2:S0.0W:3333:0:5574:0:0:157:9:3:0
DEP MLB MEET THE STARS;DESPN:1442:HM2:S0.0W:3333:0:5575:0:0:158:9:3:0
INTL CHAMP CUP;DESPN:1442:HM2:S0.0W:3333:0:5576:0:0:159:9:3:0
MLS CUP;DESPN:1442:HM2:S0.0W:3333:0:5577:0:0:160:9:3:0
[MLB1] MLB Division Series;MLB1:1442:HM2:S0.0W:3333:0:4097:0:0:163:9:3:0
[NBA1] NBA Regular;NBA1:1442:HM2:S0.0W:3333:0:4098:0:0:164:9:3:0
DEP MLB LEAGUE CHAMP SERIES;DESPN:1442:HM2:S0.0W:3333:0:4099:0:0:165:9:3:0
DEP MLB WORLD SERIES;DESPN:1442:HM2:S0.0W:3333:0:4100:0:0:166:9:3:0
DEP SPAIN NATIONAL TEAM;DESPN:1442:HM2:S0.0W:3333:0:4101:0:0:167:9:3:0
[MLB1] MLB League Championships;MLB1:1442:HM2:S0.0W:3333:0:4102:0:0:168:9:3:0
[MLB2] MLB League Championships;MLB2:1442:HM2:S0.0W:3333:0:4103:0:0:169:9:3:0
[MLB1] MLB World Series;MLB1:1442:HM2:S0.0W:3333:0:4104:0:0:170:9:3:0
Paul Finebaum;FINE:1442:HM2:S0.0W:3333:0:4105:0:0:171:9:3:0
UEFA CHAMPIONS LEAGUE;DESPN:1442:HM2:S0.0W:3333:0:4106:0:0:172:9:3:0
DEP SERIE DEL CARIBE;DESPN:1442:HM2:S0.0W:3333:0:4107:0:0:173:9:3:0
[CFB1] College FB Semis;CFB1:1442:HM2:S0.0W:3333:0:4108:0:0:174:9:3:0
[CFB1] College FB Bowls;CFB1:1442:HM2:S0.0W:3333:0:4110:0:0:176:9:3:0
[CFB2] College FB Bowls;CFB2:1442:HM2:S0.0W:3333:0:4111:0:0:177:9:3:0
DEP COLLEGE FB CHAMP;DESPN:1442:HM2:S0.0W:3333:0:4112:0:0:178:9:3:0
DEP NFL PLAYOFFS;DESPN:1442:HM2:S0.0W:3333:0:4114:0:0:179:9:3:0
DEP NFL PROBOWL;DESPN:1442:HM2:S0.0W:3333:0:4115:0:0:180:9:3:0
DEPORTES EXPRESS;ESPND:1442:HM2:S0.0W:3333:0:4116:0:0:181:9:3:0
DEP MLB DIVISION SERIES;DESPN:1442:HM2:S0.0W:3333:0:4117:0:0:182:9:3:0
SUPER COPA TECATE;DESPN:1442:HM2:S0.0W:3333:0:4118:0:0:183:9:3:0
ES ASI Y PUNTO;DESPN:1442:HM2:S0.0W:3333:0:4207:0:0:184:9:3:0
[AUX1] Football Sun PxP;AUX1:1442:HM2:S0.0W:3333:0:4120:0:0:185:9:3:0
DEP FINAL FOUR;DESPN:1442:HM2:S0.0W:3333:0:4121:0:0:186:9:3:0
[MLB1] World Baseball Classic;MLB1:1442:HM2:S0.0W:3333:0:4126:0:0:187:9:3:0
[AUX1] College FB Bowls;AUX1:1442:HM2:S0.0W:3333:0:4127:0:0:188:9:3:0
MLS PLAYOFFS;DESPN:1442:HM2:S0.0W:3333:0:4130:0:0:191:9:3:0
Spare 192;:1442:HM2:S0.0W:3333:0:4131:0:0:192:9:3:0
TV-SEC;SEC:1442:HM2:S0.0W:3333:0:4132:0:0:193:9:3:0
EFL CUP;DESPN:1442:HM2:S0.0W:3333:0:4133:0:0:194:9:3:0
TV-ESPNU;ESPNU:1442:HM2:S0.0W:3333:0:4134:0:0:195:9:3:0
Orange Bowl;:1442:HM2:S0.0W:3333:0:4135:0:0:196:9:3:0
[CFB1] College FB Championship;CFB1:1442:HM2:S0.0W:3333:0:4137:0:0:198:9:3:0
Spare 199;:1442:HM2:S0.0W:3333:0:4138:0:0:199:9:3:0
DEP NFL SUPERBOWL;DESPN:1442:HM2:S0.0W:3333:0:4109:0:0:200:9:3:0
DEP NBA FINALS;DESPN:1442:HM2:S0.0W:3333:0:4129:0:0:201:9:3:0
PANAM-SOCCER;DESPN:1442:HM2:S0.0W:3333:0:4136:0:0:202:9:3:0
```

([link](https://sourceforge.net/projects/channeleditor/))

