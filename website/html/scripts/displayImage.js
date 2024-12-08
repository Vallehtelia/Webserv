export function createImageWindow(fileName) {

    const computerScreen = document.getElementById('computer-screen');

	let imageWindow = document.getElementById("file-display-window")
	if (imageWindow)
		imageWindow.remove();
	imageWindow = document.createElement("div");
    imageWindow.classList.add('cgi-image-window');
	imageWindow.id = "file-display-window"

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

	// file name on header
	const headerFileName = document.createElement("p");
	headerFileName.innerHTML = fileName;
	header.appendChild(headerFileName);

    // Create the output container
    const outputContainer = document.createElement('div');
    outputContainer.classList.add('cgi-output-container');

    const showImageDiv = document.createElement('div');
    showImageDiv.classList.add('show-image-div');

    outputContainer.appendChild(showImageDiv);
    imageWindow.appendChild(header);
    imageWindow.appendChild(outputContainer);

	computerScreen.append(imageWindow);

    closeButton.addEventListener('click', function () {
        imageWindow.remove()
    });
    return imageWindow; // Return the image window to use as the container for the image
}

export function displayMedia(file, filename, container) {
    console.log("FILE:", filename);
    
	console.log(file)
	var mediaElement = "";
    const dotIndex = filename.lastIndexOf('.');
    if (dotIndex > -1)
    {
        const extension = filename.slice(dotIndex + 1).toLowerCase()
        console.log(extension)
    // Check if the file is an image, video, or audio
    if (['jpg', 'jpeg', 'png', 'gif', 'bmp', 'webp'].includes(extension)) {
        // Handle image files
        mediaElement = document.createElement('img');
        mediaElement.classList.add('cgi-output-image');
        mediaElement.src = file; // Set the file URL for the image
        mediaElement.alt = 'Processed image';
    } else if (['mp4'].includes(extension)) {
        // Handle video files
        mediaElement = document.createElement('video');
        mediaElement.classList.add('cgi-output-video');
		mediaElement.autoplay = true;
        mediaElement.src = file; // Set the file URL for the video
        //mediaElement.controls = true; // Add video controls
        mediaElement.alt = 'Processed video';
    } else if (['mp3', 'wav'].includes(extension)) {
        // Handle audio files
        mediaElement = document.createElement('audio');
		mediaElement.autoplay = true;
        mediaElement.classList.add('cgi-output-audio');
        mediaElement.src = file; // Set the file URL for the audio
        mediaElement.controls = true; // Add audio controls
        mediaElement.alt = 'Processed audio';
    } else {
		mediaElement = document.createElement('span');
		mediaElement.classList.add('cgi-output-text');
		mediaElement.innerHTML = "<p>cannot display this file</p>"
        console.error('Unsupported file type');
    }
    console.log(container);
    console.log("appending ");
    container.appendChild(mediaElement); // Append the created media element to the container
	}
	else {
		mediaElement = document.createElement('span');
		mediaElement.classList.add('cgi-output-text');
		mediaElement.innerHTML = "<p>cannot display this file</p>"
        console.error('Unsupported file type');
    }
}
