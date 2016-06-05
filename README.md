ADS-B 解码程序<p>

使用了以下项目的资料和代码，感谢原作者<br>
http://adsb-decode-guide.readthedocs.io<br>
https://github.com/etabbone/01.ADSB_BSBv6_UPS<br>
http://www.lll.lu/~edward/edward/adsb/DecodingADSBposition.html<p>


将 web 目录设置在 http服务器可见，如<br>
ln -s /usr/src/adsb/web /var/www/html/addsb


<pre>

使用方法：
mkdir /var/log/adsb     用来保存收到的数据包

运行 adsb 服务程序后，
在dump1090机器上执行
./dump1090 --raw | nc X.X.X.X 31001 
X.X.X.X 是运行adsb服务程序的机器



数据库创建命令：

create database adsb;

CREATE TABLE `aircraftlast` (
  `tm` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `icao` char(6) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `aid` char(8) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `lat` double NOT NULL,
  `lon` double NOT NULL,
  `alt` int(11) NOT NULL,
  `speed` float NOT NULL,
  `h` int(11) NOT NULL,
  `vr` int(11) NOT NULL,
  PRIMARY KEY (`icao`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `aircraftlog` (
  `tm` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `icao` char(6) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `aid` char(8) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
  `lat` double NOT NULL,
  `lon` double NOT NULL,
  `alt` int(11) NOT NULL,
  `speed` float NOT NULL,
  `h` int(11) NOT NULL,
  `vr` int(11) NOT NULL,
  KEY `tm` (`tm`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE `user` (
  `user` varchar(20) DEFAULT NULL,
  `pass` varchar(200) DEFAULT NULL
) ENGINE=MyISAM;

CREATE TABLE `userlog` (
  `tm` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `user` varchar(20) DEFAULT NULL,
  `IP` varchar(100) DEFAULT NULL
) ENGINE=MyISAM;

insert into user values('username',md5('password'));

</pre>


树莓派dump1090安装
<pre>
http://www.satsignal.eu/raspberry-pi/dump1090.html


1. https://www.raspberrypi.org/downloads/raspbian/ RASPBIAN JESSIE LITE
2. https://www.raspberrypi.org/documentation/installation/installing-images/README.md
3. login as pi, raspberry
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install git-core git cmake libusb-1.0.0-dev build-essential
git clone git://git.osmocom.org/rtl-sdr.git
cd rtl-sdr
mkdir build
cd build
cmake ../ -DINSTALL_UDEV_RULES=ON
make

sudo make install
sudo ldconfig

sudo cp ./rtl-sdr/rtl-sdr.rules /etc/udev/rules.d/

sudo vi /etc/modprobe.d/no-rtl.conf     Add the following lines to the new file:
blacklist dvb_usb_rtl28xxu
blacklist rtl2832
blacklist rtl2830

sudo reboot

rtl_test -t will see the RTL2832U

4. 校准频率
mkdir ~/kal
cd ~/kal
sudo apt-get install libtool autoconf automake libfftw3-dev
git clone https://github.com/asdil12/kalibrate-rtl.git
cd kalibrate-rtl
git checkout arm_memory		# Essential for the Raspberry Pi
./bootstrap
./configure
make
sudo make install

kal -s GSM900 -d 0 -g 40  找出功率最高的channel
然后
kal -c <channel> -d 0 -g 40
校准

5. 
cd ~
git clone git://github.com/MalcolmRobb/dump1090.git
cd dump1090
make  (如果错误，执行 sudo apt-get install pkg-config 后再make)

./dump1090 --raw | nc x.x.x.x 33001
</pre>
