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
X.X.X.X 是运行dump1090的机器，设置为avr输出
nc X.X.X.X 31001 | ./adsb 



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
