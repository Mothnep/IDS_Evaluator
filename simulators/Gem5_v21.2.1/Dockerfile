FROM ubuntu:18.04

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    m4 \
    scons \
    zlib1g \
    zlib1g-dev \
    libprotobuf-dev \
    protobuf-compiler \
    libprotoc-dev \
    libgoogle-perftools-dev \
    python-dev \
    python-pip \
    python3.6 \
    python3.6-dev \
    python3-pip \
    libboost-all-dev \
    pkg-config

# Set Python 3.6 as default python3
RUN ln -sf /usr/bin/python3.6 /usr/bin/python3

# Install required Python packages for both Python 2 and 3
RUN pip install six pydot numpy
RUN pip3 install six pydot numpy

# Create working directory
WORKDIR /gem5

# Clone gem5 repository from the new GitHub location
RUN git clone https://github.com/gem5/gem5.git .

# Checkout the official v21.2.1.0 release tag
RUN git checkout v21.2.1.0

# Make Python environment available to SCons
ENV PYTHONPATH="/usr/local/lib/python3.6/dist-packages:/usr/lib/python3/dist-packages"

# Build gem5 for ARM with explicit Python 3 usage
RUN python3 $(which scons) build/ARM/gem5.opt -j$(nproc)

# Command to run when container starts
CMD ["/bin/bash"]
