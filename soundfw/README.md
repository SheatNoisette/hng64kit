# hngkit64-soundfw

A basic V53A sound CPU firmware, built with Open Watcom 1.9.

It does nothing. Like really nothing:
Disables interrupts, maps bank 0 to block `$1F`, sets up the segments and the
stack, and then spins in `main`.


## Build

```sh
docker build -t hng64kit-watcom:latest .
```

```sh
docker run --rm --platform linux/amd64 \
  --user "$(id -u):$(id -g)" -v "$PWD":/kit -w /kit/soundfw \
  hng64kit-watcom:latest -c make
```

You should have `build/soundfw.bin` hopefully

## Into a cart

Do:
```sh
hng64kit-mksound --firmware soundfw/build/soundfw.bin --out sound.bin
hng64kit-mkrom --program-rom empty.prog.bin --sound sound.bin --out roms
```
