<?php

include "db.php";

if(!isset($_SESSION['user'])) {
	header("location: login.php?url=".$_SERVER["PHP_SELF"]);
	exit(0);
}


date_default_timezone_set( 'Asia/Shanghai');

if (!isset($_SESSION["jiupian"]))
	$_SESSION["jiupian"]=1;

$span = 5;
if(isset($_REQUEST['span']))
	$span = $_REQUEST['span'];

if($span<=0) $span = 5;
if($span>60) $span = 5;
$startdate=time() - $span * 60;  
$startdatestr=date("Y-m-d H:i:s",$startdate);

if (isset($_REQUEST["tm"])) {
	$cmd="tm";
	$tm=$_REQUEST["tm"];
} else {
	$cmd="map";
}

$jiupian = 0; 	// 0 不处理
		// 1 存储的是地球坐标，转换成baidu显示, 默认情况
		// 2 存储的是火星坐标，转换成baidu显示

if ( ($cmd=="map") || ($cmd=="tm")) {
	$jiupian = 1;
	if (isset($_SESSION["jiupian"]))
		$jiupian=$_SESSION["jiupian"];
}

if($jiupian>0) {
	require "wgtochina_baidu.php";
	$mp=new Converter();
}

function urlmessage($aid,$icao, $alt, $speed, $h, $vr , $icon, $dtmstr) {
        $m = "<font face=微软雅黑 size=2><img src=".$icon."> ".$aid." <a href=".$_SERVER["PHP_SELF"]."?aid=".$aid." target=_blank>数据包</a> <a id=\\\"m\\\" href=\\\"#\\\" onclick=\\\"javascript:monitor_station('".$aid."');return false;\\\">";
        $m = $m."切换跟踪</a> ";
        $m =$m."轨迹";
        $m = $m."<a href=".$_SERVER["PHP_SELF"]."?gpx=".$aid." target=_blank>GPX</a> ";
        $m = $m."<a href=".$_SERVER["PHP_SELF"]."?kml=".$aid." target=_blank>KML</a> <hr color=green>".$dtmstr."<br>";
        $m = $m."<b>海拔 ".$alt."ft<br>速度 ".$speed."kn(";
	$m = $m.sprintf("%dKPH)",$speed*1.852);
	$m = $m."/".$h."° <br>爬升 ".$vr."ft/min</b><br>";
        $m = $m."</font>";
        return $m;
}

if ($cmd=="tm") {
	global $startdatestr;

	$lon1=$_REQUEST["lon1"];
        $lon2=$_REQUEST["lon2"];
        $lat1=$_REQUEST["lat1"];
        $lat2=$_REQUEST["lat2"];

	$starttm = microtime(true);
//删除1天前的每个台站最后状态数据包
	$q="delete from aircraftlast where tm<=date_sub(now(),INTERVAL 1 day)";
	$mysqli->query($q);
	$endtm = microtime(true); $spantm = $endtm-$starttm; $startm=$endtm; echo "//".$spantm."\n";
	
	$q="select lat,lon,aid,unix_timestamp(tm),tm,icao,alt,speed,h,vr from aircraftlast where tm>=FROM_UNIXTIME(?) and tm>=? and lon >= ? and lon <= ? and lat >= ? and lat <= ? ";	
	$stmt=$mysqli->prepare($q);
       	$stmt->bind_param("isdddd",$tm,$startdatestr,$lon1,$lon2,$lat1,$lat2);
        $stmt->execute();
	$stmt->bind_result($lat,$lon,$aid,$dtm,$dtmstr,$icao,$alt,$speed,$h,$vr);

	while($stmt->fetch()) {
		if($jiupian==1) {
			$p=$mp->WGStoBaiDuPoint($lon,$lat);
			$lon = $p->getX();
			$lat = $p->getY();
		} else if($jiupian==2) {
			$p=$mp->ChinatoBaiDuPoint($lon,$lat);
			$lon = $p->getX();
			$lat = $p->getY();
		} 
		$icon = "img/".$h.".png";
                $dmsg = urlmessage($aid, $icao, $alt,$speed,$h,$vr,$icon,$dtmstr);
                echo "setstation(".$lon.",".$lat.",\"".$aid."\",".$dtm.",\"".$icon."\",\n\"".$dmsg."\");\n";
	}
	$stmt->close();
	$endtm = microtime(true); $spantm = $endtm-$starttm; $startm=$endtm; echo "//".$spantm."\n";

	if($tm==0) {
		if(isset($lon) && $lon<>0 ) 
			echo "map.centerAndZoom(new BMap.Point($lon,$lat),12);\n";
	}
	$q = "select unix_timestamp(now())";
	$result = $mysqli->query($q);
	$r=$result->fetch_array();
	echo "deleteoldstation($r[0]);\n";
	exit(0);
}

