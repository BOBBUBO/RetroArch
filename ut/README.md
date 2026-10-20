## Building

### With clickable (recommended for installable packages)

[clickable](https://clickable-ut.dev) is the standard tool for building Ubuntu
Touch click packages. It builds inside Docker with the proper UT SDK and
bundles all required libraries so the package installs on any UT device:

```sh
pipx install clickable

# Build for Redmi 9 lancelot (ARM64)
clickable --target arm64

# Build for ARM32 devices
clickable --target armhf

# Install on connected UT device
clickable install
```

Project config is in `clickables.yml` at the repo root. It uses `builder: custom`
and builds via `Makefile.ut`, installing files into clickable's install directory
so clickable can package them into a `.click`. Runtime libraries are declared
in `dependencies_target` (build time) and `install_lib` (bundled into the click).

### Manual cross build (CI only)

Without clickable, cross build on Ubuntu for CI:

```sh
make -f Makefile.ut clean CROSS_COMPILE=aarch64-linux-gnu-
make -f Makefile.ut click CROSS_COMPILE=aarch64-linux-gnu- \
  -j$(getconf _NPROCESSORS_ONLN)

make -f Makefile.ut clean CROSS_COMPILE=arm-linux-gnueabihf-
make -f Makefile.ut click CROSS_COMPILE=arm-linux-gnueabihf- \
  -j$(getconf _NPROCESSORS_ONLN)
```

Packages are written to
`ut/com.libretro.retroarch_<version>_arm64.click` and
`ut/com.libretro.retroarch_<version>_armhf.click`.

CI builds both arches on every push to master via
`.github/workflows/UbuntuTouch.yml`; artifacts are attached to the run.

Note: manually built packages link against Ubuntu 24.04 system libraries and
may be rejected by openstore on UT 24.04 devices due to library mismatch. Use
clickable for packages that install reliably.
