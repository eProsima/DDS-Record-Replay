#!/usr/bin/env bash

apt-get install liblz4-dev libzstd-dev -y
pip3 install PyQt6 -y
apt-get install swig libpython3-dev -y
#Assumes you have installed Fast-DDS, Fast-CDR, Foonathan Memory


######################### Install cmake utils and cpp utils
mkdir build/cmake_utils
cd build/cmake_utils
cmake ../../dev-utils/cmake_utils 
cmake --build . --target install
cd ../..


mkdir build/cpp_utils
cd build/cpp_utils
cmake ../../dev-utils/cpp_utils
cmake --build . --target install
cd ../..


mkdir build/ddspipe_core
cd build/ddspipe_core
cmake ../../DDS-Pipe/ddspipe_core 
cmake --build . --target install 
cd ../..

mkdir build/ddspipe_participants
cd build/ddspipe_participants
cmake ../../DDS-Pipe/ddspipe_participants 
cmake --build . --target install 
cd ../..


mkdir build/ddspipe_yaml
cd build/ddspipe_yaml
cmake ../../DDS-Pipe/ddspipe_yaml 
cmake --build . --target install 
cd ../..

# ddsrecorder_participants
mkdir build/ddsrecorder_participants
cd build/ddsrecorder_participants
cmake ../../ddsrecorder_participants 
cmake --build . --target install
cd ../..

# ddsrecorder_yaml
mkdir build/ddsrecorder_yaml
cd build/ddsrecorder_yaml
cmake ../../ddsrecorder_yaml 
cmake --build . --target install 
cd ../..


# ddsrecorder
mkdir /DDS/DDS-Record-Replay/build/ddsrecorder_tool
cd /DDS/DDS-Record-Replay/build/ddsrecorder_tool
cmake /DDS/DDS-Record-Replay/ddsrecorder
cmake --build . --target install 


# ddsreplayer
mkdir /DDS/DDS-Record-Replay/build/ddsreplayer_tool
cd /DDS/DDS-Record-Replay/build/ddsreplayer_tool
cmake /DDS/DDS-Record-Replay/ddsreplayer 
cmake --build . --target install



git clone https://github.com/eProsima/Fast-DDS-python.git
cd Fast-DDS-python
git checkout v1.4.1
cd ..

mkdir /DDS/DDS-Record-Replay/build/fastdds_python
cd /DDS/DDS-Record-Replay/build/fastdds_python
cmake /DDS/DDS-Record-Replay//Fast-DDS-python/fastdds_python
cmake --build . --target install

ldconfig
# export LD_LIBRARY_PATH=/usr/local/lib

