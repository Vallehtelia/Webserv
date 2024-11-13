#!/usr/bin/env python3

import os
import cgi
import cgitb

# Ota käyttöön CGI-virheenkäsittely (vain kehityskäyttöön)
cgitb.enable()

# Asetetaan HTTP-otsikko, jonka täytyy olla ensimmäinen tuloste CGI-ohjelmassa
print("Content-Type: text/html")
print()  # Tyhjä rivi erottaa otsikot sisällöstä

# HTML-sisältöä
print("<html>")
print("<head><title>CGI Test</title></head>")
print("<body>")
print("<h1>Hello, World!</h1>")

# Tarkistetaan, onko kyselyssä dataa (esim. POST-dataa)
form = cgi.FieldStorage()
if "name" in form:
    name = form.getvalue("name")
    print(f"<p>Hello, {name}!</p>")
else:
    print("<p>No name provided.</p>")

print("</body>")
print("</html>")
