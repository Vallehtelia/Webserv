document.addEventListener('DOMContentLoaded', () => {

    
 
 // Get the popup and overlay elements
        const popup = document.getElementById("popup");
        const overlay = document.getElementById("overlay");
        const agreeButton = document.getElementById("agree");
        const disagreeButton = document.getElementById("disagree");

        // Variables to store the form to submit after agreement
        let formToSubmit = null;

        // Show the popup when the page loads
        function showPopup(event, form) {
            event.preventDefault(); // Prevent the form from submitting immediately
            formToSubmit = form;
            popup.style.display = "block";
            overlay.style.display = "block";
        }

        // Handle "Yes" button click
        agreeButton.onclick = function () {
            popup.style.display = "none";
            overlay.style.display = "none";
            // Submit the form if the user agrees
            if (formToSubmit) {
                formToSubmit.submit();
            }
        };

        // Handle "No" button click
        disagreeButton.onclick = function () {
            window.location.href = "https://www.google.com";
        };

        // Add event listeners for the forms
        document.getElementById("uploadForm").addEventListener("submit", function(event) {
			event.preventDefault()
            showPopup(event, this);
        });

        document.getElementById("scriptsForm").addEventListener("submit", function(event) {
			event.preventDefault()
            showPopup(event, this);
        });

        document.getElementById("jsonDataForm").addEventListener("submit", function(event) {
			event.preventDefault()
            showPopup(event, this);
        });

}) ;