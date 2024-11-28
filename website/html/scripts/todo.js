document.addEventListener('DOMContentLoaded', () => {
    const todoList = document.getElementById('todo-list');
    const todoForm = document.getElementById('todo-form');
    const todoTitleInput = document.getElementById('todo-title');

    const loadTodos = async () => {
        try {
            const response = await fetch('/todo/todos.json'); 
            if (!response.ok) throw new Error('Network response was not ok');
            const data = await response.json();
            renderTodos(data.todos);
        } catch (error) {
            console.error('Error loading todos:', error);
        }
    };

    const renderTodos = (todos) => {
        todoList.innerHTML = ''; 
        todos.forEach((todo, index) => {
            const listItem = document.createElement('li');
            listItem.textContent = todo.title.toUpperCase();
            listItem.className = todo.completed ? 'completed' : 'not-completed';
            const buttonContainer = document.createElement('div');
            const toggleButton = document.createElement('button');
            const completedText = document.createElement('p');
            completedText.className = "completed-text";
            completedText.textContent = "COMPLETED: ";
            toggleButton.textContent = " ";
            toggleButton.className = 'toggle';
            toggleButton.addEventListener('click', () => toggleTodo(index));
            
            const removeButton = document.createElement('button');
            removeButton.textContent = 'REMOVE';
            removeButton.className = 'remove';
            removeButton.addEventListener('click', () => removeTodo(index));
            
            buttonContainer.className = 'button-container';
            
            buttonContainer.appendChild(completedText);
            buttonContainer.appendChild(toggleButton);
            buttonContainer.appendChild(removeButton);
            listItem.appendChild(buttonContainer);
            todoList.appendChild(listItem);
        });
    };

    todoForm.addEventListener('submit', async (event) => {
        event.preventDefault();
        const newTodo = {
            title: todoTitleInput.value,
            completed: false
        };
        await addTodo(newTodo);
        todoTitleInput.value = ''; 
    });

    const addTodo = async (newTodo) => {
        try {
            const response = await fetch('/todo/todos.json');
            const data = await response.json();
            data.todos.push(newTodo);
            await saveTodos(data.todos);
        } catch (error) {
            console.error('Error adding todo:', error);
        }
    };

    const removeTodo = async (index) => {
        try {
            const response = await fetch('/todo/todos.json');
            const data = await response.json();
            data.todos.splice(index, 1);
            await saveTodos(data.todos);
        } catch (error) {
            console.error('Error removing todo:', error);
        }
    };

    const toggleTodo = async (index) => {
        try {
            const response = await fetch('/todo/todos.json');
            const data = await response.json();
            data.todos[index].completed = !data.todos[index].completed; // Toggle completed status
            await saveTodos(data.todos);
        } catch (error) {
            console.error('Error toggling todo:', error);
        }
    };

    const saveTodos = async (todos) => {
        console.log(JSON.stringify({ todos }, null, 2));
        try {
            const response = await fetch('/todo/todos.json', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({ todos }, null, 2)
            });
            if (!response.ok) throw new Error('Network response was not ok');
            loadTodos(); 
        } catch (error) {
            console.error('Error saving todos:', error);
        }
    };

    loadTodos(); 
});
