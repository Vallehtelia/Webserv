document.addEventListener('DOMContentLoaded', () => {
    // Elements for the image upload form
    const imageUploadForm = document.getElementById('imageUploadForm');
    const fileInput = document.getElementById('image');
    const fileNameSpan = document.getElementById('file-name');
    const submitPostRequestButton = document.getElementById('submitPostRequest');
    const showImageButton = document.getElementById('showImageRequest');
    const postResponseDiv = document.getElementById('postResponse');
    const imageDiv = document.getElementById('showImageDiv');

    // Monitor file selection
    fileInput.addEventListener('change', () => {
        if (fileInput.files.length > 0) {
            const selectedFile = fileInput.files[0];
            fileNameSpan.textContent = selectedFile.name; // Display selected file name
            console.log('File selected:', selectedFile);
        } else {
            fileNameSpan.textContent = 'No image chosen';
            console.log('No file selected');
        }
    });

    // Handle POST request for image upload and processing
    submitPostRequestButton.addEventListener('click', async () => {
        const formData = new FormData(imageUploadForm);

        // Clear previous response
        postResponseDiv.innerHTML = '';

        // Debug: Log form data before sending
        console.log('Form data before sending:');
        formData.forEach((value, key) => {
            console.log(key, value);
        });

        try {
            const response = await fetch('/cgi-bin/edit_image.py', {
                method: 'POST',
                body: formData
            });

            if (response.ok) {
                const resultHtml = await response.text();
				imageDiv.innerHTML = '';
                postResponseDiv.innerHTML = resultHtml; // Display server response
            } else {
                // postResponseDiv.innerHTML = '<p style="color:red;">Failed to upload image</p>';
				const resultHtml = await response.text();
				postResponseDiv.innerHTML = resultHtml;
                console.error('Server responded with status:', response.status);
            }
        } catch (error) {
            console.error('Error:', error);
            postResponseDiv.innerHTML = `<p style="color:red;">Error: ${error.message}</p>`;
        }
    });

    // Handle GET request for image display
    showImageButton.addEventListener('click', async () => {
        // Clear previous image
        imageDiv.innerHTML = '';

        try {
            const response = await fetch('/cgi-bin/show_image.py');
            if (response.ok) {
                const imageBlob = await response.blob();
                const imageUrl = URL.createObjectURL(imageBlob);
				postResponseDiv.innerHTML = '';
                imageDiv.innerHTML = `<img class="cgi-output-image" src="${imageUrl}" alt="Processed image">`;
            } else {
                imageDiv.innerHTML = '<p style="color:red;">Failed to load image.</p>';
                console.error('Server responded with status:', response.status);
            }
        } catch (error) {
            console.error('Error:', error);
            imageDiv.innerHTML = `<p style="color:red;">Error: ${error.message}</p>`;
        }
    });
});

