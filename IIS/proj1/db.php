<?php

$DB_USER = "iis";
$DB_PASS = "iis";
$DB_NAME = "iis";
$DB_HOST = "localhost";

$mysqli = new mysqli($DB_HOST, $DB_USER, $DB_PASS, $DB_NAME);

if($mysqli->connect_errno || !$mysqli->set_charset("utf8")) {
    echo "Failed to connect to MySQL: (" . $mysqli->connect_errno . ") "
        . $mysqli->connect_error;
    exit();
}


?>
