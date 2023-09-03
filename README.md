# wipEout Rewrite

This is a re-implementation of the 1995 PSX game wipEout.

Play here: https://phoboslab.org/wipegame/

More info in my blog: https://phoboslab.org/log/2023/08/rewriting-wipeout

⚠️ Work in progress. Expect bugs.

## Building

See [Building.md](Building.md) for more info.

### Flags

The makefile accepts several flags. You can specify them with `make FLAG=VALUE`

- `DEBUG` – `true` or `fals`, default is `false`. Whether to include debug symbols in the build.
- `RENDERER` – `GL` or `SOFTWARE`, default is `GL` (the `SOFTWARE` renderer is very much unfinished and only works with SDL)
- `USE_GLX` – `true` or `false`, default is `false` and uses `GLVND` over `GLX`. Only used for the linux build.


## Running

This repository does not contain the assets (textures, 3d models etc.) required to run the game. This code mostly assumes to have the PSX NTSC data, but some menu models from the PC version are required as well. Both of these can be easily found on archive.org and similar sites. The music (optional) needs to be provided in [QOA format](https://github.com/phoboslab/qoa). The intro video as MPEG1.

The directory structure is assumed to be as follows

```
./wipegame # the executable
./wipeout/textures/
./wipeout/music/track01.qoa
./wipeout/music/track02.qoa
...
```

Note that the blog post announcing this project may or may not provide a link to a ZIP containing all files needed. Who knows!

Optionally, if you want to use a game controller that may not be supported by SDL directly, you can place the [gamecontrollerdb.txt](https://github.com/gabomdq/SDL_GameControllerDB) in the root directory of this project (along the compiled `wipegame`).



## Ideas for improvements

PRs Welcome.

### Not yet implemented

Some things from the original game are not yet implemented in this rewrite. This includes

- screen shake effect
- game-end animations, formerly `Spline.cpp` (the end messages are just shown over the attract mode cameras)
- viewing highscores in options menu
- reverb for sfx and music when there's more than 4 track faces (tunnels and such)
- some more? grep the source for `TODO` and `FIXME`

### Gameplay, Visuals

- less punishing physics for ship vs. ship collisions
- less punishing physics for sideways ship vs. track collisions (i.e. wall grinding like in newer wipEouts)
- somehow resolve the issue of inevitably running into an enemy that you just shot
- add option to lessen the roll in the internal view
- add additional external view that behaves more like in modern racing games
- dynamic lighting on ships
- the scene geometry could use some touch-ups to make an infinite draw distance option less awkward
- increase FOV when going over a boost
- better menu models for game exit and video options
- gamepad analog input feels like balancing an egg
- fix collision issues on junctions (also present in the original)

### Technical

- implement frustum culling for scene geometry, the track and ships. Currently everything within the fadeout radius is drawn.
- put all static geometry into a GPU-side buffer. Currently all triangles are constructed at draw time. Uploading geometry is complicated a bit by the fact that some scene animations and the ship's exhaust need to update geometry for each frame.
- the menu system is... not great. It's better than the 5000 lines of spaghetti that it was before, but the different layouts need a lot of `if`s
- the save data is just dumping the whole struct on disk. A textual format would be preferable.
- since this whole thing is relying on some custom assembled assets anyway, maybe all SFX should be in QOA format too (like the music). Or switch everything to Vorbis.
- a lot of functions assume that there's just one player. This needs to be fixed for a potential splitscreen mode.


## License

There is none. This code may or may not be based on the source code of the PC (ATI-Rage) version that was leaked in 2022. If it were, it would probably violate copyright law, but it may also fall under fair use ¯\\\_(ツ)\_/¯

Working with this source code is probably fine, considering that this game was originally released 28 years ago (in 1995), that the current copyright holders historically didn't care about any wipEout related files or code being available on the net and that the game is currently not purchasable in any shape or form.

In any case, you may NOT use this source code in a commercial release. A commercial release includes hosting it on a website that shows any forms of advertising.

PS.: Hey Sony! If you're reading this, I would love to work on a proper, officially sanctioned remaster. Please get in touch <3
