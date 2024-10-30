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

        const nickname = document.getElementById('nickname').value;
        const profilePic = document.getElementById('profilepic').files[0];

        if (!nickname || !profilePic) {
            alert('Please fill out all fields.');
            return;
        }

        const formData = new FormData();
        formData.append('nickname', nickname);
        formData.append('profilepic', profilePic);

        try {
            const response = await fetch('/submit', {
                method: 'POST',
                body: formData, // This contains the multipart data
            });

            if (response.ok) {
                const responseText = await response.text();
                alert(`Success: ${responseText}`);
            } else {
                alert('Upload failed. Please try again.');
            }
        } catch (error) {
            console.error('Error:', error);
            alert('An error occurred. Please try again.');
        }
    });
});
