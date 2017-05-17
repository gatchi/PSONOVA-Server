<?php
	/* Created by Soly (github.com/Solybum) */
	
	function db_connect() 
	{
		$hostname = "localhost";
		$username = "php";
		$password = "password";
		$database = "pso";
	
		// Define connection as a static variable, to avoid connecting more than once 
		static $connection;

		// Try and connect to the database, if a connection has not been established yet
		if(!isset($connection)) 
		{
			$connection = mysqli_connect($hostname, $username, $password, $database);
		}
		
		// If connection was not successful, handle the error
		if($connection === false) 
		{
			return mysqli_connect_error(); 
		}
		return $connection;
	}
	
	function db_query($query) 
	{
		// Connect to the database
		$connection = db_connect();

		// Query the database
		$result = mysqli_query($connection, $query);
		return $result;
	}
	
	function db_error() 
	{
		$connection = db_connect();
		return mysqli_error($connection);
	}
	
	function db_escape($value) 
	{
		$connection = db_connect();
		return mysqli_real_escape_string($connection, $value);
	}
?>