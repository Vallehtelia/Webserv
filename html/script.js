document.getElementById('myForm').addEventListener('submit', function(event) {
    event.preventDefault(); // Prevent the default form submission
document.addEventListener('DOMContentLoaded', () => {
    const runScriptButton = document.getElementById('runScriptButton');
    const cgiOutputDiv = document.getElementById('cgiOutput');
    const outputContent = document.getElementById('outputContent');

    // Run CGI script -button
    runScriptButton.addEventListener('click', async () => {
        try {
            // Fetch the CGI output directly from the expected file location
            const response = await fetch('/cgi-bin/hello.py');  // Adjusted to fetch the static file

            if (response.ok) {
                const fileText = await response.text();
                outputContent.innerHTML = fileText; // Display file content in the output div
                cgiOutputDiv.style.display = 'block';
            } else {
                outputContent.innerHTML = '<p style="color: red;">Failed to load CGI output file.</p>';
                cgiOutputDiv.style.display = 'block';
            }
        } catch (error) {
            console.error('Error:', error);
            outputContent.innerHTML = '<p style="color: red;">An error occurred while loading CGI output.</p>';
            cgiOutputDiv.style.display = 'block';
        }
    });

    // Lomakkeen lähetys
    form.addEventListener('submit', async (event) => {
        event.preventDefault(); // Prevent the default form submission

    const form = event.target;
    const formData = new FormData(form); // Create a FormData object from the form

    // Send the form data using fetch
    fetch('http://localhost:8002', {
        method: 'POST',
        body: formData, // Send the form data as the request body
    })
    .then(response => {
        if (!response.ok) {
            throw new Error('Network response was not ok');
        }
        return response.text(); // Convert response to text
    })
    .then(data => {
        document.getElementById('response').innerText = data; // Display the response
    })
    .catch(error => {
        console.error('There was a problem with the fetch operation:', error);
        document.getElementById('response').innerText = 'Error: ' + error.message; // Show error
    });
});
