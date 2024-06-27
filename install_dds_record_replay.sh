#!/usr/bin/env bash


######################### Install cmake utils and cpp utils
mkdir build/cmake_utils
cd build/cmake_utils
cmake ../../dev-utils/cmake_utils 
cmake --build . --target install -j${nproc}
cd ../..


mkdir build/cpp_utils
cd build/cpp_utils
cmake ../../dev-utils/cpp_utils
cmake --build . --target install -j${nproc}
cd ../..


mkdir build/ddspipe_core
cd build/ddspipe_core
cmake ../../DDS-Pipe/ddspipe_core 
cmake --build . --target install -j${nproc}
cd ../..

mkdir build/ddspipe_participants
cd build/ddspipe_participants
cmake ../../DDS-Pipe/ddspipe_participants 
cmake --build . --target install -j${nproc}
cd ../..


mkdir build/ddspipe_yaml
cd build/ddspipe_yaml
cmake ../../DDS-Pipe/ddspipe_yaml 
cmake --build . --target install -j${nproc}
cd ../..

# ddsrecorder_participants
mkdir build/ddsrecorder_participants
cd build/ddsrecorder_participants
cmake ../../ddsrecorder_participants 
cmake --build . --target install -j${nproc}
cd ../..

# ddsrecorder_yaml
mkdir build/ddsrecorder_yaml
cd build/ddsrecorder_yaml
cmake ../../ddsrecorder_yaml 
cmake --build . --target install -j${nproc}
cd ../..


# ddsrecorder
mkdir build/ddsrecorder_tool
cd build/ddsrecorder_tool
cmake ../../ddsrecorder
cmake --build . --target install -j${nproc}
cd ../..


# ddsreplayer
mkdir build/ddsreplayer_tool
cd build/ddsreplayer_tool
cmake ../../ddsreplayer 
cmake --build . --target install -j${nproc}
cd ../..



mkdir build/fastdds_python
cd build/fastdds_python
cmake ../../Fast-DDS-python/fastdds_python
cmake --build . --target install -j${nproc}

ldconfig
# export LD_LIBRARY_PATH=/usr/local/lib

