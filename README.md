# Space Ava 201

This is the code for my game *Space Ava 201*. If you simply want to get the game, I recommend my [itch.io page](https://nicole-express.itch.io/space-ava-201) instead.

## Building

This game requires HuC 3.99 to build. Put the route of HuC in the src/Makefile.

Additionally, if you wish to have the CD error function in your built copy, you must make some modifications to your HuC library.asm. These are detailed in src/cderr/cderr.c.

Finally, some of the mod files are not converted to WAV format in order to save space. You can see which ones need to be manually converted in the Makefile or you can just convert them all into .wav files of the same name as the .mod. Without these WAV files present the build will not complete.

With all of these steps completed, create a folder "build" in the root, a folder "iso" inside of that folder, and touch an empty file (name irrelevant) in the "iso" folder. Then, you can build the game.

## Licensing

This code is offered AS-IS with no promise of support or correct functionality.
