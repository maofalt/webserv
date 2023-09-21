from os import environ

print ("""<!DOCTYPE html>
<html>
<head>
	<meta charset="utf-8" />
	<title>Webserv</title>
	<link rel="stylesheet" href="css/style_index.css" />
	<link rel="icon\" type="image/png" href="motero.png" />
</head>

<body>
	<header>
		<h1>Your cookies</h1>
		<ul>""")

try:
	for cookie in map(str.strip, environ['HTTP_COOKIE'].split(';')):
		print ("			<li>%s</li>" % cookie)
except KeyError:
	print("			<p>No cookies, sad.</p>")

print ("""		</ul>
	</header>

	<footer>
		<p>Rgarrigo&nbsp;&nbsp;&nbsp;Motero&nbsp;&nbsp;&nbsp;Znogueir</p>
	</footer>
</body>

</html>""")
