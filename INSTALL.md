Currently the VCL works on Ubuntu Linux (v16.04) with OpenCV (v3.3) and TileDB (v0.6.1), but it should work on any recent version of Ubuntu. It's also been tested with g++ v5.4.0. If you're having issues, check your Ubuntu and g++ versions first. 

## Basic Dependencies
    sudo apt-get install git wget

If your system does not have [scons](http://scons.org/) installed, run the following:

    sudo apt-get install scons

[Google Test](https://github.com/google/googletest) is used for the unit tests included in the test folder. To install:

    sudo apt-get install cmake libgtest-dev

Unfortunately this doesn't actually install gtest; you need to do the following steps to get it to work correctly:

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
    make -jX   # substitute X for a maximum number of CPUs to use (or leave it off to use all available)
    sudo make install  

## [TileDB](https://tiledb.io/)
Currently the VCL works with TileDB v0.6.1. Since it is an older version, please follow the installation instructions below. 
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
Download [TileDB 0.6.1](https://github.com/TileDB-Inc/TileDB/archive/0.6.1.zip)

    unzip 0.6.1.zip
    cd 0.6.1/
    mkdir build
    cd build/
    ifconfig 

**NOTE** if your MAC address is not eth0, you need to include the compilation flag -DMAC_ADDRESS_INTERFACE="your_interface" in the next command

    cmake [flag] ..
    make -jX # substitute X for a maximum number of CPUs to use (or leave it off to use all available)
    sudo make install

## Compilation
    git clone https://github.com/IntelLabs/vcl # or download a release
    cd vcl
    scons

Make sure libvcl is on your LD_LIBRARY_PATH

To run the unit tests:

    mkdir test/image_results
    ./test/unit_test

Right now, three tests may fail (TDBImageTest.CopyConstructor, TDBImageTest.DeleteImage, ImageDataTest.DeleteTDB), this is a known issue with the 0.6.1 version of TileDB.
