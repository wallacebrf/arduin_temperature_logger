<?php
error_reporting(E_ALL ^ E_NOTICE);
$measurement="House_Temp";

$influxdb_host="localhost";
$influxdb_port=8086;
$influxdb_name="db_name";
$influxdb_pass="db_pass";
$house_equipment_script_enable=1;
$sensor_name="influd_db_sensor_name";
$equipment_max=90];

if ($house_equipment_script_enable==1){

	
	$middle_temp_whole=(filter_var($_GET['middle_temp_whole'], FILTER_SANITIZE_NUMBER_INT));
	$middle_temp_fract=(filter_var($_GET['middle_temp_fract'], FILTER_SANITIZE_NUMBER_INT));
	$middle_temp_status=(filter_var($_GET['middle_temp_status'], FILTER_SANITIZE_NUMBER_INT));
	$cold_side_temp_whole=(filter_var($_GET['cold_side_temp_whole'], FILTER_SANITIZE_NUMBER_INT));
	$cold_side_temp_fract=(filter_var($_GET['cold_side_temp_fract'], FILTER_SANITIZE_NUMBER_INT));
	$cold_side_temp_status=(filter_var($_GET['cold_side_temp_status'], FILTER_SANITIZE_NUMBER_INT));
	$cold_badsensorcount=(filter_var($_GET['cold_badsensorcount'], FILTER_SANITIZE_NUMBER_INT));
	$middle_badsensorcount=(filter_var($_GET['middle_badsensorcount'], FILTER_SANITIZE_NUMBER_INT));
	
	$sensor1_influx=$middle_temp_whole+($middle_temp_fract/100);
	$sensor2_influx=$cold_side_temp_whole+($cold_side_temp_fract/100);
	$sensor1_sensor2_average_influx=($sensor1_influx+$sensor2_influx)/2;
	
	$post_url="".$measurement.",sensor_name=$sensor_name sensor1=$sensor1_influx,sensor2=$sensor2_influx,average=$sensor1_sensor2_average_influx";


	$output = shell_exec('curl -XPOST "http://'.$influxdb_host.':'.$influxdb_port.'/api/v2/write?bucket='.$influxdb_name.'&org=home" -H "Authorization: Token '.$influxdb_pass.'" --data-raw "'.$post_url.'"');
	echo "<pre>$output</pre>";


	print $sensor1_sensor2_average_influx;
	if ($sensor1_sensor2_average_influx >$equipment_max){
		include_once 'equipment_cabinet_mail.php';
		print "over temp - sending email";
	}else{
		print "under temp - everything is good";
	}
}
?>
