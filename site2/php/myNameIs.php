<!DOCTYPE html>
<html>
<head>
	<title>PHP test</title>
</head>

<body>
	<div>
		<?php if (isset($_GET["firstname"]) && isset($_GET["firstname"]))
			echo '<p>Your name is <b>' . $_GET["firstname"] . " " . $_GET["lastname"] . '</b>!</p>';
		else
		{
			echo '<p>Tell me your name by appending the url with: "?firstname=*yourFirstName*&lastname=*yourSurname*"</p>';
			echo '<p>Or use the following form which uses GET method:</p>';
			echo '<form action="myNameIs.php" method="GET">';
			echo '<p>	Firstname: <input name="firstname"/></p>';
			echo '<p>	Lastname: <input name="lastname"/></p>';
			echo '<p>	<button>GO!</button></p>';
			echo '</form>';
		}?>
	</div>
</body>
</html>
