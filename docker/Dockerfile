#########################################################################################
# DDS Recorder Demos Dockerfile
#########################################################################################

FROM ubuntu:jammy

# Avoids using interactions during building
ENV DEBIAN_FRONTEND=noninteractive

# Use a bash shell so it is possigle to run things like `source` (required for colcon builds)
SHELL ["/bin/bash", "-c"]

# Avoid interactuation with installation of some package that needs the locale.
ENV TZ=Europe/Madrid
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Download DDS Recorder dependencies
RUN apt update && \
    apt install -y \
        cmake \
        g++ \
        pip \
        wget \
        git \
        libasio-dev \
        libtinyxml2-dev \
        libssl-dev \
        libyaml-cpp-dev \
        liblz4-dev \
        libzstd-dev && \
    pip3 install -U colcon-common-extensions vcstool

# Download eProsima dependencies
WORKDIR /ddsrecorder
RUN wget https://raw.githubusercontent.com/eProsima/DDS-Recorder/feature/docker/docker/recorder.repos && \
    mkdir src && \
    vcs import src < recorder.repos

# Build DDS Recorder
RUN colcon build

# Add DDS Recorder configuration files
COPY resources resources

# Build TypeIntrospection example
WORKDIR /dyn_types_example
RUN cp -r /ddsrecorder/src/fastdds/examples/cpp/dds/TypeIntrospectionExample/* . && \
    source /ddsrecorder/install/setup.bash && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make

# Source built workspace
RUN echo "source /ddsrecorder/install/setup.bash" >> ~/.bashrc

# Restore working directory to default
WORKDIR /
