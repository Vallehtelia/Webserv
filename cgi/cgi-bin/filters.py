#!/venv/bin/python3
from PIL import Image
import os
import sys
import json
import tempfile
import io

UPLOAD_FOLDER = '/app/website/uploads/'
TEMP_FOLDER = '/app/website/temp/'

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

    return form_data

def convert_to_bw(image_data):
    """ Convert the image data to black and white """
    image = Image.open(io.BytesIO(image_data))
    bw_image = image.convert('L')
    return bw_image

def apply_sepia_filter(image_data):
    """ Apply sepia filter to the image data """
    image = Image.open(io.BytesIO(image_data))
    width, height = image.size
    pixels = image.load()

    for py in range(height):
        for px in range(width):
            r, g, b = image.getpixel((px, py))

            tr = int(0.393 * r + 0.769 * g + 0.189 * b)
            tg = int(0.349 * r + 0.686 * g + 0.168 * b)
            tb = int(0.272 * r + 0.534 * g + 0.131 * b)

            if tr > 255:
                tr = 255
            if tg > 255:
                tg = 255
            if tb > 255:
                tb = 255

            pixels[px, py] = (tr, tg, tb)

    return image

def save_image(image, filename):
    """ Save the image to the temporary folder """
    output_dir = TEMP_FOLDER
    output_file = os.path.join(output_dir, filename)
    image.save(output_file)
    return output_file

def main():
    """ Main function to handle the CGI request and return JSON response """
    try:
        # Parse multipart form data from stdin
        form_data = parse_multipart_data()

        # Get the image data
        if 'image' not in form_data:
            raise ValueError("No image found in the form data.")

        image_data = form_data['image']['data']
        filename = form_data['image']['filename']
        
        # Initialize response object
        response = {'status': 'success', 'results': []}

        # If the form requests a BW conversion, process it
        if 'bw' in form_data:
            bw_image = convert_to_bw(image_data)
            bw_filename = f'bw_{filename}'
            bw_file_path = save_image(bw_image, bw_filename)
            response['results'].append({'type': 'bw', 'filename': bw_filename, 'path': bw_file_path})

        # If the form requests a sepia filter, process it
        if 'sepia' in form_data:
            sepia_image = apply_sepia_filter(image_data)
            sepia_filename = f'sepia_{filename}'
            sepia_file_path = save_image(sepia_image, sepia_filename)
            response['results'].append({'type': 'sepia', 'filename': sepia_filename, 'path': sepia_file_path})

        # If neither BW nor sepia is requested, return an error
        if not response['results']:
            response['status'] = 'error'
            response['message'] = 'No valid transformation specified in the form data.'

        # Return the response as JSON
        print("Content-Type: application/json\n")
        print(json.dumps(response))

    except Exception as e:
        error_response = {'status': 'error', 'message': str(e)}
        print("Content-Type: application/json\n")
        print(json.dumps(error_response))

if __name__ == "__main__":
    main()
