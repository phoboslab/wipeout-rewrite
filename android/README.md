# Android build

This project uses the SDL2 Android Java layer plus a native shared library built
with the NDK.

## Prereqs
- Android Studio with SDK Platform **36** installed.
- NDK **r29+** installed.
- CMake **3.22+** (Android Studio can install this).

If your local NDK revision isn't `29.0.0`, pass `-PndkVersion=29.x.y` to Gradle
or edit `android/app/build.gradle`.

## SDL2 source
SDL2 is fetched by CMake on first build (tag `release-2.32.10`) into
`third_party/SDL`. If you want to use a local checkout instead, populate that
folder ahead of time.

## Assets
Place the game data under `android/app/src/main/assets/wipeout/` so paths like
`wipeout/textures/` and `wipeout/music/` resolve inside the APK.

```
android/app/src/main/assets/wipeout/
  textures/
  sound/
  music/
  intro.mpeg
```

## Build
Open the `android/` folder in Android Studio and run the app. If you prefer a
CLI build, generate a Gradle wrapper in `android/` or use a local Gradle
install to run `assembleDebug`.
