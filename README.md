# Allefresher

A tool for patching LittleBigPlanet for the Playstation Portable to allow connecting to custom servers.

## Installation

Copy to the `SEPLUGINS` folder on your PSP, and create/edit a file called `game.txt` in that folder too.

Place the text `ef0:/SEPLUGINS/Allefresher.prx 1` into that text file

You should have `SEPLUGINS/Allefresher.prx` and `SEPLUGINS/game.txt` on the root of your PSP's storage

Open LBP and have fun!

## Building from source

### Setup

Install [pspsdk](https://pspdev.github.io/pspsdk/) onto your system

### Compiling

`$ make`
