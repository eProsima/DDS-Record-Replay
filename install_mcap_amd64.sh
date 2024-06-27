#!/usr/bin/env bash

# Install required libraries
apt-get install -y --no-install-recommends liblz4-tool zstd

# Get MCAP v0.0.46 of MCAP for the AMD64 linux arch.
wget https://github.com/foxglove/mcap/releases/download/releases/mcap-cli/v0.0.46/mcap-linux-amd64 -O mcap

# Install the script and move it to the bin
chmod +x mcap && mv mcap /usr/bin
