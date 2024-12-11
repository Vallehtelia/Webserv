#!/usr/bin/env python3
from PIL import Image
import os
import sys
import json
import tempfile
import io

UPLOAD_FOLDER = './website/uploads/'
TEMP_FOLDER = './website/temp/'

def parse_multipart_data():
    """ Parse raw multipart form data from stdin """
    boundary = None
    content_type = os.environ.get("CONTENT_TYPE", "")
    content_length = int(os.environ.get("CONTENT_LENGTH", 0))

    # Extract boundary from the content type
    if 'boundary=' in content_type:
        boundary = content_type.split('boundary=')[1]

    if not boundary:
        raise ValueError("Boundary not found in content-type.")

    raw_data = sys.stdin.buffer.read(content_length)

    # Split the raw data into parts using the boundary
    parts = raw_data.split(f'--{boundary}'.encode())

    form_data = {}

    for part in parts:
        if part:
            # Extract headers and data
            part = part.strip()
            headers_end = part.find(b'\r\n\r\n') + 4  # Find end of headers (two \r\n)
            headers = part[:headers_end]
            data = part[headers_end:]

            headers = headers.decode(errors='ignore')
            content_disposition = None
            content_type = None

            for line in headers.split('\r\n'):
                if line.startswith('Content-Disposition:'):
                    content_disposition = line
                elif line.startswith('Content-Type:'):
                    content_type = line

            # Parse the content-disposition header to extract the name and filename
            name = None
            filename = None
            if content_disposition:
                for param in content_disposition.split(';'):
                    param = param.strip()
                    if param.startswith('name='):
                        name = param.split('=')[1].strip('"')
                    elif param.startswith('filename='):
                        filename = param.split('=')[1].strip('"')

            form_data[name] = {
                'filename': filename,
                'content_type': content_type,
                'data': data
            }

    # Extract the 'preview' field (default to 'false' if not provided)
    preview_flag = form_data.get('preview', {}).get('data', b'false').decode('utf-8').lower() == 'true'
    form_data['preview'] = preview_flag

    return form_data


def apply_sepia_filter(form_data):
    """ Apply a sepia filter to the image data """
    response = {}
    try:
        # Get image data
        if 'image' not in form_data:
            raise ValueError("No image found in the form data.")

        image_data = form_data['image']['data']
        image = Image.open(io.BytesIO(image_data))
        
        # Apply sepia filter
        width, height = image.size
        pixels = image.load()  # Create the pixel map

        for py in range(height):
            for px in range(width):
                r, g, b = image.getpixel((px, py))

                tr = int(0.393 * r + 0.769 * g + 0.189 * b)
                tg = int(0.349 * r + 0.686 * g + 0.168 * b)
                tb = int(0.272 * r + 0.534 * g + 0.131 * b)

                # Set new values for the pixel, ensuring they don't exceed 255
                tr = min(255, tr)
                tg = min(255, tg)
                tb = min(255, tb)

                pixels[px, py] = (tr, tg, tb)

        # Prepare output path based on preview flag
        filename = form_data['image']['filename']
        edited_filename = f'sepia_{filename}'
        output_dir = TEMP_FOLDER if form_data['preview'] else UPLOAD_FOLDER
        
        output_file = os.path.join(output_dir, edited_filename)
        
        # Save the sepia-filtered image
        image.save(output_file)

        response['status'] = 'success'
        response['filename'] = edited_filename
        response['path'] = f"/temp/{edited_filename}" if form_data['preview'] else f"appwebsite/uploads/{edited_filename}"

    except Exception as e:
        response['status'] = 'error'
        response['message'] = str(e)

    return response


def main():
    """ Main function to handle the CGI request and return JSON response """
    try:
        # Parse multipart form data from stdin
        form_data = parse_multipart_data()

        # Apply sepia filter to the image
        result = apply_sepia_filter(form_data)

        # Return a JSON response
        print(json.dumps(result))
    
    except Exception as e:
        error_response = {
            'status': 'error',
            'message': str(e)
        }
        print(json.dumps(error_response))


if __name__ == "__main__":
    main()
