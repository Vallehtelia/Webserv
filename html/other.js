document.addEventListener('DOMContentLoaded', () => {
    const form = document.querySelector('form');

    form.addEventListener('submit', async (event) => {
        event.preventDefault(); // Prevent the default form submission

        const nickname = document.getElementById('nickname').value; // Get the nickname input value
        const profilePic = document.getElementById('profilepic').files[0]; // Get the selected profile picture
        const video = document.getElementById('video').files[0]; // Get the selected video file
        const audio = document.getElementById('audio').files[0]; // Get the selected audio file

        if (!nickname || !profilePic) {
            alert('Please fill out all required fields.'); // Alert if required fields are empty
            return;
        }

        const chunkSize = 1024 * 1024; // Define chunk size (1 MB in this example)

        try {
            // Function to upload file in chunks
            const uploadFileInChunks = async (file, fileType) => {
                let offset = 0; // Initialize offset for file slicing
                const totalChunks = Math.ceil(file.size / chunkSize); // Calculate total number of chunks

                while (offset < file.size) {
                    // Create a slice of the file for the current chunk
                    const chunk = file.slice(offset, offset + chunkSize);
                    const chunkFormData = new FormData();
                    chunkFormData.append('nickname', nickname); // Include nickname with each chunk
                    chunkFormData.append(fileType, chunk); // Append the current chunk

                    // Send the chunk
                    const response = await fetch('/submit', {
                        method: 'POST',
                        headers: {
                            'X-Chunk-Upload': 'true',
                            'X-Chunk-Index': offset / chunkSize // or whatever index you want
                        },
                        body: chunkFormData,
                    });


                    // Check if the response was successful
                    if (!response.ok) {
                        throw new Error(`Chunk upload failed: ${response.statusText}`);
                    }

                    // Update the offset for the next chunk
                    offset += chunkSize;
                    console.log(`Uploaded ${fileType} chunk ${Math.ceil(offset / chunkSize)} of ${totalChunks}`);
                }
            };

            // Upload profile picture in chunks
            await uploadFileInChunks(profilePic, 'profilepic');

            // Upload video if selected
            if (video) {
                await uploadFileInChunks(video, 'video');
            }

            // Upload audio if selected
            if (audio) {
                await uploadFileInChunks(audio, 'audio');
            }

            alert('File upload complete!'); // Alert when the upload is complete
        } catch (error) {
            console.error('Error:', error); // Log any errors
            alert('An error occurred during upload. Please try again.'); // Alert the user of the error
        }
    });
});
