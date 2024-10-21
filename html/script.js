document.getElementById('myForm').addEventListener('submit', function(event) {
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
