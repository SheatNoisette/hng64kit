# Hyper Neo Geo 64 triangle rasterizer

> @TODO Continue to research

## Theorical Performance

In attract mode:
| Game     | Polygons (median / max) | Objects (median / max) | 3D packets (median / max) | Uploads (mean / max) | 3D rate | Triangles/s (median / max) | Sprites (mean / max) |
|----------|-------------------------|------------------------|---------------------------|----------------------|---------|----------------------------|----------------------|
| roadedge | 2060 / 3086             | 84 / 158               | 71 / 129                  | 4.9 / 9              | 60 Hz   | 123.6k / 185.2k            | 80 / 212             |
| sams64   | 1103 / 4449             | 41 / 67                | 41 / 73                   | 24.2 / 29            | 30 Hz   | 33.1k / 133.5k             | 17 / 81              |
| xrally   | 2092 / 3371             | 111 / 163              | 96 / 209                  | 4.5 / 14             | 60 Hz   | 125.5k / 202.3k            | 45 / 101             |
| bbust2   | 1222 / 3207             | 41 / 139               | 12 / 143                  | 2.7 / 9              | 60 Hz   | 73.3k / 192.4k             | 11 / 16              |
| fatfurwa | 1798 / 2325             | 17 / 78                | 36 / 93                   | 3.2 / 6              | 60 Hz   | 107.9k / 139.5k            | 31 / 55              |
| buriki   | 2342 / 2424             | 66 / 74                | 72 / 80                   | 4.4 / 5              | 60 Hz   | 140.5k / 145.4k            | 167 / 314            |

Polygons are the triangles in the objects sent, before culling

| Figure               | Math                           | Value                                   |
|----------------------|--------------------------------|-----------------------------------------|
| Triangles/s          | polygons x 3D rate             | xrally median: 2092 x 60 = 125.5k       |
| Pixels per frame     | 512 x 448                      | 229,376                                 |
| Fill rate, 1 layer   | 512 x 448 x 3D rate            | 13.76M pixel/s at 60 Hz, 6.88M at 30 Hz |
| Fill rate + overdraw | 512 x 448 x 3D rate x overdraw | overdraw 2: 27.5M, overdraw 3: 41.3M    |

Please note that triangles/s assumes that the polygon counts are per 3D frame.
Overdraw is not measured, so fill rate is a floor for a full-screen scene.
Only tested in emulator unfortunately.

So, in theory:
- Peak triangles/s: 202.3k (xrally, 3371 x 60)
- Fill rate: 13.76M pixel/s per full-screen layer at 60 Hz, 27.5M to 41.3M with overdraw 2 to 3
