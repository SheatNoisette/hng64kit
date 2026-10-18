# Hyper Neo Geo 64 BIOS boot flow

> @TODO: Clean up, schematics

- Reset at `0xBFC00000` (Boot stage)
  - Soft reset with bit 0 of `0x1F701114` set skips the cold init
  - Cold init: sysregs, cache init, Status and Conf, `sp = 0x80004000`
  - Cart `0x60` is not `"HYPER NEOGEO64"`: jmp to `0xBFC066A4` (no cart path)
  - Cart flags bit 15 set: fast boot
    - DMA the game unless flags bit 13 is set
    - Jump to cart `0x5C`
  - Reflash the System or Bios sector from the cart if the header asks
  - Jump to the System stage at `0xBFC06008`
- System stage (`0xBFC06008`)
  - RTC register D bit 4 is 0: Log Reader ? never returns
  - RTC init
  - System init: cache on, TLB entries, 3D buffer control bytes (?)
  - I/O MCU init cmd
  - POST list 1: scroll RAM, palette RAM
  - Text video init and title screen (unless is quiet)
  - POST list 2: sprite RAM, I/O dual port RAM, network dual-port RAM, 3D banks, sound RAM, main RAM
    - Main RAM failure hangs the hw
  - Network CPU handshake (failure is not fatal thankfully)
  - I/O MCU handshake (failure hangs)
  - Crosshatch ? (unless quiet mode)
  - Unless flags bit 13 is set:
    - DMA the game
    - DMA the Bios stage to RAM `0x00004000`
  - Jump to cart `0x5C`, bye!
- No cart (`0xBFC066A4`)
  - RTC init, system init, POST lists 1 and 2 in quiet mode
  - Blue background if no errors, red if there were errors
  - Hang, again
