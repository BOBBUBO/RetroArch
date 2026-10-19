## Building with clickable (recommended)

[clickable](https://github.com/donald-666/clickable) is the standard tool
for building Ubuntu Touch click packages. It builds inside a Docker container
with the proper UT SDK, bundling all libraries so the package installs on any
UT device:

```sh
clickable --target arm64      # Redmi 9 lancelot (ARM64)
clickable --target armhf      # ARM32 devices
```

Install on device:

```sh
clickable install
```

## Manual cross build (CI)

Ubuntu Touch packages are cross built on Ubuntu without clickable:

```sh
make -f Makefile.ut clean CROSS_COMPILE=aarch64-linux-gnu-
make -f Makefile.ut click CROSS_COMPILE=aarch64-linux-gnu- \
  -j$(getconf _NPROCESSORS_ONLN)

make -f Makefile.ut clean CROSS_COMPILE=arm-linux-gnueabihf-
make -f Makefile.ut click CROSS_COMPILE=arm-linux-gnueabihf- \
  -j$(getconf _NPROCESSORS_ONLN)
```

The packages are written to
`ut/com.libretro.retroarch_<version>_arm64.click` and
`ut/com.libretro.retroarch_<version>_armhf.click`.

Note: manually built packages may be rejected by openstore if they depend on
libraries not available on the target UT device. Use clickable for packages
that install reliably.
