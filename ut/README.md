## Building

### With clickable (recommended for installable packages)

[clickable](https://clickable-ut.dev) is the standard tool for building Ubuntu
Touch click packages. It builds inside Docker with the proper UT SDK and
bundles all required libraries so the package installs on any UT device:

```sh
pipx install clickable-ut

# Build for Redmi 9 lancelot (ARM64)
clickable build --arch arm64

# Build for ARM32 devices
clickable build --arch armhf

# Install on connected UT device
clickable install --arch arm64
```

Project config is in `clickable.json` at the repo root. It uses `builder: custom`
and builds via `Makefile.ut`, installing files into clickable's install directory
so clickable can package them into a `.click`. Runtime libraries are declared
in `dependencies_target` (build time) and `install_lib` (bundled into the click).

### Manual cross build (CI only)

Without clickable, cross build on Ubuntu 24.04 for CI. Install the target
development packages and the `click` packaging tool first:

```sh
sudo apt install click \
  gcc-aarch64-linux-gnu g++-aarch64-linux-gnu \
  gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf \
  libsdl2-dev:arm64 libfreetype-dev:arm64 libfontconfig1-dev:arm64 \
  libudev-dev:arm64 libxkbcommon-dev:arm64 libpulse-dev:arm64 \
  libasound2-dev:arm64 libatomic1:arm64 libpng-dev:arm64 \
  libjpeg-turbo8-dev:arm64 zlib1g-dev:arm64 \
  libegl1-mesa-dev:arm64 libgles2:arm64 \
  libsdl2-dev:armhf libfreetype-dev:armhf libfontconfig1-dev:armhf \
  libudev-dev:armhf libxkbcommon-dev:armhf libpulse-dev:armhf \
  libasound2-dev:armhf libatomic1:armhf libpng-dev:armhf \
  libjpeg-turbo8-dev:armhf zlib1g-dev:armhf \
  libegl1-mesa-dev:armhf libgles2:armhf

make -f Makefile.ut clean CROSS_COMPILE=aarch64-linux-gnu-
make -f Makefile.ut click CROSS_COMPILE=aarch64-linux-gnu- \
  -j$(getconf _NPROCESSORS_ONLN)

make -f Makefile.ut clean CROSS_COMPILE=arm-linux-gnueabihf-
make -f Makefile.ut click CROSS_COMPILE=arm-linux-gnueabihf- \
  -j$(getconf _NPROCESSORS_ONLN)
```

Packages are written to
`ut/com.libretro.retroarch_<version>_arm64.click` and
`ut/com.libretro.retroarch_<version>_armhf.click`. The cross build bundles the
same runtime libraries as clickable and uses `click build`, so the result is a
valid Click 0.4 package.

CI builds both arches on every push to master via
`.github/workflows/UbuntuTouch.yml`; artifacts are attached to the run.

For local use, prefer clickable: it uses the Ubuntu Touch 24.04 framework and
its Docker image already provides the matching toolchain and libraries.

### Platform limits

Ubuntu Touch's compositor implements `wl_shm` only, so it hands no EGL to
Wayland clients and an SDL2 app cannot get a GL context there. The build
therefore defaults to SDL2's own renderer and the launcher asks SDL2 for the
software renderer. The click AppArmor policy denies `/dev/input`, so input
arrives through SDL2 (touch, keyboard) instead of evdev.

The compositor reports no display DPI either, and without one the menu is
scaled for a 1080p desktop panel, which leaves the text too small on a phone
and the touch drag threshold at zero, which leaves the menu without drag
scrolling. The build therefore reports a phone's DPI when the platform has
none. The click package reads the user's own directories (`Documents`,
`Downloads`, `Music`, `Pictures`, `Videos` and the SD card under `/media`) so
the file browser can see game files; everything the app writes stays in its
own configuration directory.

The launcher moves `XDG_CONFIG_HOME` into the package's own directory, which
also moves PulseAudio's client cookie out of the way, so it points
`PULSE_COOKIE` at the cookie where it really is.

Cores that need a hardware rendering context cannot run here at all, so the
launcher names them (in `RETROARCH_NO_HW_CORES`): the core downloader leaves
them out of its list, and a content that names one is refused with a message
instead of taking the app down with it.
