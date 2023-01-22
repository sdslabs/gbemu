# Build
## Release
```
git submodule update --init --recursive
mkdir build && cd build
cmake -DDEBUG=off ..
cmake --build . -j8 
```

## Debug
``` 
git submodule update --init --recursive
mkdir build && cd build
cmake -DDEBUG=on ..
cmake --build . -j8
```

After this run the binary gbemu in the build folder.
