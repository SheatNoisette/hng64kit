# Notes on the libsnk

> High quality infodump for now

What I refer about "libsnk" is the library shipped by SNK for working with the
Hyper Neo Geo.

While reversing, I found interesting piece of information scattered about games
and instead of this getting lost, here they are.

Most of the five of the six dumped game that I found have some part which
nearly match from each other:
- crt0
- interrupt handling
- exception system
- rtc
- IOMCU
- A cooperative task system
- 3d GPU functions
- C library
- Sound firmware interaction (with the firmware of course)

There's a high chance that the games were compiled with the same compiler,
maybe a GCC 2.X from 1997.

Please note that this is speculation and pure guesswork. Most of the function
are pure fabrication.

## Traces

Some interesting RCS strings:

| Game       | In `.data`?                                                                              | RCS `$Id$` strings ?                                                                                                                                           |
|------------|------------------------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `roadedge` | `(C)SNK Thu Aug 21 11:36:13 1997`                                                        | none                                                                                                                                                           |
| `xrally`   | `(C)SNK Fri Apr 10 10:23:39 1998 AKIYAMA (build 1176)`                                   | `ge_sys.c` 1.2 1997/11/14, `ge_gp.c` 1.1 1997/10/07 11:43:06, `ge_gpint.c` 1.1 1997/10/07 11:44:06, `sintable.c` 1.1 1997/10/07 11:45:48                       |
| `bbust2`   | `(C)SNK Tue Jul 28 02:01:33 1998`, and `(C)ADK Fri Jun 12 16:22:40 1998` at `0x800A41B0` | none                                                                                                                                                           |
| `fatfurwa` | `BY JKR Tue Jan 05 18:32:21 1999`, and a banner "1999/01/05 Program by JKR"              | none                                                                                                                                                           |
| `buriki`   | `(C)SNK Fri Apr 23 05:16:53 1999 NISHIYAMA (build 1439)`                                 | `main.c` 1.2 1998/08/19, `gd_task.c` 1.4 1998/10/16, `fix.c` 1.4 1998/08/24, `snk_int.c` 1.2 1998/08/19, `etc.c` 1.1 1998/08/18 11:41:07, `ge_gp.c` 1.1 1998/0 |

## Library

### Random

At `0x8000BD70`:
```c
u8 rnd(void)
{
    // Cycle counter
    rnd_count = cp0_count();
    rnd_state = rnd_table[(rnd_state + rnd_count) & 0xFF] ^ rnd_state;
    return rnd_state;
}
```
