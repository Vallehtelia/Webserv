import {createImageWindow, displayMedia} from './displayImage.js'
import { directoryListing } from './directoryListing.js';




async function uploadFiles(data) {
    console.log("DATA: ", data);
    try {
    const response = await fetch('/uploads', {
        method: 'POST',
        body: data,
    });
    if (!response.ok) {
        throw new Error(`Chunk upload failed: ${response.statusText}`);
    }
    if (response.ok) {
        try {
            // Parse the JSON body of the response
            const jsonResponse = await response.json();
            console.log(jsonResponse);
        } catch (error) {
            console.error('Error parsing JSON:', error);
        }
    }
    }
    catch (error) {
        console.error('Error:', error); // Log any errors
        alert('An error occurred during upload. Please try again.'); // Alert the user of the error
}
}






document.addEventListener('DOMContentLoaded', () => {
    const computerScreen = document.getElementById("computer-screen")

    directoryListing(computerScreen);
 document.querySelectorAll("#file-upload-form input[type='file']").forEach(function(input) {

     input.addEventListener("change", function() {
         // Find the sibling span element (file name display)
         var fileNameDisplay = this.nextElementSibling;
         console.log(fileNameDisplay);

         // Update the text with the name of the file or "No file chosen"
         var fileName = this.files[0] ? this.files[0].name : "No file !!chosen";
         fileNameDisplay.classList.add("file-name-display");
         fileNameDisplay.textContent = fileName;
        });
    });



    const form = document.querySelector('#file-upload-form');

    form.addEventListener('submit', async (event) => {
        event.preventDefault(); // Prevent the default form submission
        const profilePic = document.getElementById('profilepic').files[0];
        const video = document.getElementById('video').files[0];
        const audio = document.getElementById('audio').files[0];
        if (!profilePic && !audio && !video) {
            return;
        }
        //showLoadingSpinner();
        try {
            const formData = new FormData();
            formData.append('profilepic', profilePic);
            if (video) {
                formData.append('video', video);
            }
            if (audio) {
                formData.append('audio', audio);
            }
            await uploadFiles(formData);
            directoryListing(computerScreen);
        } catch (error) {
            console.error('Error:', error); // Log any errors
            alert('An error occurred during upload. Please try again.'); // Alert the user of the error
        }
    });

} );