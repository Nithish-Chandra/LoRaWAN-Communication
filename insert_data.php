<?php
// Database credentials
 
$servername = " localhost" ,
$username = "root"; $password = 
$dbname = "sensor db";
// Get temperature and pressure values from POST data
$temperature = $ POST['temperature'],
$pressure = $ POST['pressure'], $humidity = $ POST['humidity'],
// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);
 Check connection if ($conn->connect_error) { die("Connection failed: " $conn->connect_error);
// Prepare and execute the SQL query
$sql = "INSERT INTO LoRawan_final (temperature, pressure,humidity) VALUES ('$temperature',
' $pressure' $humidity') " ;
if ($conn->query($sql) TRUE) { echo "Data inserted successfully" ,
} else { echo "Error. $sql . "<br>" . $conn->error;
$conn->close();
?>
