document.querySelectorAll("#file-upload-form input[type='file']").forEach(function(input) {
    input.addEventListener("change", function() {
        // Find the sibling span element (file name display)
        var fileNameDisplay = this.nextElementSibling;

        // Update the text with the name of the file or "No file chosen"
        var fileName = this.files[0] ? this.files[0].name : "No file chosen";
        fileNameDisplay.textContent = fileName;
    });
});

const listUploadedFiles = (response) => {
    // Create a new div to hold the response
    const newDiv = document.createElement("div");
    const fileList = document.createElement("ul");
    fileList.classList.add('file-list');
    //newDiv.classList.add('file-list');
    response.uploadedFiles.forEach(file => {
        const listItem = document.createElement("li");
        listItem.innerHTML = "file: " + file.filename + "<br>url: " + file.fileurl;
        fileList.appendChild(listItem);
    });

    // Append the file list to the new div
    newDiv.appendChild(fileList);
    document.getElementById("getResponse").innerHTML = "<h1>uploaded files succesfully:</h1>";
    document.getElementById("getResponse").appendChild(newDiv);
};

const showLoadingSpinner = () => {
    document.getElementById("getResponse").innerHTML = '<h1>uploading files<h1><div id="loading-icon" class="spinner"></div>'
}

const form = document.querySelector('#file-upload-form');

    form.addEventListener('submit', async (event) => {
        event.preventDefault(); // Prevent the default form submission
        const profilePic = document.getElementById('profilepic').files[0];
        const video = document.getElementById('video').files[0];
        const audio = document.getElementById('audio').files[0];
        
        if (!profilePic && !audio && !video) {
            return;
        }
        showLoadingSpinner();
        try {
            const formData = new FormData();
            formData.append('profilepic', profilePic);
            if (video) {
                formData.append('video', video);
            }
            if (audio) {
                formData.append('audio', audio);
            }
            const response = await fetch('/submit', {
                method: 'POST',
                body: formData,
            });
            if (!response.ok) {
                throw new Error(`Chunk upload failed: ${response.statusText}`);
            }
            if (response.ok) {
                try {
                    // Parse the JSON body of the response
                    const jsonResponse = await response.json();
                    console.log(jsonResponse); // Log the parsed JSON response
                    listUploadedFiles(jsonResponse);
                } catch (error) {
                    console.error('Error parsing JSON:', error);
                }
            }
        } catch (error) {
            console.error('Error:', error); // Log any errors
            alert('An error occurred during upload. Please try again.'); // Alert the user of the error
        }
    });