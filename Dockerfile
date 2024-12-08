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

# Copy your C++ server code to the container
COPY . /app
RUN echo HELLO
RUN ls -l /app
# RUN make fclean && make

# Expose the server port
EXPOSE 8002

# CMD to run the C++ server or Python code, e.g., 
# You can adjust the server command accordingly (use the virtual environment for Python)
# CMD ["./socket", "configuration/default.conf"]

# Alternatively, if running Python, you can use:
# CMD ["/venv/bin/python3", "/app/cgi-bin/edit_image.py"]
