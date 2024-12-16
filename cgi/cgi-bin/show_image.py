#!/usr/bin/env python3
import os
import sys
import json
import cgi
import cgitb

cgitb.enable()  # Enable debugging for errors

def main():
    # Read the request body from a predefined file
    input_file = "./cgi/tmp/cgi_input.html"
    try:
        with open(input_file, "r") as f:
            request_body = json.load(f)
    except FileNotFoundError:
        error_response(f"Input file '{input_file}' not found", 500)
        return
    except json.JSONDecodeError:
        error_response(f"Invalid JSON in input file '{input_file}'", 400)
        return

    # Extract the 'filepath' from the parsed request body
    file_param = request_body.get("filepath")
    if not file_param:
        error_response("Missing 'filepath' parameter in the request body", 400)
        return

    image_directory = "./html/edited/"
    image_path = os.path.join(image_directory, file_param)

    # Check if the file exists and is a supported image type
    if not os.path.exists(image_path):
        error_response(f"File '{file_param}' not found", 404)
        return

    if not image_path.lower().endswith((".png", ".jpg", ".jpeg", ".gif")):
        error_response(f"Unsupported file type for '{file_param}'", 415)
        return

    # Determine MIME type and return the image
    content_type = "image/jpeg" if image_path.lower().endswith((".jpg", ".jpeg")) else "image/png"
    print(f"Content-Type: {content_type}\n")
    try:
        with open(image_path, "rb") as img_file:
            sys.stdout.buffer.write(img_file.read())
    except Exception as e:
        error_response(f"Error reading file: {str(e)}", 500)

def error_response(message, status):
    """Send a JSON error response"""
    print(f"Status: {status}")
    print("Content-Type: application/json\n")
    print(json.dumps({"error": message, "status": status}))

if __name__ == "__main__":
    main()
