document.getElementById('myForm').addEventListener('submit', function(event) {
    event.preventDefault(); // Prevent the default form submission

    // Create a FormData object to handle form data
    const formData = new FormData(this);  // 'this' refers to the form element

    // Separate text fields (username, email) and file (profilePic)
    const jsonObject = {};
    formData.forEach((value, key) => {
        if (key !== 'profilePic') {
            jsonObject[key] = value;  // Add text fields to jsonObject
        }
    });

    // Handle file separately
    const profilePicFile = formData.get('profilePic');

    // Send JSON (for text fields) and FormData (for the file) to the server
    fetch('/userdata', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json'  // Send JSON data
        },
        body: JSON.stringify(jsonObject)
    })
    .then(response => response.text())  // Use text() to handle plain HTML response
    .then(htmlResponse => {
        document.getElementById('response').innerHTML = htmlResponse;  // Insert HTML directly

        if (profilePicFile) {
            // Now, send the file using FormData
            const fileData = new FormData();
            fileData.append('profilePic', profilePicFile);

            // Send the file to a separate endpoint (e.g., '/upload')
            return fetch('/upload', {  // Replace '/upload' with your file upload endpoint
                method: 'POST',
                body: fileData  // Send file using FormData
            });
        } else {
            return null;  // No file to upload
        }
    })
    .then(fileResponse => {
        if (fileResponse) {
            return fileResponse.text();  // Handle the file upload response as plain HTML
        }
    })
    .then(fileResponseText => {
        if (fileResponseText) {
            document.getElementById('response').innerHTML += fileResponseText;  // Append file response HTML
        }
    })
    .catch(error => {
        console.error('Error:', error);
        document.getElementById('response').innerHTML = "<p>Error submitting form.</p>";
    });
});
