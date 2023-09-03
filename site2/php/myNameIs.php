<!DOCTYPE html>
<html>
<head>
	<title>PHP test</title>
</head>

<body>
	<div>
		<?php if (isset($_GET["firstname"]) && isset($_GET["firstname"]))
			echo 'Your name is <b>' . $_GET["firstname"] . " " . $_GET["surname"] . '</b>!';
		else
			echo 'Tell me your name by appending the url with: "?firstname=*yourFirstName*&surname=*yourSurname*"'?>
	</div>
</body>
</html>
