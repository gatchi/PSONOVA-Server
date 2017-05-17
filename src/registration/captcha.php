<?php
	/* Created by Soly (github.com/Solybum) */
	 
	function getCaptchaResult($ip, $response)
	{
		if ($response == null || strlen($response) == 0) 
		{
            return null;
        }
		
		$url = "https://www.google.com/recaptcha/api/siteverify";
		$data = array
		(
			"secret" => "your rivate key, don't share it",
			"remoteip" => $ip,
			"response" => $response,
		);
		
		// use key 'http' even if you send the request to https://...
		$options = array(
			'http' => array(
				'header'  => "Content-type: application/x-www-form-urlencoded\r\n",
				'method'  => 'POST',
				'content' => http_build_query($data),
			),
		);
		$context  = stream_context_create($options);
		$result = file_get_contents($url, false, $context);
		$answers = json_decode($result, true);
		
        if (trim($answers ['success']) == true)
		{
            return true;
        }
		else
		{
			return false;
        }
	}
?>