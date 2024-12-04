# Use the official Ubuntu 20.04 image
FROM ubuntu:20.04

# Inspect the architecture and sources list
RUN uname -m && cat /etc/apt/sources.list

# Add necessary repositories explicitly
RUN echo "deb http://archive.ubuntu.com/ubuntu focal main restricted universe multiverse" > /etc/apt/sources.list && \
    echo "deb http://archive.ubuntu.com/ubuntu focal-updates main restricted universe multiverse" >> /etc/apt/sources.list && \
    echo "deb http://security.ubuntu.com/ubuntu focal-security main restricted universe multiverse" >> /etc/apt/sources.list

# Ensure apt-get doesn't prompt for user input
ENV DEBIAN_FRONTEND=noninteractive

# Update package lists and install necessary tools
RUN apt-get update && \
    apt-get dist-upgrade -y && \
    apt-get install -y g++ make build-essential && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Copy the server code into the container
COPY . /app

# Build the server using a Makefile
RUN make FLAGS="-std=c++17 -Wall -Wextra -Werror -fPIE -pie"

# Expose the port for the server
EXPOSE 8002

# Run the server with the configuration file
CMD ["./socket", "configuration/default.conf"]
