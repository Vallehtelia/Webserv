#!/usr/bin/env python3
import os
import sys
import re
from PIL import Image, ImageFilter, ImageDraw, ImageFont, ImageStat

def get_contrasting_color(avg_color):
    return tuple(255 - int(c) for c in avg_color)

def save_image_and_process(temp_path, filename, name):
    try:
        with Image.open(temp_path) as img:
            if img.mode == 'P':
                img = img.convert("RGB")


            avg_color = ImageStat.Stat(img).mean[:3]
            text_color = get_contrasting_color(avg_color)
            img = img.filter(ImageFilter.EMBOSS)

            if name:
                nameLength = len(name)
            else:
                nameLength = 1
            draw = ImageDraw.Draw(img)
            font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", img.width // nameLength * 4 // 3)
            text_width, text_height = draw.textsize(name, font=font)
            text_position = (img.width // 2 - text_width // 2, img.height - text_height - 10)

            draw.text(text_position, name, fill=text_color, font=font)
            print("<p>Image processed with text overlay.</p>")

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
    content_type = os.environ.get("CONTENT_TYPE", "")
    boundary = content_type.split("boundary=")[-1]
    content_length = int(os.environ.get("CONTENT_LENGTH", 0))
    data = sys.stdin.buffer.read(content_length)

    parts = data.split(f"--{boundary}".encode())

    for part in parts:
        # Skip empty parts
        if not part.strip():
            continue

        if b"Content-Disposition" in part:
            headers_end = part.rfind(b"\r\n\r\n")
            headers = part[:headers_end].decode("utf-8", errors="replace")
            file_data = part[headers_end + 4:]

            if 'filename="' in headers:
                filename = re.search(r'filename="(.+?)"', headers).group(1)
                name_index = part.find(b"\r\n\r\n")
                name_match = part[name_index + 4:].split(b"\r\n")[0].decode("utf-8", errors="replace")
                name = name_match

                file_data = file_data.split(b"\r\n--")[0]

                temp_path = f"/tmp/{filename}"
                with open(temp_path, "wb") as f:
                    f.write(file_data)

                if os.path.exists(temp_path):
                    edited_filename = save_image_and_process(temp_path, filename, name)
                    print("<h1>Image uploaded and edited successfully!</h1>")
                    os.remove(temp_path)
                else:
                    print("<p>Error: Temporary file could not be created.</p>")

                print("</body></html>")
                return

if __name__ == "__main__":
    main()
