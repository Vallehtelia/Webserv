#!/usr/bin/env python3

import cgi
import cgitb

cgitb.enable()

print("Content-Type: text/html\n")

form = cgi.FieldStorage()

username = form.getvalue("username")
message = form.getvalue("message")

print("<html><body>")
print("<h2>POST Request Received</h2>")
print(f"<p>Username: {username}</p>")
print(f"<p>Message: {message}</p>")
print("</body></html>")
