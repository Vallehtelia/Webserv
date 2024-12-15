# DO NOT DELETE OR MODIFY THIS FILE!!! SAVE IT WITH A DIFFERENT NAME!!!
# THEN USE ANOTHER DOCKERFILE IF NECESSARY
# Use a lightweight Linux distribution
FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive

# Update package list and install necessary tools
ARG CACHE_BUSTER=1
RUN apt-get update && \
    apt-get install -y build-essential g++ cmake siege python3 python3-pip python3-venv && \
    siege --version && \
    python3 --version && \
    ln -fs /usr/share/zoneinfo/Etc/UTC /etc/localtime && \
    dpkg-reconfigure --frontend noninteractive tzdata && \
    rm -rf /var/lib/apt/lists/*

# Create and install a virtual environment for Python packages
RUN python3 -m venv /venv

# Set the PATH environment variable to include the virtual environment
ENV PATH="/venv/bin:$PATH"

# Install Pillow in the virtual environment
RUN pip install --no-cache-dir Pillow

# Set the working directory
WORKDIR /app

# Copy your server code to the container
COPY . /app

# Ensure correct permissions
RUN chmod -R 755 /app

#RUN chmod +r /app/main.cpp
RUN echo HELLO
RUN ls -l /app
RUN make fclean && make

# Expose the server port
EXPOSE 8002

# CMD to run the C++ server or Python code, e.g., 
# You can adjust the server command accordingly (use the virtual environment for Python)
# CMD ["./socket", "configuration/default.conf"]

# Alternatively, if running Python, you can use:
# CMD ["/venv/bin/python3", "/app/cgi-bin/edit_image.py"]

# Set the default command to run the server
CMD ["./socket", "configuration/default.conf"]
