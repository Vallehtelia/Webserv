# run this using 	docker build --no-cache -t cpp-webserver . 2>&1 | tee build.log
# or just 			docker build -t cpp-webserver .
# then 				docker run -d -p 8002:8002 --name webserverc cpp-webserver
# or without name 	docker ps
# then				docker stop <id from ps> or webserverc
# Use a lightweight Linux distribution like Ubuntu
FROM ubuntu

# Update package list and install necessary tools
RUN apt-get update && \
    apt-get install -y build-essential && \
    apt-get install -y g++ && \
    apt-get install -y cmake && \
    rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Copy your C++ server code to the container
COPY . /app
#RUN chmod +r /app/main.cpp
RUN echo HELLO
RUN ls -l /app
RUN make fclean && make


# Expose the server port
EXPOSE 8002

# Run the server
# CMD ["./socket", "configuration/default.conf"]
