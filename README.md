# hng64kit

Very early homebrew SDK for the Hyper Neo Geo 64 platform.

**Only tested on MAME emulator**

Content:
- docs/ : Documentation and reverse engineering notes
- bios/ : A custom BIOS written from scratch
- src/ : Core SDK libs
- examples/ : SDK examples

## Documentation

See [here](./docs/index.md) for a index.

Getting started is [here](./docs/sdk/getting-started.md)

## Dockerfile

Toolchain:
```
docker build -t hng64kit:latest .
```

## License

Code is under MIT License, documentation is under CC BY-SA 4.0.

Use [funscii](https://github.com/jdebp/funscii) by jdebp, CC0 1.0.
