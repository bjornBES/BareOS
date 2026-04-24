import os
import sys
import subprocess

from config import arch, config, imageType

debug = False
if sys.argv.__len__() > 1:
    debug = bool(sys.argv[1])

project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
sys.path.append(project_root)

buildsPath = os.path.join(project_root, "build")
if not os.path.exists(buildsPath):
    exit(-1)

image = os.path.join(project_root, f"build/{arch}_{config}/image.img")

if arch == "i686":
    qemu_command="qemu-system-i386"
else:
    qemu_command=f"qemu-system-{arch}"

qemu_args = [qemu_command]

qemu_args.extend([
    "-k", "da", "-debugcon", "stdio", "-m", "1g", "-d", "guest_errors,int,mmu", "-D", "debug.txt",
    "-netdev", "user,id=mynet0", "-net", "nic,model=rtl8139,netdev=mynet0"])

if imageType == "floppy":
    qemu_args.append(f"-fda")
    qemu_args.append(f"{image}")
else:
    qemu_args.extend(["-drive", f"file={image},format=raw,id=disk,if=none"])
    qemu_args.extend(["-device", "ahci,id=ahci"])
    qemu_args.extend(["-device", "ide-hd,drive=disk"])

qemu_args.extend(["-device", "sb16"])

print(qemu_args)

subprocess.run(qemu_args, stdout=subprocess.PIPE, text=True)
