## yocto/bitake configuration
- Paths are highly system dependent
- relative paths can be defined in relation to the `build` directory (initialized with `oe-init-build-env`), though it is more robust to use absolute paths  
## example project structure:

```
.
├── poky
│   ├── bitbake
│   ├── build-everest
│   ├── contrib
│   ├── documentation
│   ├── meta
│   ├── meta-everest
│   ├── meta-openembedded
│   ├── meta-openjdk-temurin
│   ├── meta-poky
│   ├── meta-raspberrypi
│   ├── meta-selftest
│   ├── meta-skeleton
│   ├── meta-yocto-bsp
│   └── scripts
├── shared-downloads
│   ├── ...
│   └── ...
└── shared-sstate
    ├── ...
    └── ...
```

with build-everest containing:
```
.
├── bitbake-cookerdaemon.log
├── cache
├── conf                <-- this directory
└── tmp-glibc
```