if ($cmd=="map") {  
?>
<!DOCTYPE html>
<html>
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
	<meta name="viewport" content="initial-scale=1.0, user-scalable=no" />
	<style type="text/css">
		body, html{width: 100%;height: 100%;margin:0;font-family:"微软雅黑";}
		#allmap {height:100%; width: 100%;}
		#menu {position: fixed; background-color:blue; left: 10px; bottom: 30px; color: white}
	</style>
	<title>ADSB</title>
	<script type="text/javascript" src="http://api.map.baidu.com/api?v=2.0&ak=7RuEGPr12yqyg11XVR9Uz7NI"></script>
</head>
<body>
<div id="allmap"></div>
<div id=menu>
<input type=checkbox checked id=disp_pathpoints onclick="change_pathpoints(this);">显示航点</input>&nbsp; 
显示老化<select id=path_time onchange="span=this.options[this.options.selectedIndex].value;">
<option value=1>1分钟</option>
<option value=5 selected>5分钟</option>
<option value=10>10分钟</option>
</select>
</div>
</body>
</html>
<script type="text/javascript">
var markers = {};
var lasttms = {};
var iconurls = {};
var infowindows = {};
var lasttm=0;
var movepaths = {};
var polylines = {};
var movepoints = {};
var display_pathpoints = true;
var span = 5;

var colors = ["#000000", "#1400FF","#14F0FF","#78FF00","#FF78F0","#0078F0","#F0FF14","#FF78F0","#FF78F0","#FF78F0"];

function getcolor(label){
	var colorindex = 0;
	for(var i=0;i<label.length;i++)
		colorindex += label.charCodeAt(i);
	colorindex = colorindex % colors.length;
	return colors[colorindex];
}

function updatecalls(calls) {
	// console.log("calls:"+calls);
}

function updatepkts(pkts) {
	// console.log("pkts:"+pkts);
}

function addp(aid,lon,lat,msg) {
	var icon = new BMap.Icon("p.png", new BMap.Size(3, 3));	
	var m = new BMap.Marker(new BMap.Point(lon,lat), {icon: icon});
	var infowindow = new BMap.InfoWindow(msg, {width:300});
	(function(){
        	m.addEventListener('click', function(){
            	this.openInfoWindow(infowindow);
        	});
	})();
	map.addOverlay(m);
	return m;
}

function change_pathpoints()
{
	if(!display_pathpoints) {
		display_pathpoints = true;
		return;
	}
	for ( aid in lasttms ) {
		for(i=0;i<movepoints[aid].length;i++) 
			map.removeOverlay(movepoints[aid][i]);
		movepoints[aid].splice(0, movepoints[aid].length);
	}
	display_pathpoints = false;
}

function deleteoldstation(tm)
{	for ( aid in lasttms ) {
		if( lasttms[aid] < tm - span*60) {  // too old, delete it
			console.log(aid);	
			map.removeOverlay(polylines[aid]);
			delete polylines[aid];
			map.removeOverlay(markers[aid]);
			delete markers[aid];
			delete infowindows[aid];
			delete iconurls[aid];
			delete movepaths[aid];
			delete lasttms[aid];
			for(i=0;i<movepoints[aid].length;i++) 
				map.removeOverlay(movepoints[aid][i]);
			delete movepoints[aid];
		}
	}
}

function setstation(lon, lat, label, tm, iconurl, msg)
{	
	if(markers.hasOwnProperty(label)) {   // call已经存在
		if(tm<=lasttms[label]) return; // 同一时间的点已经更新过了（假定每秒最多传回来一个点，第二个点不再处理）
		markers[label].setPosition( new BMap.Point(lon, lat) ); // 更改最后的点位置
		infowindows[label].setContent(msg);
		if(iconurls[label]!=iconurl) {
			var nicon = new BMap.Icon(iconurl, new BMap.Size(24, 24), {anchor: new BMap.Size(12, 12)});
			markers[label].setIcon(nicon);
			iconurls[label]=iconurl;
		}
		lasttms[label] = tm;
		if(tm>lasttm) lasttm = tm;
		// 更新航迹
		var p = new BMap.Point(lon,lat);
		movepaths[label].push (p);
		polylines[label].setPath(movepaths[label]);
		if(display_pathpoints) {
			m = addp(label,lon,lat,msg);
			movepoints[label].push(m);
		}
		return;
	}
	// 新call
	var icon = new BMap.Icon(iconurl, new BMap.Size(24, 24), {anchor: new BMap.Size(12, 12)});	
	var marker = new BMap.Marker(new BMap.Point(lon,lat), {icon: icon});
	var lb = new BMap.Label(label, {offset: new BMap.Size(20,-10)});
	lb.setStyle({border:0, background: "#eeeeee"});
	marker.setLabel(lb);
	var infowindow = new BMap.InfoWindow(msg, {width:300});
	(function(){
        	marker.addEventListener('click', function(){
            	this.openInfoWindow(infowindow);
        	});
	})();
	map.addOverlay(marker);
	markers[label]= marker;
	lasttms[label] = tm;
	iconurls[label]=iconurl;
	infowindows[label]=infowindow;
	if(tm>lasttm) lasttm = tm;
	// 处理航迹
	var p = new BMap.Point(lon,lat);
	movepaths[label] = new Array();
	movepaths[label].push (p);
	polylines[label] = new Array();
	polylines[label] = new BMap.Polyline(movepaths[label],{strokeColor:getcolor(label), strokeWeight:4, strokeOpacity:0.9});
	map.addOverlay(polylines[label]);
	movepoints[label] = new Array();
	if(display_pathpoints) {
		m = addp(label,lon,lat,msg);
		movepoints[label].push(m);
	}
}

var xmlHttpRequest;     //XmlHttpRequest对象     
function createXmlHttpRequest(){     
	var http_request = false;
	if (window.XMLHttpRequest) { // Mozilla, Safari,...
		http_request = new XMLHttpRequest();
		if (http_request.overrideMimeType) {
			http_request.overrideMimeType('text/xml');
		}
        } else if (window.ActiveXObject) { // IE
		try {
			http_request = new ActiveXObject("Msxml2.XMLHTTP");
		} catch (e) {
			try {
				http_request = new ActiveXObject("Microsoft.XMLHTTP");
			} catch (e) {}
		}
        }
        if (!http_request) {
		alert('Giving up :( Cannot create an XMLHTTP instance');
		return false;
        }
	return http_request;
}     

function UpdateStation(){     
//	alert(lastupdatetm);
	var b = map.getBounds();
        var url = window.location.protocol+"//"+window.location.host+"/"+window.location.pathname+"?tm="+lasttm+"&span="+span+"&lon1="+b.getSouthWest().lng+"&lat1="+b.getSouthWest().lat+"&lon2="+b.getNorthEast().lng+"&lat2="+b.getNorthEast().lat;
        //1.创建XMLHttpRequest
        xmlHttpRequest = createXmlHttpRequest();     
        //2.设置回调函数     
        xmlHttpRequest.onreadystatechange = UpdateStationDisplay;
        //3.初始化XMLHttpRequest组建     
        xmlHttpRequest.open("post",url,true);     
        //4.发送请求     
        xmlHttpRequest.send(null);
}

//回调函数     
function UpdateStationDisplay(){     
        if(xmlHttpRequest.readyState == 4){
		if(xmlHttpRequest.status == 200){  
        		var b = xmlHttpRequest.responseText;  
			eval(b);
		}
       	   	setTimeout("UpdateStation();","2000");  
        }     
}    

function centertocurrent(){
	var geolocation = new BMap.Geolocation();
	geolocation.getCurrentPosition(function(r){
		if(this.getStatus() == BMAP_STATUS_SUCCESS){
			map.centerAndZoom(r.point,12);
	}},{enableHighAccuracy: false});
}

// 百度地图API功能
var map = new BMap.Map("allmap");
map.enableScrollWheelZoom();

var top_left_control = new BMap.ScaleControl({anchor: BMAP_ANCHOR_TOP_LEFT});// 左上角，添加比例尺
var top_left_navigation = new BMap.NavigationControl();  //左上角，添加默认缩放平移控件
	
//添加控件和比例尺
map.addControl(top_left_control);        
map.addControl(top_left_navigation);     
map.addControl(new BMap.MapTypeControl());
map.centerAndZoom(new BMap.Point(108.940178,34.5), 6);

UpdateStation();  
</script>
<?php
	exit(0);
}
?>
