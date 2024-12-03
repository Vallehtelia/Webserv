document.addEventListener('DOMContentLoaded', () => {
    // Elements for the image upload form
    const imageUploadForm = document.getElementById('imageUploadForm');
    const fileInput = document.getElementById('image');
    const fileNameSpan = document.getElementById('file-name');
    const submitPostRequestButton = document.getElementById('submitPostRequest');
    const showImageButton = document.getElementById('showImageRequest');
    const postResponseDiv = document.getElementById('postResponse');
    const imageDiv = document.getElementById('showImageDiv');
    const imageWindow = document.getElementById('cgi-image-window');
    const computerScreen = document.getElementById('computer-screen');

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
                postResponseDiv.innerHTML = resultHtml; // Display server response
            } else {
                // postResponseDiv.innerHTML = '<p style="color:red;">Failed to upload image</p>';
				const resultHtml = await response.text();
                const textContainer = document.createElement("div");
                textContainer.innerHTML = resultHtml;
                textContainer.classList.add("text-container")
                const imageWindow = createImageWindow();
				imageWindow.appendChild(textContainer);
                console.error('Server responded with status:', response.status);
            }
        } catch (error) {
            console.error('Error:', error);
            postResponseDiv.innerHTML = `<p style="color:red;">Error: ${error.message}</p>`;
        }
    });


    // <div id="computer-screen" class="computer-screen-class cgi-computer-screen">
    // <div id="cgi-image-window" class="cgi-image-window" style="visibility: hidden;">
    //     <div class="cgi-output-window-header">
    //         <div class="cgi-header-button-container">
    //             <button class="cgi-header button">X</button>
    //         </div>
    //     </div>
    //     <div id="showImageDiv" class="cgi-output-container">
    //     </div>
    //     <div id="postResponse">
    //     </div>
    // </div>

    function createImageWindow() {
        computerScreen.innerHTML = "";
        const imageWindow = document.createElement('div');
        imageWindow.classList.add('cgi-image-window');
        imageWindow.style.visibility = 'hidden'; // Initially hidden
    
        const header = document.createElement('div');
        header.classList.add('cgi-output-window-header');
    
        const headerButtonContainer = document.createElement('div');
        headerButtonContainer.classList.add('cgi-header-button-container');
    
        const closeButton = document.createElement('button');
        closeButton.classList.add('cgi-header', 'button');
        closeButton.textContent = 'X';
    
        // Attach the close button to the header
        headerButtonContainer.appendChild(closeButton);
        header.appendChild(headerButtonContainer);
    
        // Create the output container
        const outputContainer = document.createElement('div');
        outputContainer.classList.add('cgi-output-container');
    
        const showImageDiv = document.createElement('div');
        showImageDiv.classList.add('show-image-div');
        outputContainer.appendChild(showImageDiv);
    
        // Create a div for post-response
        const postResponse = document.createElement('div');
        postResponse.id = 'postResponse';
        outputContainer.appendChild(postResponse);
    
        // Append the header and output container to the image window
        imageWindow.appendChild(header);
        imageWindow.appendChild(outputContainer);
    
        // Append the image window to the container
        document.getElementById('computer-screen').appendChild(imageWindow);
    
        // Add event listener to close button
        closeButton.addEventListener('click', function () {
            imageWindow.remove();  // Hide the image window when close button is clicked
        });
    

        imageWindow.style.visibility = 'visible';
        return imageWindow;
    }


function displayImage(file, container) {
    // Create an image element and set the source to the uploaded file
    console.log(file)
    const image = document.createElement('img');
    image.classList.add('cgi-output-image');
    image.src = file; // Create a URL for the file
    image.alt = 'Processed image';
    container.appendChild(image);
}

    // Handle GET request for image display
    showImageButton.addEventListener('click', async () => {
        // Clear previous image
        imageDiv.innerHTML = '';
        try {
            const response = await fetch('/cgi-bin/show_image.py');
            if (response.ok) {
                const imageBlob = await response.blob();
                const imageUrl = URL.createObjectURL(imageBlob);
                console.log(imageUrl)
                computerScreen.innerHTML = "";
                const imageWindow = createImageWindow();
                console.log(imageWindow)
                displayImage(imageUrl, imageWindow);
                // imageDiv.innerHTML += `<img class="cgi-output-image" src="${imageUrl}" alt="Processed image">`;
                // imageWindow.style.visibility = 'visible';
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

