#!/home/codespace/.python/current/bin/python3
import os
import sys
from PIL import Image, ImageFilter, ImageDraw, ImageFont

def save_image_and_process(temp_path, filename, name):
    try:
        print("<p>Opening temporary image file for processing...</p>")
        
        # Ladataan kuva väliaikaisesta tiedostosta
        with Image.open(temp_path) as img:
            print("<p>Image opened successfully.</p>")
            
            # Lisätään suodatin ja teksti
            img = img.filter(ImageFilter.BLUR)
            draw = ImageDraw.Draw(img)
            font = ImageFont.load_default()
            text_position = (10, 10)
            draw.text(text_position, name, fill="white", font=font)
            print("<p>Image processed with text overlay.</p>")
            
            # Tallennetaan muokattu kuva "edited" kansioon
            edited_directory = "./html/edited"
            os.makedirs(edited_directory, exist_ok=True)
            
            edited_filename = f"edited_{filename}"
            output_path = os.path.join(edited_directory, edited_filename)
            img.save(output_path)
            print(f"<p>Image saved to {output_path}</p>")
            return edited_filename
    except Exception as e:
        print(f"<p>Error: Could not process the image. {str(e)}</p>")

def main():
    print("Content-Type: text/html\n")
    print("<html><body><p>Python script started</p>")

    content_type = os.environ.get("CONTENT_TYPE", "")
    boundary = content_type.split("boundary=")[-1]
    content_length = int(os.environ.get("CONTENT_LENGTH", 0))
    data = sys.stdin.read(content_length)
    
    print("<p>Received data from POST request.</p>")
    parts = data.split(f"--{boundary}")
    
    for part in parts:
        if "Content-Disposition" in part:
            headers, file_data = part.split("\r\n\r\n", 1)
            file_data = file_data.rstrip("--\r\n")

            if 'filename="' in headers:
                filename = headers.split('filename="')[1].split('"')[0]
                name_field = headers.split('name="')[1].split('"')[0]
                name = name_field or "User"
                
                print(f"<p>Received file with filename: {filename}</p>")
                
                # Luodaan väliaikainen tiedosto kuvan käsittelyä varten
                temp_path = f"/tmp/{filename}"
                with open(temp_path, "wb") as f:
                    f.write(file_data.encode('latin1'))  # Tallennetaan binääridata oikein
                    print("<p>Temporary image file written.</p>")
                
                # Käsitellään kuva ja tallennetaan muokattu versio
                edited_filename = save_image_and_process(temp_path, filename, name)

                # CGI-vastaus, jossa on linkki muokattuun kuvaan
                print("<h1>Image uploaded and edited successfully!</h1>")
                print(f'<p><a href="/edited/{edited_filename}">Download edited image</a></p>')
                print("</body></html>")
                
                # Poistetaan väliaikainen tiedosto lopuksi
                os.remove(temp_path)
                print("<p>Temporary file removed.</p>")
                return

if __name__ == "__main__":
    main()
