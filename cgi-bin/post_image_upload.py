#!/usr/bin/env python3

import cgi
import os
import cgitb
from pathlib import Path

cgitb.enable()  # Debuggausta varten

# Määritellään tallennuskansio
upload_dir = "./html/uploads"

# Luodaan CGI form -olio
form = cgi.FieldStorage()

# Varmistetaan että kuvatiedosto on lähetetty
if "image" not in form or not form["image"].filename:
    print("Content-Type: text/html")
    print()
    print("<html><body><h1>Error: No file uploaded</h1></body></html>")
else:
    # Noudetaan tiedosto ja sen nimi
    file_item = form["image"]
    filename = os.path.basename(file_item.filename)

    # Varmistetaan että tallennuskansio on olemassa
    Path(upload_dir).mkdir(parents=True, exist_ok=True)

    # Määritellään tallennuspolku
    file_path = os.path.join(upload_dir, filename)

    # Tallennetaan tiedosto
    with open(file_path, "wb") as output_file:
        output_file.write(file_item.file.read())

    # Palautetaan HTML-vastaus
    print("Content-Type: text/html")
    print()
    print(f"<html><body><h1>File uploaded successfully!</h1><p>Saved to: {file_path}</p></body></html>")
