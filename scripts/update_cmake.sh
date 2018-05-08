sudo apt remove cmake
sudo apt purge --auto-remove cmake

# Download src files: https://cmake.org/files/v3.11/cmake-3.11.1.tar.gz
version=3.11
build=1
mkdir ~/temp
cd ~/temp
wget https://cmake.org/files/v$version/cmake-$version.$build.tar.gz
tar -xzvf cmake-$version.$build.tar.gz
cd cmake-$version.$build/
./bootstrap
make -j4
sudo make install