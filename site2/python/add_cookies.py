import cgi, cgitb

form = cgi.FieldStorage()

for i in range(0, 5):
	if (form.getvalue('cookie_{}'.format(i))):
		print ("Set-Cookie:%s\r\n" % form.getvalue('cookie_{}'.format(i)), end='')
print ("ContentType:text/html\r\n", end='')
print ("\r\n", end='')

print ("""<!DOCTYPE html>
<html>
<head>
	<meta charset="utf-8" />
	<title>Webserv</title>
	<link rel="stylesheet" href="css/style_index.css" />
	<link rel="icon\" type="image/png" href="motero.png" />
</head>

<body>
	<p>Cookies added</p>
</body>

</html>""")
