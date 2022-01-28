<?php
error_reporting(E_ALL ^ E_NOTICE);
$current_date = date("Y-m-d H:i:s");//what is the time right now?
use PHPMailer\PHPMailer\PHPMailer;

require 'Exception.php';
require 'PHPMailer.php';
require 'SMTP.php';



//USER DEFINED VARIABLES
$house_email="admin@admin.com";
$equipment_max=90;
$servername = "127.0.0.1:3307";
$username = "root";
$password = "password";
$dbname = "db_name";
# EMAIL Message Subject 
$emailsubject="Equipment Cabinet Monitor";
# EMAIL Message Body
$msg = ""; 
$msg .= "".$current_date." - The Average Equipment Cabinet Temperature has exceeded ".$equipment_max." degrees F.".$eol; 
$msg .= " Check the Status of the Cooling System".$eol.$eol; 


// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);
// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
} 

$sql = "SELECT * FROM `equipmentmail` WHERE 1";
$result = $conn->query($sql);//what was the saved time if time was saved?

if ($result->num_rows > 0) {//if time was saved because the timer has been started

	$row = $result->fetch_assoc();
	$dteStart = new DateTime($row["datetime"]); 
	$dteEnd   = new DateTime($current_date);
	$interval = $dteStart->diff($dteEnd); //calculate the elapsed time between when the timer started and the current time
	
	if ($interval->format("%h") >0){
		
		//1 hour elapsed, clear out table to start 1 hour timer again
		print "More than 1 hour has elapsed since the last email was sent. Sending new notification email";
		$sql = "TRUNCATE TABLE `equipmentmail`";
		$result = $conn->query($sql);
		//send email indicating 1 hour elapsed and issue is still present

		# To Email Address 
		$emailaddress=$house_email;  

		# Common Headers 
		$headers .= 'From: <'.$house_email.'>'.$eol; 
		$headers .= 'Reply-To: '.$house_email.''.$eol; 
		$headers .= 'Return-Path: '.$house_email.'>'.$eol;     // these two to set reply address 
		$headers .= "Message-ID:<".$now." TheSystem@".$_SERVER['SERVER_NAME'].">".$eol; 
		$headers .= "X-Mailer: PHP v".phpversion().$eol;           // These two to help avoid spam-filters 
		# Boundry for marking the split & Multitype Headers 
		$mime_boundary=md5(time()); 
		$headers .= 'MIME-Version: 1.0'.$eol; 
		$headers .= "Content-Type: multipart/related; boundary=\"".$mime_boundary."\"".$eol; 

		# SEND THE EMAIL  
		if(mail($emailaddress, $emailsubject, $msg, $headers)){
			print "<br>E-Mail successfully sent";
		}else{
			print "<br>E-Mail could not be sent, check system settings and network connection / status";
		}
		
		//because no timer data is saved, save new timer data
		$sql = "INSERT INTO `equipmentmail` (data) VALUES(1)";
		$result = $conn->query($sql);
		
	}else{
		print "Notification E-Mail was sent less than 1 hour ago, test email will only send once per hour";
	}
} else {
		print "Sending new notification email";
		# To Email Address 
		$emailaddress=$house_email;  

		# Common Headers 
		$headers .= 'From: <'.$house_email.'>'.$eol; 
		$headers .= 'Reply-To: '.$house_email.''.$eol; 
		$headers .= 'Return-Path: '.$house_email.'>'.$eol;     // these two to set reply address 
		$headers .= "Message-ID:<".$now." TheSystem@".$_SERVER['SERVER_NAME'].">".$eol; 
		$headers .= "X-Mailer: PHP v".phpversion().$eol;           // These two to help avoid spam-filters 
		# Boundry for marking the split & Multitype Headers 
		$mime_boundary=md5(time()); 
		$headers .= 'MIME-Version: 1.0'.$eol; 
		$headers .= "Content-Type: multipart/related; boundary=\"".$mime_boundary."\"".$eol; 

		# SEND THE EMAIL 
		if(mail($emailaddress, $emailsubject, $msg, $headers)){
		  print "<br>E-Mail sucessfully sent";
		}else{
		  print "<br>E-Mail could not be sent, check system settings and network connection / status";
		} 
		//because no timer data is saved, save new timer data
		$sql = "INSERT INTO `equipmentmail` (data) VALUES(1)";
		$result = $conn->query($sql);
}
$conn->close();
?>