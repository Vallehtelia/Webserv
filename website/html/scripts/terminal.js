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
		const response = await fetch(`/cgi-bin/${script}`);
		if (response.ok) {
			
			// Fetch the CGI output HTML
			const terminalOutput = await fetch('/cgi_output.html');
			
			if (terminalOutput.ok) {
				const bodyContent = await terminalOutput.text();
				const lines = bodyContent.split('\n');
				let i = 0;
				for (let line of lines) {
					const lineElement = document.createElement('p');
					lineElement.innerHTML = line;
					terminalBody.insertBefore(lineElement, terminalBody.children[1]);
					//console.log(line);  // You can process each line here
					i++;
				}
			} else {
				const errorMessage = document.createElement('p');
				errorMessage.classList.add('error-message');
				errorMessage.innerHTML = 'Failed to fetch CGI output, status:' + response.status;
				terminalBody.insertBefore(errorMessage, terminalBody.children[1]);
			}
		} else {
			const errorMessage = document.createElement('p');
			errorMessage.classList.add('error-message');
			errorMessage.innerHTML = 'Server responded with status:' + response.status;
			terminalBody.insertBefore(errorMessage, terminalBody.children[1]);
		}
	} catch (error) {
		console.error('Error fetching data:', error);
	}
}

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