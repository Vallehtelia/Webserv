document.addEventListener('DOMContentLoaded', () => {

const terminalBody = document.getElementById('terminal-body');
const promptContainer = document.getElementById('prompt-container');
const promptInput = document.getElementById('prompt');
const scriptList = document.getElementById('cgi-script-list');




// Add an event listener to detect when the Enter key is pressed
promptInput.addEventListener('keydown', function(event) {
    // Check if the pressed key is Enter (keyCode 13)
    if (event.key === 'Enter') {
        event.preventDefault();  // Prevent form submission
        submitInput(promptInput.value);  // Call a function to handle the input

        // Optionally, clear the input after submission
        promptInput.value = '';
    }
});


const sendRequest = async (script) => {
	try {
		const response = await fetch(`/cgi/cgi-bin/${script}`);
		if (response.ok) {
			const bodyContent = await response.text();
			console.log(bodyContent);

			const lines = bodyContent.split('\n');
			let i = 0;
			for (let line of lines) {
				const lineElement = document.createElement('p');
				lineElement.innerText = line;
				terminalBody.insertBefore(lineElement, terminalBody.children[1]);
				i++;
			}
		} else {
			const errorMessage = document.createElement('p');
			errorMessage.classList.add('error-message');
			errorMessage.innerHTML = 'Server responded with status:' + response.status;
			terminalBody.insertBefore(errorMessage, terminalBody.children[1]);
		}
	} catch (error) {
		console.error('Error fetching data:', error);
		const errorMessage = document.createElement('p');
		errorMessage.classList.add('error-message');
		errorMessage.innerHTML = 'An error occurred while fetching the data.';
		terminalBody.insertBefore(errorMessage, terminalBody.children[1]);
	}
};


function submitInput(inputValue) {
    console.log('Input submitted:', inputValue);
	const scriptName = inputValue + ".sh";
	console.log("SCRIPT NAME: ", scriptName)
	const lineElement = document.createElement('p');
	lineElement.innerHTML = "/cgi-bin/" + scriptName;
	terminalBody.insertBefore(lineElement, terminalBody.children[1]);
	sendRequest(scriptName)
	

}

}
)