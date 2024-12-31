# Webserv

Welcome to **Webserv**, a lightweight HTTP server developed as part of the 42 School curriculum. This project showcases our understanding of networking, HTTP protocol handling, and creating a robust, scalable server capable of hosting web applications.

## 🌟 Features
- **HTTP/1.1 Compliance**: Fully supports HTTP/1.1 protocol specifications.
- **Request Parsing**: Handles GET, POST, and DELETE methods.
- **Static File Hosting**: Serve static HTML, CSS, JS, and other files.
- **Dynamic Content Handling**: Supports CGI scripts for dynamic content generation.
- **Multiple Configurations**:
  - Host multiple servers on different ports.
  - Configure routes, methods, and error pages.
  - Set custom client body size limits.
- **Logging**: User-friendly logging for debugging and tracking requests.
- **Error Handling**: Customizable error pages for different HTTP error codes.
- **Persistent Connections**: Implements connection keep-alive for better performance.

## 🚀 Getting Started

### Prerequisites
- A UNIX-based system (Linux or macOS).
- `gcc` or any C++ compiler supporting C++98 standard.
- Basic knowledge of HTTP and networking.

### Installation
1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/webserv.git
   cd webserv

2. Build the project:
   ```bash
   make

3. Run the server:
   ```bash
   ./webserv [configuration_file]
   Example: ./webserv configuration/default.conf
   ```

### Usage 🛠
Once the server is running, you can test it using a web browser, `curl`, or any HTTP client. Here are some examples:

### Example Requests
```bash
# Access a static file
curl http://localhost:8080/index.html
```

### Send a POST request
```bash
curl -X POST -d "key=value" http://localhost:8080/form
```

### Delete a resource
```bash
curl -X DELETE http://localhost:8080/resource
```


### Learning Outcomes
## 📚 Learning Outcomes
This project reinforced our understanding of:
- The HTTP/1.1 protocol and its specifications.
- Socket programming and handling client-server communication.
- Parsing and validating HTTP requests and responses.
- Managing multiple connections using `poll()`.
- Implementing CGI scripts for dynamic content.
- Building robust and scalable software with proper error handling.

## 🧑‍💻 Authors
- **Valle Vaalanti**
- **Olli Karejoki**
- **Marek Burakowski**


