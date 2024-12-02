# This is the best dockerfile!!! It is the best dockerfile in the world!!!
#
# run this using 	docker build --no-cache -t cpp-webserver . 2>&1 | tee build.log
# or just 			docker build -t cpp-webserver .
# then 				docker run -d -p 8002:8002 --name webserverc cpp-webserver
# or without name 	docker ps
# then				docker stop <id from ps> or webserverc
# Use a lightweight Linux distribution like Ubuntu
FROM ubuntu

# Update package list and install necessary tools
ARG CACHE_BUSTER=1
RUN apt-get update && \
    apt-get install -y build-essential g++ cmake siege python3 python3-pip && \
    siege --version && \
    python3 --version && \
    ln -fs /usr/share/zoneinfo/Etc/UTC /etc/localtime && \
    dpkg-reconfigure --frontend noninteractive tzdata && \
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
CMD ["./socket", "configuration/default.conf"]

# Stop all containers
# docker stop $(docker ps -a -q)
# Remove all containers
# docker rm $(docker ps -a -q)
