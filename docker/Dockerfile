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

################################
# Download system dependencies #
################################
RUN apt update && \
    apt install -y cmake g++ pip wget git && \
    apt install -y libasio-dev libtinyxml2-dev && \
    apt install -y libssl-dev && \
    apt install -y libyaml-cpp-dev liblz4-dev libzstd-dev && \
    pip3 install -U colcon-common-extensions vcstool

###########################
# Download eProsima repos #
###########################
# Download all dependencies for dds recorder (except the recorder)
RUN mkdir -p /home/recorder/src
COPY recorder.repos /home/recorder/recorder.repos
RUN cd /home/recorder && \
    vcs import src < recorder.repos
# Copy DDS Recorder private repo
COPY ddsrecorder /home/recorder/src

########################
# Build eProsima repos #
########################
RUN cd /home/recorder && \
    colcon build --cmake-args "-DCOMPILE_EXAMPLES=ON"

#################
# Add resources #
#################
COPY resources/ /home/configurations

# Source built workspace
RUN echo "source /home/recorder/install/setup.bash" >> ~/.bashrc
