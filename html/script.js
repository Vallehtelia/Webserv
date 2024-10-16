document.getElementById('myForm').addEventListener('submit', function(event) {
    event.preventDefault(); // Prevent the default form submission

    // Create a FormData object to handle all form data
    const formData = new FormData(this);  // 'this' refers to the form element

    // Send the FormData to the server as multipart/form-data
    fetch('/submit', {  // Change '/submit' to your server endpoint
        method: 'POST',
        body: formData // Send FormData directly
    })
    .then(response => {
        if (!response.ok) {
            throw new Error('Network response was not ok');
        }
        return response.text(); // Get the response as text
    })
    .then(htmlResponse => {
        document.getElementById('response').innerHTML = htmlResponse; // Display response in the div
    })
    .catch(error => {
        console.error('Error:', error);
        document.getElementById('response').innerHTML = "<p>Error submitting form.</p>";
    });
});
