<?php
//requires library: https://github.com/influxdata/influxdb-client-php#install-the-library

//***************************************
//USER VARIABLES
//***************************************
$measurement="House_Temp_Hum";
$email_file_name="equipment_cabinet_mail.php";
$config_file_location="/volume1/web/config/config_files/config_files_local/house_config.txt";


//***************************************
//START OF CODE
//***************************************

error_reporting(E_ALL ^ E_NOTICE);
$data = file_get_contents("".$config_file_location."");
$pieces = explode(",", $data);
$influxdb_host=$pieces[12];
$influxdb_port=$pieces[13];
$influxdb_name=$pieces[14];
$influxdb_user=$pieces[15];
$influxdb_pass=$pieces[16];
$script_enable=$pieces[17];
$sensor_name=$pieces[18];
$max_temp=$pieces[19];
include $_SERVER['DOCUMENT_ROOT']."/functions.php";
require $_SERVER['DOCUMENT_ROOT']."/admin/vendor/autoload.php";

use InfluxDB2\Client;
use InfluxDB2\Point;

$generic_error="";

if ($script_enable==1){
	
	
	[$middle_temp_whole, $generic_error] = test_input_processing($_GET['middle_temp_whole'], 0, "numeric", 0, 150);
	[$middle_temp_fract, $generic_error] = test_input_processing($_GET['middle_temp_fract'], 0, "numeric", 0, 100);
	[$middle_temp_status, $generic_error] = test_input_processing($_GET['middle_temp_status'], 0, "numeric", 0, 1);
	[$cold_side_temp_whole, $generic_error] = test_input_processing($_GET['cold_side_temp_whole'], 0, "numeric", 0, 150);
	[$cold_side_temp_fract, $generic_error] = test_input_processing($_GET['cold_side_temp_fract'], 0, "numeric", 0, 100);
	[$cold_side_temp_status, $generic_error] = test_input_processing($_GET['cold_side_temp_status'], 0, "numeric", 0, 1);
	[$cold_badsensorcount, $generic_error] = test_input_processing($_GET['cold_badsensorcount'], 0, "numeric", 0, 4294967296);
	[$middle_badsensorcount, $generic_error] = test_input_processing($_GET['middle_badsensorcount'], 0, "numeric", 0, 4294967296);
	
	$sensor1_influx=$middle_temp_whole+($middle_temp_fract/100);
	$sensor2_influx=$cold_side_temp_whole+($cold_side_temp_fract/100);
	$sensor1_sensor2_average_influx=($sensor1_influx+$sensor2_influx)/2;
	
	$post_url="".$measurement.",sensor_name=$sensor_name sensor1=$sensor1_influx,sensor2=$sensor2_influx,average=$sensor1_sensor2_average_influx";



$client = new InfluxDB2\Client(["url" => "http://".$influxdb_host.":".$influxdb_port."", "token" => "".$influxdb_pass."",
    "bucket" => "".$influxdb_name."",
    "org" => "home",
    "precision" => InfluxDB2\Model\WritePrecision::NS
]);
$write_api = $client->createWriteApi();
$write_api->write($post_url);
$write_api->close();
$client->close();


	print $sensor1_sensor2_average_influx;
	if ($sensor1_sensor2_average_influx >$max_temp){
		include_once ''.$email_file_name.'';
		print "over temp";
	}else{
		print "under temp";
	}
}
?>
