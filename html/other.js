document.addEventListener('DOMContentLoaded', () => {
    const form = document.querySelector('form');

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
                body: formData,
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
