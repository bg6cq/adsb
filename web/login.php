<?php

include "db.php";

if(isset($_REQUEST['user'])) {
	$user = $_REQUEST['user'];
	$pass = $_REQUEST['pass'];
	$q = "select user from user where user= ? and pass = md5(?)";
	$stmt=$mysqli->prepare($q);
        $stmt->bind_param("ss",$user,$pass);
        $stmt->execute();
        $stmt->bind_result($user);
	if($stmt->fetch()) { //  got user
		$_SESSION['user'] = $user;
	}
	$stmt->close();
	$q = "insert into userlog values(now(), ? ,? )";
	$stmt=$mysqli->prepare($q);
	$ip = $_SERVER['REMOTE_ADDR'];
        $stmt->bind_param("ss",$user,$ip);
        $stmt->execute();
}

if(!isset($_SESSION['user'])) {
?>

您好：<p>

请输入用户和密码验证身份：<br>
<form action=login.php method=post>
用户：<input name=user><p>
密码：<input name=pass type=password><p>
<?php echo "<input name=url type=hidden value=\"".$_SERVER["PHP_SELF"]."\">"; ?>
<input type=submit value='登录'>

<?php
	exit(0);
}
	if(isset($_REQUEST['url']))
		header("location: ".$_REQUEST['url']); 
	else 
		header("location: f.php"); 
?>
