## Building

### With clickable (recommended)

[clickable](https://github.com/donald-666/clickable) is the standard tool for
building Ubuntu Touch click packages. It builds inside Docker with the proper
UT SDK, bundling all libraries so the package installs on any UT device.

```sh
pipx install clickable

# Build for Redmi 9 lancelot (ARM64)
clickable --target arm64

# Build for ARM32 devices
clickable --target armhf

# Install on connected UT device
clickable install
```

Config is in `clickables.yml` at the repo root. Note: clickable builds inside
its own Docker SDK images. If your Makefile.ut cross-toolchain paths differ
from clickable's image, pass a custom build command:

```sh
clickable --build-command "make -f Makefile.ut click CROSS_COMPILE=aarch64-linux-gnu-"
```

### Manual cross build (CI)

Without clickable, cross build on Ubuntu:

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
may be rejected by openstore on UT 24.04 devices due to library mismatch.
Use clickable for packages that install reliably.
