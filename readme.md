# Cli Chat

![Status](https://img.shields.io/badge/status-development-orange) ![Language](https://img.shields.io/badge/language-C-blue)

**Cli Chat** is a simple console chat application thats being developed alongside [https://github.com/WoutervanDixhoorn/Minimal-Sockets](`msock.h`). 
The goal is to learn the fundatmentals of application design in C and have a proper use case for `msock.h`
to stimulate its development and mature the library.

## Goals
1. Have a very basic chat application with usernames that can be used. Accept visual glitches.
2. Expend the `msock.h` library to support sending packets instead of plain text.
3. Build a simple *console application engine* to port this chat application to and eliminate the visual glitches, this will be a different project.
4. ....

## Getting Started

### Prerequisites
* GCC/Clang (or any standard C compiler)

### Usage
1. Just clone the repo using `git clone ....`.
2. * To build the examples just bootstrap the nob.c by compling it one time into nob.exe and just run. To include debug symbols run `.\nob.exe -d`
3. Run the `clichat.exe` from the console. To host the server run `clichat.exe -s`.

> **Note:** This application currenly only works on localhost (127.0.0.1) on port 420 as of now. This is changing while the project matures.

## References
* Tsoding (Nobuild): This project makes use of the [Nobuild](https://github.com/tsoding/nobuild) concept by Tsoding. 
A huge shoutout to the [Tsoding Daily](https://www.youtube.com/@TsodingDaily) channel. his content inspired me to start learning C for fun!
* [https://github.com/WoutervanDixhoorn/Minimal-Sockets](Minimal Sockets): This is my own simple project developed parralel from this chat application.

## Note
* Please if you have any tips or input for the project, feel free to create an issue or a pull request!
