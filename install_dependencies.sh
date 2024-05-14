#!/usr/bin/env bash

apt-get install liblz4-dev libzstd-dev -y
pip3 install PyQt6 -y
apt-get install swig libpython3-dev -y

./install_dds_record_replay.sh