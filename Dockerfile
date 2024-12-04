# Use the official Ubuntu 20.04 image
FROM ubuntu:20.04

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

# Copy the server code into the container
COPY . /app

# Build the server using a Makefile
RUN make FLAGS="-std=c++17 -Wall -Wextra -Werror -fPIE -pie"

# Expose the server port
EXPOSE 8002 8003

# Run the server with the configuration file
CMD ["./socket", "configuration/default.conf"]
