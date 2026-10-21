; V53A startup for block $1F :)
; (c) SheatNoisette - MIT Licensed
.186

DGROUP group CONST, CONST2, _DATA, _BSS, STACK

VECTORS segment para public 'VECTORS'
        db 400h dup(0)
VECTORS ends

_TEXT segment word public 'CODE'
        assume cs:_TEXT, ds:DGROUP, ss:DGROUP
        extrn main_ : near

        public _cstart_
_cstart_:
        cli

        ; Every bank already maps block $1F in MAME but buriki sets bank 0 anyway
        mov dx, 200h
        mov ax, 3Fh
        out dx, ax

        ; Small model wants ds and ss on DGROUP, bss is zero since the linker
        ; fills up to the reset jump stuff
        mov ax, DGROUP
        mov ds, ax
        mov es, ax
        mov ss, ax
        mov sp, offset DGROUP:stack_top

        call main_

hang:
        hlt
        jmp hang
_TEXT ends

CONST segment word public 'DATA'
CONST ends

CONST2 segment word public 'DATA'
CONST2 ends

_DATA segment word public 'DATA'
_DATA ends

_BSS segment word public 'BSS'
_BSS ends

STACK segment para stack 'STACK'
        db 400h dup(?)
stack_top label word
STACK ends

; The V53A starts at FFFF:0000, block offset FFF0 while every bank maps $1F from MAME
RESET segment para public 'RESET'
        jmp far ptr _cstart_
RESET ends

        end
