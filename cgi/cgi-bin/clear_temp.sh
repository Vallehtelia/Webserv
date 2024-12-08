#!/bin/bash

# Check if the request method is DELETE
if [ "$REQUEST_METHOD" != "DELETE" ]; then
    echo '{"status": "error", "message": "Only DELETE method is allowed."}'
fi

# Try to remove files in the temp folder
TEMP_FOLDER="/app/website/temp/"

if rm -rf "${TEMP_FOLDER}"*; then
    echo '{"status": "success", "message": "Temporary files cleared."}'
else
    echo '{"status": "error", "message": "Failed to clear temp folder."}'
fi
