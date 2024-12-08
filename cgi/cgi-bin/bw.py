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

    # Extract the 'preview' field (default to 'false' if not provided)
    preview_flag = form_data.get('preview', {}).get('data', b'false').decode('utf-8').lower() == 'true'
    form_data['preview'] = preview_flag

    return form_data


def convert_to_bw(form_data):
    """ Convert the image data to black and white """
    response = {}
    try:
        # Get image data
        if 'image' not in form_data:
            raise ValueError("No image found in the form data.")

        image_data = form_data['image']['data']
        image = Image.open(io.BytesIO(image_data))
        
        # Convert to black and white
        bw_image = image.convert('L')
        
        # Prepare output path based on preview flag
        filename = form_data['image']['filename']
        edited_filename = f'bw_{filename}'
        output_dir = TEMP_FOLDER if form_data['preview'] else UPLOAD_FOLDER
        
        output_file = os.path.join(output_dir, edited_filename)
        
        # Save the black-and-white image
        bw_image.save(output_file)

        response['status'] = 'success'
        response['filename'] = edited_filename
        response['path'] = f"/temp/{edited_filename}" if form_data['preview'] else f"/uploads/{edited_filename}"

    except Exception as e:
        response['status'] = 'error'
        response['message'] = str(e)

    return response


def main():
    """ Main function to handle the CGI request and return JSON response """
    try:
        # Parse multipart form data from stdin
        form_data = parse_multipart_data()

        # Convert image to black and white
        result = convert_to_bw(form_data)

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
