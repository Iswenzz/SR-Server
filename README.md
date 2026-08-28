# SR CoD4x

A modification of the Call of Duty 4: Modern Warfare server. CoD4x fixes several bugs in the original binaries and lets developers extend server functionality with extra variables and plugins. When a player joins a CoD4x server, their client is automatically updated to the CoD4x client, which fixes known base-game client bugs and, together with the server, unlocks extra features.

## Features

* Administration commands
* Plugin system
* Measures against hackers
* Reliable player identification
* Pbss-like screenshot feature
* Backwards compatibility with 1.7 and 1.7a servers
* Support for stock 1.7 clients alongside CoD4X clients
* A new masterserver for when the official one is down
* gsclib standard library for the GSC scripting language
* Extended player movements including: 190, 210, Q3, Q3CPM, CS
* Increased assets pool
* Demo recording and playback
* Player FPS calculation
* Netcode optimization
* Voice chat proximity
* Sound assets streaming over voice protocol

## Setting up a server
You also require the base game to run a server. Copy every .iwd file in `client/main` to `server/main`.
Also copy everything inside `client/zone` to `server/zone`.

Now you can run the server with `./cod4x18_dedrun +map mp_killhouse`. If you are running a local server on Windows use `cod4x18_dedrun.exe +map mp_killhouse +set dedicated 2 +set net_ip 127.0.0.1`. Join the server with your client via the console (`^`) by typing `/connect 127.0.0.1` (if hosted locally), and see if you can join the server.

A more detailed server tutorial is available on [our wiki](https://github.com/callofduty4x/CoD4x_Server/wiki/Server-setup).

## Building (Windows)

_Pre-Requisites:_

1. [MinGW](https://github.com/brechtsanders/winlibs_mingw/releases/download/15.2.0posix-13.0.0-msvcrt-r5/winlibs-i686-posix-dwarf-gcc-15.2.0-mingw-w64msvcrt-13.0.0-r5.zip)
2. [CMake](https://cmake.org/) and [vcpkg](https://vcpkg.io/en/)
3. [NASM](https://www.nasm.us/)

_Build Command:_

    install.bat

## Building (Linux)

_Pre-Requisites:_

1. [CMake](https://cmake.org/) and [vcpkg](https://vcpkg.io/en/)

_Pre-Requisites:_

    sudo dpkg --add-architecture i386
    sudo apt update
    sudo apt install build-essential gcc-multilib g++-multilib pkg-config linux-libc-dev nasm:i386

_Build Command:_

    install.sh

## [Download](https://github.com/Iswenzz/SR-CoD4x/releases)

## Contributors:

**_Note:_** If you would like to contribute to this repository, feel free to send a pull request, and I will review your code. Also feel free to post about any problems that may arise in the issues section of the repository.

