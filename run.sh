#!/bin/bash

current_directory=$(pwd)
last_keyword=$(basename "$current_directory")

if [[ $last_keyword == "build" ]]; then
    # Execute commands for the specified directory
    echo "Executing commands for build"
    echo "removing build directory"
    cd ..
    rm -r build
    cd ..

    # Add your commands here
elif [[ $last_keyword == "gbemu" ]]; then
    # Execute commands for another directory
    echo "Executing commands for gbemu"

    if [[ -d "$current_directory/build" ]]; then
        rm -r build
        echo "removing build directory"
        echo "making new build directory"
        mkdir build
        cd build
        cmake -DDEBUG=on ..
        cmake --build . -j8
        ./gbemu
    else 
        echo "making new build directory"
        mkdir build
        cd build
        cmake -DDEBUG=on ..
        cmake --build . -j8
        ./gbemu
    fi

    # Add your commands here
else
    # Default case if no match is found
    echo "No matching directory found."
fi
