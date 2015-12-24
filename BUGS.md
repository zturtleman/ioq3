#BUGS
* Issue where the client/server will crash with the error message: OP_BLOCK_COPY out of range!  I'm unable to reproduce the issue consistently, but it does happen, even though rarely.
* In Linux, when using KDE3, KDE4 or Trinity desktop managers, the "vid_restart" command will repeatedly execute over and over until the game crashes, since the engine believes the resolution settings changed right after each restart.  The game also seems to want the resolution to be set to 640x480.  This does not happen in other desktop managers or environments.  This issue comes from IOQuake3.
* Some weapons and icons will not load initially when an arena type mode is set.
* Some weapon beam projectiles (eg. lightning) won't appear when using the "OpenGL2" rendering mode.  Either it will happen when you start a map, or change to a new map.  A "vid_restart" will fix the issue for the current session.  I'm able to reproduce it in OpenArena as well, however, have yet to test it in Quake 3.
* AAS file checksums for maps are incorrect when 64bit BSPC checksums are made.  Currently, I have AAS checksum checking disabled, until a fix is made in BSPC (64bit version) to fix the checksum error.  Sorry.  :P
* The message is currently disabled, but playing with bots shows some "weapon number out of range" error messages.  Currently the current bot code is mangled with code from Quake 3 and OpenArena, and will fix this when I bother to learn how to make it work correctly.  If anyone wants to jump in, and provide a fix, that would be nice.  :3
* Practice mode custom options are incomplete, and will not start a match.
* UI menu looks misaligned.  This needs to be redesigned from scratch.
* Spreadshot (Plasma gun upgrade) has a magenta muzzle flash.  It looks wrong in other words.
* Spreadshot (Plasma gun upgrade) makes a loud sound when shooting yourself in the foot.  I would too.
* Loading screen (after the connection attempt screen) looks messy, and misaligned.  This screen is incomplete, and will be fixed some time later.
* Shotgun pellets for clients other than slot 0, will see twice the amount of pellets shot.  For some reason, the lagged pellets are displayed along with the delagged pellets.
* Hit-scan weapons such as the shotgun and chaingun will show an enemy getting hit or not, but it does not reflect what the server sees.  In other words, What You See, Is -Not- What You Get.
* Since the update to SDL2, mouse input sensitivity is doubled (perhaps depending on the system, but I can confirm on Linux).  Workaround is to half the number set to the sensitivity setting (eg: 7 -> 3.5), while just moving around slower in the game's UI menu.  :/
