# check if build directory exists
#
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
else
    echo "Build directory already exists."
fi

cd build
cmake ..
make
cd ..
./build/my_b
