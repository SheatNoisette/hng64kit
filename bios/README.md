# hng64kit-flan

A yummy BIOS for HNG64.

**NOT TESTED ON HARDWARE**
**DOES NOT BOOT ORIGINAL GAMES YET**

An open source BIOS for the Hyper Neo Geo 64 for research, hopefully may allow
to test out hardware features without a flash cart.

## Todo

- [x] Setup FCSR
- [x] DMA Seg
- [ ] CPU Bookeeping
- [x] Fast boot mode
- [ ] Magic check
- [ ] System init (K0, TLB, 3D buffer)
- [ ] RTC init
- [ ] I/O MCU init and handshake
- [ ] Network CPU handshake
- [ ] Normal boot DMA and jump
- [ ] Bios stage copy (if games use it)
- [ ] Title and quiet mode
- [ ] POST
- [ ] Crosshatch (May not be done)
- [ ] No cart screen
- [ ] Error log and Log Reader
- [ ] Reflash from cart

## Build

Build the toolkit then:
```sh
docker run --rm --user "$(id -u):$(id -g)" -v "$PWD":/kit -w /kit/bios hng64kit:latest -c make
```

Done

## Run in MAME

Copy it as `brom1.bin` (lowercase) into a `hng64` folder on the rompath and so pass `-bios japan`.

Like that:
```sh
hng64 fatfurwa -bios japan -rompath "<dir with hng64/brom1.bin>;<dir with fatfurwa/>"
```

## License

MIT
