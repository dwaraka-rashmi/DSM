# cs5600_fl16_DSM_project

Detailed Proposal:
===================
https://www.overleaf.com/7195668hdzvcnyjsfmx#/24822192/


Distributed Shared Memory support for xv6
=========================================

The distributed shared memory (DSM) implements the shared
memory model in distributed systems, which have no physical shared
memory. The shared memory model provides a virtual address space shared
between all nodes. This Shared memory model will be implemented on xv6 codebase.
The DSM implementation will involve kernel modification along with a user library support.

-------------------------------------------------------------------------------------------

Contributors:
=============

Abhay Kasturia

Nakul Camasamudram

Rashmi Dwaraka

-------------------------------------------------------------------------------------------


BUILDING AND RUNNING XV6:

=========================

The latest xv6 source is available via
git clone git://pdos.csail.mit.edu/xv6/xv6.git

To build xv6 on an x86 ELF machine (like Linux or FreeBSD), run "make".
On non-x86 or non-ELF machines (like OS X, even on x86), you will
need to install a cross-compiler gcc suite capable of producing x86 ELF
binaries.  See http://pdos.csail.mit.edu/6.828/2011/tools.html.
Then run "make TOOLPREFIX=i386-jos-elf-".

To run xv6, you can use Bochs or QEMU, both PC simulators.
Bochs makes debugging easier, but QEMU is much faster. 
To run in Bochs, run "make bochs" and then type "c" at the bochs prompt.
To run in QEMU, run "make qemu".

-------------------------------------------------------------------------------------------


ACKNOWLEDGMENTS:

================

xv6 is inspired by John Lions's Commentary on UNIX 6th Edition (Peer
to Peer Communications; ISBN: 1-57398-013-7; 1st edition (June 14,
2000)). See also http://pdos.csail.mit.edu/6.828/2007/v6.html, which
provides pointers to on-line resources for v6.
