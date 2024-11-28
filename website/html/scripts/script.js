document.addEventListener('DOMContentLoaded', () => {
    // Elements for the POST image upload form
    const imageUploadForm = document.getElementById('imageUploadForm');
    const submitPostRequestButton = document.getElementById('submitPostRequest');
    const postResponseDiv = document.getElementById('postResponse');

    // Existing GET request button
    const runScriptButton = document.getElementById('runScriptButton');
    const getResponseDiv = document.getElementById('getResponse'); // Div for GET response

    const showImageButton = document.getElementById('showImageRequest');
    const imageDiv = document.getElementById('showImageDiv');

    // Handle POST request for image upload and processing
    submitPostRequestButton.addEventListener('click', async () => {
        const formData = new FormData(imageUploadForm);
        // Log form data entries
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
                postResponseDiv.innerHTML = resultHtml;
            } else {
                postResponseDiv.innerHTML = '<p style="color:red;">Failed to upload image</p>';
            }
        } catch (error) {
            console.error('Error:', error);
            postResponseDiv.innerHTML = `<p style="color:red;">Error: ${error.message}</p>`;
        }
    });

    // Handle GET request for image display
    showImageButton.addEventListener('click', async () => {
        try {
            const response = await fetch('/cgi-bin/show_image.py');
            if (response.ok) {
                const imageBlob = await response.blob();
                const imageUrl = URL.createObjectURL(imageBlob);
                imageDiv.innerHTML = `<img class="cgi-output-image" src="${imageUrl}" alt="Processed image">`;
            } else {
                imageDiv.innerHTML = '<p style="color:red;">Failed to load image.</p>';
            }
        } catch (error) {
            imageDiv.innerHTML = `<p style="color:red;">Error: ${error.message}</p>`;
        }
    });
});


