FROM ubuntu:18.04

# Avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies including 32-bit libraries
RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    g++ \
    make \
    libxml2-dev \
    gcc-multilib \
    g++-multilib \
    libc6-dev-i386

# Set working directory
WORKDIR /mcpat

# Clone McPAT repository
RUN git clone https://github.com/HewlettPackard/mcpat .

# Checkout version 1.3.0
RUN git checkout v1.3.0

# Build McPAT
RUN make

# Command to run when container starts
CMD ["/bin/bash"]