# System calls

BareOS as an Operating System is not just “another Linux flavored Operating System”, BareOS at its core is a POSIX compliant system, with some features taken from other Unix like Operating Systems and Kernels like Linux, BSD and others. That means the system call functions will follow POSIX specs in the range 0-255, other system calls that aren’t POSIX are at the 256-511 range, another noteworthy range is the BWS NT system calls that has the range 512-1023.

Not all Linux system calls will be in BareOS, some will, the Linux system calls that will be in BareOS will not have anything to do with the Linux Kernels underlying subsystems like TLS. And so to test if a Linux system call will be in BareOS the system call most not touch any internal state and or subsystem within the Linux Kernel (e.g. TLS internals, cgroups and epoll’s internal readiness lists).
