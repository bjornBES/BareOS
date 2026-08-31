# ABI-contracts

BareOS’s ABI contracts go from the MBR all the way up to the Kernel and even user space.

## MBR/VBR jump table

The first of these contracts is in the MBR/VBR (see boot/bootloader.md for more) where the VBR uses a jump table at 0x7C00 filled out by the MBR, the jump table contains addresses to functions in the MBR so that code isn’t repeated and the size of the VBR is cut.

## Stage2/Kernel GDT

Another contract is between the bootloader stage2 and the kernel on x86 systems where the first 2 usable entries in the GDT (selectors 0x08/0x10) will always be 32 bit Code/Data segments on 32 bit protected mode systems and 64 bit Code/Data segments on systems where long mode (Intel64) can be enabled, that means the kernel can trust the GDT until the kernel installs its own GDT.
