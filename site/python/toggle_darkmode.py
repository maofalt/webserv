from os import environ

darkmode = "on"

try:
	for cookie in map(str.strip, environ['HTTP_COOKIE'].split(';')):
		(key, value) = cookie.split('=')
		if (key == "darkmode" and value=="on"):
			darkmode = "off"
except KeyError:
	darkmode = "on"

print ("204\r\n", end='')
print ("Set-Cookie:darkmode=%s; Path=/\r\n" % darkmode, end='')
print ("\r\n", end='')
