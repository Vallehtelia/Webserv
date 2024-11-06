document.addEventListener('DOMContentLoaded', () => {
    // Elements for the POST image upload form
    const imageUploadForm = document.getElementById('imageUploadForm');
    const submitPostRequestButton = document.getElementById('submitPostRequest');
    const postResponseDiv = document.getElementById('postResponse');

    // Existing GET request button
    const runScriptButton = document.getElementById('runScriptButton');

    // Handle POST request for image upload and processing
    submitPostRequestButton.addEventListener('click', async () => {
        const formData = new FormData(imageUploadForm); // Create FormData from the form

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

    // Handle existing GET request button
    runScriptButton.addEventListener('click', async () => {
        try {
            const response = await fetch('/cgi-bin/hello.py'); // Update path to your GET CGI script
            if (response.ok) {
                const fileText = await response.text();
                alert("CGI GET Output:\n" + fileText);
            } else {
                alert("Failed to load CGI GET output.");
            }
        } catch (error) {
            console.error('Error:', error);
            alert("An error occurred while loading CGI GET output.");
        }
    });
});

