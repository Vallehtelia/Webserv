document.addEventListener('DOMContentLoaded', () => {
    const todoList = document.getElementById('todo-list');
    const todoForm = document.getElementById('todo-form');
    const todoTitleInput = document.getElementById('todo-title');

    // Function to load the to-do list from the server
    const loadTodos = async () => {
        try {
            const response = await fetch('todos.json'); // Adjust the path to your JSON file
            if (!response.ok) throw new Error('Network response was not ok');
            const data = await response.json();
            renderTodos(data.todos);
        } catch (error) {
            console.error('Error loading todos:', error);
        }
    };

    // Function to render to-do items
    const renderTodos = (todos) => {
        todoList.innerHTML = ''; // Clear the existing list
        todos.forEach((todo, index) => {
            const listItem = document.createElement('li');
            listItem.textContent = todo.title;
            listItem.className = todo.completed ? 'completed' : 'not-completed';

            // Create a remove button for each item
            const removeButton = document.createElement('button');
            removeButton.textContent = 'Remove';
            removeButton.addEventListener('click', () => removeTodo(index));

            listItem.appendChild(removeButton);
            todoList.appendChild(listItem);
        });
    };

    // Function to add a new to-do item
    todoForm.addEventListener('submit', async (event) => {
        event.preventDefault();
        const newTodo = {
            title: todoTitleInput.value,
            completed: false
        };
        await addTodo(newTodo);
        todoTitleInput.value = ''; // Clear the input field
    });

    // Function to send a POST request to add a new to-do
    const addTodo = async (newTodo) => {
        try {
            // Fetch the current todos
            const response = await fetch('todos.json');
            const data = await response.json();

            // Add the new todo to the current list
            data.todos.push(newTodo);

            // Send the updated list back to the server
            await saveTodos(data.todos);
        } catch (error) {
            console.error('Error adding todo:', error);
        }
    };

    // Function to remove a to-do item
    const removeTodo = async (index) => {
        try {
            // Fetch the current todos
            const response = await fetch('todos.json');
            const data = await response.json();

            // Remove the specified todo from the list
            data.todos.splice(index, 1);

            // Send the updated list back to the server
            await saveTodos(data.todos);
        } catch (error) {
            console.error('Error removing todo:', error);
        }
    };

    // Function to save the full to-do list to the server
	const saveTodos = async (todos) => {
		console.log(JSON.stringify({ todos }, null, 2));
		try {
			const response = await fetch('todos.json', {
				method: 'POST',
				headers: {
					'Content-Type': 'application/json'
				},
				// Use a second argument in JSON.stringify to add indentation
				body: JSON.stringify({ todos }, null, 2) // Indent with 2 spaces
			});
			if (!response.ok) throw new Error('Network response was not ok');
			loadTodos(); // Reload the todo list
		} catch (error) {
			console.error('Error saving todos:', error);
		}
	};

    loadTodos(); // Initial load of the to-do list
});
