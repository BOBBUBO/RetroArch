## Building

Ubuntu Touch packages (`.click`) are cross built on Ubuntu:

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

## Testing

Install on an Ubuntu Touch device with clickable:

```sh
clickable install ut/com.libretro.retroarch_<version>_arm64.click
```
