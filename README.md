# Dependencies
## SDL
### MacOS
`brew install sdl2`
### Linux
`sudo apt install libsdl2-dev`

### Windows 
Download the development pack `SDL2-devel-2.0.5-VC.zip` from [here](https://github.com/libsdl-org/SDL/releases/tag/release-2.26.2)

Or use winget or choco

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

# Usage
After building, run the emulator with:
```
./gbemu <boot_rom_path> <game_rom_path>
```

Example:
```
./gbemu ../src/dmg_boot.gb ../tests/tetris.gb
```

Arguments:
- `boot_rom_path`: Path to the DMG boot ROM file (e.g., dmg_boot.gb)
- `game_rom_path`: Path to the Game Boy ROM file to run

