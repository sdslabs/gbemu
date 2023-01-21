# Build
## Release
```
git submodule update --init --recursive
mkdir build && cd build
cmake ..
cmake --build . -j8 --config Release
```

## Debug
``` 
git submodule update --init --recursive
mkdir build && cd build
cmake ..
cmake --build . -j8
```

After this run the binary gbemu in the build folder.