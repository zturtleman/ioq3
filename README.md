# MFArena
* A fork off of [IOQuake3](https://github.com/ioquake/ioq3), which brings all of the action and fun from Quake 1 through 3, and even bits of Doom.

* No need to worry about spending years to perfect movement. Tactics, reflex, and aim are the key to have fun.

* MFArena is the Art of Rage and Salt Over the Web.

    *Do note that this is a prototype demo, and the project is simply getting started.*

## Why play MFA, instead of other Quake-like games?
- No Railgun!
- Has a powerful Super Shotgun (it will blast you across the room)
- This is a real Quake clone
- Run a LAN server
- Run a dedicated MFA server, with your own settings, and a mod if needed
- No requirement to have Steam
- No need to log-in to enjoy a game
- No need for server invites
- No "Authentication Failed" errors
- Not a closed alpha development build (...DoomBringer)
- Not a Windows-ONLY game (...QuakeLive ...Reflex ...DoomBringer)
- No loadouts, No global ammo
- No item timers
- You can choose any in-game nick, and change it anytime
- Clan-tag appears by the player's nick on the scoreboard
- No autojump (holding jump/space)
- No hold-forward extra acceleration while jumping
- Bunny hopping and strafe jumping exploits disabled (unless the server has pro-mode turned on)
- Will focus mainly on gameplay, instead of movement (...Reflex)
- Doesn't have the name "Live" in the title
- Vocal taunts
- Doom like keycards (note that this feature was available before QuakeLive had keys)
- No silly broken browser for a main menu, no need to worry about cookies or browser cache
- Blood and guts
- Doesn't have the map Aerowalk (Seriously, if you want it, make it you n00b)
- Adjustable FOV
- No motion blur effect to make you sick
- No need to call a vote just for a coin toss, just type the command "/coinflip"
- Player configs will still work after a major update (...Warsow)
- No need to download server variables (csprogs.dat) everytime you connect to a server (...Xonotic)
- Ignored and unacknowledged by the ArenaFPS community
- Not designed for E-Sports

##MFArena Prototype Notes
* The options thoughout the UI were not updated with the changes made to the game, requiring some options to be set via the console.
* The cvar 'g_promode' enables classic Quake physics modes W-Mode(World/QW) and A-Mode(Arena/VQ3).
* Selecting between 'World' and 'Arena' physics, require binding a key to '+button12' known as the 'special' button, which can be set in the 'misc.' section of the 'controls' menu.
* The cvars 'fraglimit' and 'capturelimit' were replaced with 'scorelimit'.
* There is a rule set enforcer set by default, called 'g_ruleSet'.  Setting it to '0' will allow a custom rule set.  A value of '1' (default) will enforce a standard rule set, locking most rule set cvars (e.g. 'timelimit', 'scorelimit', etc.).  Setting the value to '2', will enable hardcore mode.
* Some of the 'cg_*' cvars that control HUD functions won't work, as they are being replaced with the new HUD system.
* Few of the HUD elements ('hud_*') are hardcoded, and cannot be customized as of yet.
* Installation of the game is currently not user friendly.

##Compiling the engine
* Enter the directory where MFArena's source is cloned (downloaded via git).
* Besure you have install the game assets for MFArena.
* Make sure your system has the SDL2-dev libraries installed.
* Open a console, and type 'make'.
* After the engine compiled successfully, copy (or move) files 'mfaded.*', 'mfarena.*' and 'renderer_opengl*.so' from 'build/release-linux-*' back to the directory where you install MFArena.
* Run 'mfarena' (eg: ./mfarena.x86_64), and enjoy.

*Note: These instructions are for Linux only.*

*Note 2: These instructons are somewhat outdated, but will still work.*

##Misc. Notes
* If you are making a video of this demo, please modify cvar 'hud_iUnderstand' with the value of '2' (or '3').  People will (hopefully) understand that this is a prototype demo, and not the final product, due to the placeholder textures.

## Credits
* [ID](https://github.com/id-Software) - Quake III Arena code release.
* [IOQuake team](https://github.com/ioquake) - Modified Q3A code, with updated changes, including fixes and SDL2 usage.
* [OpenArena team](https://github.com/OpenArena/) - Temporary assets used until replacements are made.
* [Metro-MP](https://github.com/Metro-MP) - Assets, code edits.
* And you, for your interest in MFArena.

*"Winners don't use Windows"*