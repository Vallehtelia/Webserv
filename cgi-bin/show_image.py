#!/usr/bin/env python3
import os
import sys

def get_most_recent_image(directory):
    files = [os.path.join(directory, f) for f in os.listdir(directory) if f.lower().endswith(('.png', '.jpg', '.jpeg', '.gif'))]
    if not files:
        return None
    return max(files, key=os.path.getmtime)

def main():
    image_directory = './html/edited/'
    image_path = get_most_recent_image(image_directory)

    if image_path:
        content_type = "image/jpeg" if image_path.lower().endswith(".jpg") or image_path.lower().endswith(".jpeg") else "image/png"
        print(f"Content-Type: {content_type}\n")

        # Luetaan ja tulostetaan binääridata
        with open(image_path, "rb") as img_file:
            sys.stdout.buffer.write(img_file.read())
    else:
        print("Content-Type: text/html\n")
        print('<html><body><h1>No images found</h1></body></html>')

if __name__ == "__main__":
    main()
