# run this using 	docker build --no-cache -t cpp-webserver . 2>&1 | tee build.log
# or just 			docker build -t cpp-webserver .
# then 				docker run -d -p 8002:8002 --name webserverc cpp-webserver
# or without name 	docker ps
# then				docker stop <id from ps> or webserverc
# Use a lightweight Linux distribution like Ubuntu
FROM ubuntu:20.04

# Add a line to inspect the sources.list
RUN cat /etc/apt/sources.list

# Add necessary repositories
RUN echo "deb http://archive.ubuntu.com/ubuntu focal main restricted universe multiverse" > /etc/apt/sources.list && \
    echo "deb http://archive.ubuntu.com/ubuntu focal-updates main restricted universe multiverse" >> /etc/apt/sources.list && \
    echo "deb http://security.ubuntu.com/ubuntu focal-security main restricted universe multiverse" >> /etc/apt/sources.list
	
# Update package list and install necessary tools
RUN apt-get update && \
    apt-get dist-upgrade -y && \
    #  apt-mark unhold $(dpkg --get-selections | grep hold | awk '{print $1}') && \
    apt-get install -y --fix-missing g++ && \
    apt-get install -y --fix-missing make && \
    apt-get install -y --fix-missing build-essential && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Copy your C++ server code to the container
COPY . /app
#RUN chmod +r /app/main.cpp
#RUN ls -l /app

# Compile the code
RUN make

# Expose the server port
EXPOSE 8002

# Run the server
CMD ["./socket", "configuration/default.conf"]
