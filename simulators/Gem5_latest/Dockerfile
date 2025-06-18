FROM ubuntu:22.04

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install essential dependencies
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
    python3-dev \
    python3-pip \
    libboost-all-dev \
    pkg-config \
    libhdf5-serial-dev \
    libpng-dev \
    libelf-dev \
    wget \
    curl \
    libunwind-dev \
    libpcap-dev \
    libnetfilter-queue-dev \
    libncurses-dev \
    libtcmalloc-minimal4 \
    libssl-dev \
    libbz2-dev \
    gdb \
    cmake \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Install Python packages
RUN pip3 install --no-cache-dir -U pip && \
    pip3 install --no-cache-dir \
    numpy \
    matplotlib \
    pydot \
    six \
    PyYAML \
    termcolor \
    pyelftools \
    pydot \
    pytest

# Create workdir
WORKDIR /gem5

# Clone the latest gem5 repository
RUN git clone https://github.com/gem5/gem5.git .

# Build ARM version
RUN scons build/ARM/gem5.opt -j$(nproc)


# Set up environment
ENV PATH="/gem5/build/ARM/gem5.opt:/gem5/build/X86/gem5.opt:${PATH}"

# Create a directory for simulation outputs
RUN mkdir -p /outputs

WORKDIR /gem5

# Default command (can be overridden)
CMD ["/bin/bash"]