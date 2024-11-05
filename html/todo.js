document.addEventListener('DOMContentLoaded', () => {
    const todoList = document.getElementById('todo-list');
    const todoForm = document.getElementById('todo-form');
    const todoTitleInput = document.getElementById('todo-title');

    const loadTodos = async () => {
        try {
            const response = await fetch('todos.json'); 
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
            listItem.textContent = todo.title;
            listItem.className = todo.completed ? 'completed' : 'not-completed';

            const toggleButton = document.createElement('button');
            toggleButton.textContent = todo.completed ? 'Mark as Incomplete' : 'Mark as Complete';
            toggleButton.className = 'toggle';
            toggleButton.addEventListener('click', () => toggleTodo(index));

            const removeButton = document.createElement('button');
            removeButton.textContent = 'Remove';
            removeButton.className = 'remove';
            removeButton.addEventListener('click', () => removeTodo(index));

            listItem.appendChild(toggleButton);
            listItem.appendChild(removeButton);
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
            const response = await fetch('todos.json');
            const data = await response.json();
            data.todos.push(newTodo);
            await saveTodos(data.todos);
        } catch (error) {
            console.error('Error adding todo:', error);
        }
    };

    const removeTodo = async (index) => {
        try {
            const response = await fetch('todos.json');
            const data = await response.json();
            data.todos.splice(index, 1);
            await saveTodos(data.todos);
        } catch (error) {
            console.error('Error removing todo:', error);
        }
    };

    const toggleTodo = async (index) => {
        try {
            const response = await fetch('todos.json');
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
            const response = await fetch('todos.json', {
                method: 'PUT',
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