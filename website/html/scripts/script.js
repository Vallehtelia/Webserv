import { directoryListing } from './directoryListing.js';
import {createImageWindow, displayMedia} from './displayImage.js'


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
    const editedImage = document.getElementById('edited_image')
    const uploadEditedImage = document.getElementById('upload-edited');
    let selectedFilter = null;
    let originalImageUrl = null;
    let originalImageName = null;

    let formData;

    
    console.log("COMPUTER SCREEN SCRIPTS.JS: ", computerScreen)
    directoryListing(computerScreen)
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


        

    

    const changeFilter = (newSrc, filter) => {
        selectedFilter = filter;
        console.log("set filter to: ", filter)
        if (filter == "none")
            editedImage.src = originalImageUrl
        else
            editedImage.src = newSrc;
    }

    const createFilterElement = (filter, imageUrl) => {
        const li = document.createElement('li')
        li.classList.add("cgi-filter")

        const img = document.createElement("img")
        img.classList.add("image-filter-preview")
        if (filter == "none")
            imageUrl = originalImageUrl
        img.src = imageUrl
        console.log("image src  is: ", img.src)
        const p = document.createElement("p")
        p.classList.add("filter-name")
        p.innerHTML = filter.split('.')[0];
        li.appendChild(img)
        li.appendChild(p)
        li.addEventListener('click', () => {
            changeFilter(img.src, filter);
        })
        return li
    }



    async function fetchPreviewImage(filter, formData) {
        console.log("fetching: ", filter);
        formData.append('preview', 'true');
        printFormData(formData)
        try {
            const response = await fetch(`/cgi/cgi-bin/${filter}`, {
                method: 'POST',
                body: formData,
            });
            if (response.ok) {
                console.log(response.body);
                const jsonResponse = await response.json(); // Await the JSON response
                const imageUrl = jsonResponse.path; // Get the path from the response
                console.log(imageUrl);
                return imageUrl; // Return the image URL
            } else {
                console.log("error in response:");
                throw new Error("Failed to fetch preview image.");
            }
        } catch (error) {
            console.error('Error fetching preview image:', error);
        }
    }
    
   
    const showFilters = async (formData) => {
        const filters = ['none', 'bw.py', 'sepia.py'];
        const filterList = document.getElementById("cgi_filter-list");
        const filterListContainer = document.getElementById("cgi-filter-list-container")
        
        for (const filter of filters) {
            console.log("creating element for :", filter)
            let imageUrl;
            if (filter != "none")
                imageUrl = await fetchPreviewImage(filter, formData)
            else
                imageUrl = originalImageUrl
            console.log(imageUrl)
            const li = createFilterElement(filter, imageUrl);
            console.log(li, filterList)
            filterList.appendChild(li);
            // console.log(filterList)
        }
        filterListContainer.style.visibility = "visible"
    }


    const printFormData = (formData) => {
        console.log('Form data:');
        formData.forEach((value, key) => {
            console.log(key, value);
        });
    }

    submitPostRequestButton.addEventListener('click', async () => {
        formData = new FormData(imageUploadForm);
        console.log("hola")
        // Clear previous response
        postResponseDiv.innerHTML = '';


        printFormData(formData);

        try {
            const response = await fetch('/temp', {
                method: 'POST',
                body: formData
            });
            if (response.ok) {
                console.log("upload ok")
                const jsonResponse = await response.json();
                console.log(jsonResponse)
                originalImageUrl = jsonResponse.uploadedFiles[0].fileurl
                const filename = jsonResponse.uploadedFiles[0].filename
                originalImageName = filename
                await showFilters(formData);
                editedImage.src = originalImageUrl;

            } else {

				// const resultHtml = await response.text();
                // const textContainer = document.createElement("div");
                // textContainer.innerHTML = resultHtml;
                // textContainer.classList.add("text-container")
                // const imageWindow = createImageWindow();
				// imageWindow.appendChild(textContainer);
                console.error('Server responded with status:', response.status);
            }
        } catch (error) {
            console.error('submit Error:', error);
            console.log(response)
            postResponseDiv.innerHTML = `<p style="color:red;">Error: ${error.message}</p>`;
        }
    });


    const clearTempFolder = async () => {
        try {
            const response = await fetch(`/cgi/cgi-bin/clear_temp.sh`, {
                method: 'DELETE',
            });
    
            // Check if the response is successful (status 200)
            if (response.ok) {
                const jsonResponse = await response.json();
                console.log('Success:', jsonResponse);
            } else {
                console.log('Failed to clear temp folder', response.status);
            }
        } catch (error) {
            console.error('Error:', error);
        }
    }

    const uploadFilteredImage = async () => {
        const filterListContainer = document.getElementById("cgi-filter-list-container")

        formData.append('preview', 'false');
        printFormData(formData)

        try {
            let response;
            if (selectedFilter == "none")
            {
                response = await fetch('uploads/', {
                    method: 'POST',
                    body: formData,
                });
            }
            else
            {

                response = await fetch(`cgi/cgi-bin/${selectedFilter}`, {
                    method: 'POST',
                    body: formData,
                });

            }
            if (response.ok) {
                console.log(response)
                const jsonResponse = await response.json();
                if (!jsonResponse.ok)
                    console.log("json errooor")
                console.log("json response: ", jsonResponse);
                filterListContainer.style.visibility = "hidden";
                document.getElementById("cgi_filter-list").innerHTML = "";
                console.log("clearing temp")
                clearTempFolder();
                selectedFilter = null;
                originalImageName = null;
                originalImageUrl = null;
                directoryListing(computerScreen);
                imageUploadForm.reset();

            } else {
                console.log("Error uploading filtered image.");
            }
        } catch (error) {
            console.error("Error during upload:", error);
        }
    };

    uploadEditedImage.addEventListener('click', async (formData) => {
        uploadFilteredImage(formData);
    });

});







