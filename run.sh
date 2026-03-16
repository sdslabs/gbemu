#!/bin/bash

# Default ROM paths (can be overridden with arguments)
BOOT_ROM="../src/dmg_boot.gb"
GAME_ROM="../tests/dmg_sound/rom_singles/02-len ctr.gb"

# Check if arguments were provided
if [ $# -eq 2 ]; then
    BOOT_ROM="$1"
    GAME_ROM="$2"
fi

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
    echo "Using Boot ROM: $BOOT_ROM"
    echo "Using Game ROM: $GAME_ROM"

    if [[ -d "$current_directory/build" ]]; then
        rm -r build
        echo "removing build directory"
        echo "making new build directory"
        mkdir build
        cd build
        cmake ..
        cmake --build . -j8
        ./gbemu "$BOOT_ROM" "$GAME_ROM"
    else 
        echo "making new build directory"
        mkdir build
        cd build
        cmake ..
        cmake --build . -j8
        ./gbemu "$BOOT_ROM" "$GAME_ROM"
    fi

    # Add your commands here
else
    # Default case if no match is found
    echo "No matching directory found."
fi
