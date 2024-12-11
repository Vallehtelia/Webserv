import {createImageWindow, displayMedia} from './displayImage.js'


async function fetchFile(filePath) {
    console.log(filePath);
    try {
        const response = await fetch(`${filePath}`);
        if (!response.ok) {
            throw new Error('Failed to fetch file');
        }
        const blob = await response.blob();
        return URL.createObjectURL(blob);
    } catch (error) {
        console.error('Error fetching file:', error);
    }
}



async function handleFileClick(file) {
	console.log(file)
    console.log(`File clicked: ${file.name}`);
    const filePath = await fetchFile(file.fileurl);
    const imageWindow = createImageWindow(file.name);
    console.log ("IMAGE WINDOW: ", imageWindow)
	if (file.type == "file")
    	displayMedia(filePath, file.name, imageWindow);
	if (file.type == "directory")
	{
		const directoryContainer = document.createElement("div")
		directoryListing(directoryContainer, file.name)
		imageWindow.appendChild(directoryContainer)
	}
	else
		return ;
}

async function handleDirectoryClick(directoryPath, directoryname)
{
	const fullPath = directoryPath + "/" + directoryname;
	console.log("full path:", fullPath)
    const directoryWindow = createImageWindow(directoryname);
    console.log ("DIRECTORY WINDOW: ", directoryWindow)
	const directoryContainer = document.createElement("div")
	directoryListing(directoryContainer, fullPath)
	directoryWindow.appendChild(directoryContainer)
}


const getFileType = (file) => {
    if (!file || !file.name) return null; // Handle cases where file or file.name is undefined
    const dotIndex = file.name.lastIndexOf('.');
    if (dotIndex > -1)
    {
        const extension = file.name.slice(dotIndex + 1).toLowerCase()
        console.log(extension)
        if (extension === "mp3")
            return "/assets/audio_icon.png"
        else if (extension === "jpg" || extension === "jpeg" || extension === "png")
            return "/assets/image_icon.png"
        else if (extension === "mp4")
            return "/assets/video_icon.png"
    }
    return "/assets/random_icon.png"
}

const selectIcon = (file) => {
    console.log(file.type)
    if (file.type === "file")
    {
        return getFileType(file)
    }
    else
        return "/assets/folder-icon.png"
}

export async function directoryListing(container, directory) {

	console.log(directory)
	let filepath;
	if (directory)
		filepath = directory
	else
		filepath = "/uploads"
	console.log(filepath)
    const response = await fetch(filepath);
    const jsonResponse = await response.json();
    console.log(jsonResponse);

	const directoryPath = jsonResponse.directoryPath;
    // document.getElementById("computer-screen").innerHTML = "";
	console.log("container to list directory in: ", container)
    const newDiv = document.createElement("div");
    newDiv.classList.add('directory-list');
    jsonResponse.files.forEach(file => {
        const listItem = document.createElement("div");
        const fileIcon = document.createElement("img");
        fileIcon.src = selectIcon(file) // Set the image path
        fileIcon.alt = "file icon"; // Optionally set alternative text
        fileIcon.classList.add('file-icon');
        listItem.classList.add('directory-item');
        const fileName = document.createElement("p");
        fileName.classList.add('file-name');
        fileName.innerHTML = file.name;
        listItem.appendChild(fileIcon);
        listItem.appendChild(fileName);
        newDiv.appendChild(listItem);

        listItem.addEventListener('click', () => {
			if (file.type == "file")
			{
				console.log(`Clicked on file: ${file.name}`);
				handleFileClick(file); // Replace with your function
			}
			else if (file.type == "directory")
			{
				console.log("clicked on a directory: ", file.name)
				handleDirectoryClick(directoryPath, file.name)
			}
        });

    });
    container.innerHTML = "";
    container.appendChild(newDiv);
}