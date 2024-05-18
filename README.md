# Allefresher

A set of CFW plugins for patching LittleBigPlanet games to allow connecting to custom servers.

## Platform Support

- PSP (LBP PSP)
- Vita (LBP Vita/LBP PSP)

## Installation (PSP)

Copy `Allefresher.prx` to the `SEPLUGINS` folder on your PSP, and create/edit a file called `game.txt` in that folder too. This can be on your memory stick (`ms0:`) or the internal storage (`ef0:`). Keep note of which you put it on!

If you put the plugin in your memory stick, place `ms0:/SEPLUGINS/Allefresher.prx 1` into `game.txt`, however if you put it on your internal storage, place `ef0:/SEPLUGINS/Allefresher.prx 1` into the file

Write the domain into `/SEPLUGINS/Allefresher_domain.txt`, and write the format string into `/SEPLUGINS/Allefresher_format.txt` (these will be provided by your server host)

Open LBP and have fun!

## Installation (Vita)

Copy `Allefresher_vita.prx` to the `ur0:tai` folder, then add it to `ur0:config.txt` for your LBP Vita title ID.

Open LBP Vita and have fun!

## Building from source (PSP)

### Setup

Install [pspsdk](https://pspdev.github.io/pspsdk/) onto your system

### Compiling

`$ make`
