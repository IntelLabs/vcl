Currently the VCL works on Ubuntu Linux (v16.04) with OpenCV (v3.3) and TileDB
(v1.3.1), but it should work on any recent version of Ubuntu. It's also been
tested with g++ v5.4.0. If you're having issues, check your Ubuntu and g++
versions first.

## Basic Dependencies
    sudo apt-get install git wget

If your system does not have [scons](http://scons.org/) installed,
run the following:

    sudo apt-get install scons
    sudo apt-get install ffmpeg

[Google Test](https://github.com/google/googletest) is used for the unit tests included in the test folder. To install:

    sudo apt-get install cmake libgtest-dev

Unfortunately this doesn't actually install gtest;
you need to do the following steps to get it to work correctly:

    cd /usr/src/gtest/
    sudo cmake CMakeLists.txt
    sudo make
    sudo cp *.a /usr/lib

[Doxygen](http://www.stack.nl/~dimitri/doxygen/) is used to create the API documentation. To install:

    sudo apt-get install doxygen

## [OpenCV](https://opencv.org/)
The VCL may be fine with newer versions of OpenCV, but below are instructions for installing OpenCV v3.3

Download [OpenCV 3.3.1](https://github.com/opencv/opencv/archive/3.3.1.zip)

    unzip 3.3.1.zip
    cd 3.3.1
    mkdir build
    cd build/
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local ..
    make -jX   # substitute X for maximum number of CPUs (or leave it off to use all available)
    sudo make install

**Note**: When using videos, and getting the following error: "Unable to stop the stream: Inappropriate ioctl for device", you may need to include more flags when compiling OpenCV. Follow these instructions ([source](https://stackoverflow.com/questions/41200201/opencv-unable-to-stop-the-stream-inappropriate-ioctl-for-device)):

    sudo apt-get install ffmpeg
    sudo apt-get install libavcodec-dev libavformat-dev libavdevice-dev

    // Rebuild OpenCV with the following commands:

    cd build
    cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local  -D WITH_FFMPEG=ON -D WITH_TBB=ON -D WITH_GTK=ON -D WITH_V4L=ON -D WITH_OPENGL=ON -D WITH_CUBLAS=ON -DWITH_QT=OFF -DCUDA_NVCC_FLAGS="-D_FORCE_INLINES" ..
    make -j7
    sudo make install

## [TileDB](https://tiledb.io/)
Currently the VCL works with TileDB v1.3.1. It has not been tested with the
Docker image, though please let us know if you try that and it works! The
directions below will help you install TileDB v1.3.1 from source. You can also
follow the directions listed
[here](https://docs.tiledb.io/en/latest/installation.html).

### Dependencies
    sudo apt-get install zlib1g-dev libssl-dev liblz4-dev libbz2-dev

### Install Blosc
    git clone https://github.com/Blosc/c-blosc.git`
    cd c-blosc/
    mkdir build
    cd build/
    cmake -DCMAKE_INSTALL_PREFIX='/usr' ..
    cmake --build .
    ctest
    sudo cmake --build . --target install

### Install ZStandard
    wget https://github.com/facebook/zstd/archive/v1.1.0.tar.gz
    tar xf v1.1.0.tar.gz
    cd zstd-1.1.0
    sudo make install PREFIX='/usr'

### Build TileDB

    git clone https://github.com/TileDB-Inc/TileDB
    cd TileDB
    git checkout 1.3.1
    mkdir build
    cd build
    ../bootstrap --prefix=/usr/local/
    make -jX   # substitute X for maximum number of CPUs (or leave it off to use all available)
    sudo make install

**Note:** If you have previously installed other version of TileDB
on your machine, you will need to remove the existing header file
(/usr/local/include/tiledb.h),
since the newer version of TileDB installs to /usr/local/include/tiledb/.

## [Faiss](https://github.com/facebookresearch/faiss)
Facebook Faiss library for similarity search, used as alternitive engines
on VCL::DescriptorSet.

Download [Faiss 1.2.1](https://github.com/facebookresearch/faiss/archive/v1.2.1.tar.gz)

    tar -xzvf v1.2.1.tar.gz
    cd faiss-1.2.1
    mkdir build
    cd build/
    cmake ..
    make -j

You may need to change some flags in the CMakeFile depending on
configurations on your system.

Things that we have needed to change in CMakeLists.txt:
* If your system doesn't have a GPU, make sure that BUILD_WITH_GPU is OFF
* You may need to add -msse4 to set(CMAKE_CXX_FLAGS
* Change the add_library call to be SHARED instead of STATIC

This library does not offer an install. Make sure you move .h files
to /usr/lib/include/faiss or /usr/local/lib/include/faiss,
and make sure you make the library (libfaiss.so) is available system-wide
Or follow instructions
[here](https://github.com/facebookresearch/faiss/blob/v1.2.1/INSTALL.md)

## Compilation
    git clone https://github.com/IntelLabs/vcl # or download a release
    cd vcl
    scons

Make sure libvcl is on your LD_LIBRARY_PATH

To run the unit tests:

    cd test
    sh run_tests.sh
