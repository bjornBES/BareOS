import os
import sys
import subprocess
import shutil

from config import arch, config, imageType

debug = False
if sys.argv.__len__() > 1:
    debug = True


project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
sys.path.append(project_root)

buildsPath = os.path.join(project_root, "build")
if not os.path.exists(buildsPath):
    exit(-1)

sys_root = os.path.join(project_root, f"rootfs/user")
sys_root_bin = os.path.join(sys_root, f"bin")
image = os.path.join(project_root, f"build/{arch}_{config}/image.iso")
kernel_elf = os.path.join(project_root, f"build/{arch}_{config}/kernel/kernel.elf")
stage2_elf = os.path.join(project_root, f"build/{arch}_{config}/stage2/stage2.elf")
init_elf = os.path.join(project_root, f"build/{arch}_{config}/user/init.elf")
libcore = os.path.join(project_root, f"build/{arch}_{config}/libcore.a")
libc = os.path.join(project_root, f"build/{arch}_{config}/user/libc.a")
ash = os.path.join(sys_root_bin, f"ash")

debug_bash_script = os.path.join(project_root, f"scripts/debug.sh")
run_bash_script = os.path.join(project_root, f"scripts/run.sh")
gdb_script = os.path.join(project_root, f".vscode/.gdb_script.gdb")

if arch == "i686":
    qemu_command="qemu-system-i386"
else:
    qemu_command=f"qemu-system-{arch}"

qemu_args = [qemu_command]
active_cmd = ""

if debug:
    active_cmd = debug_bash_script
    qemu_args.append("-S")
    qemu_args.extend(["-gdb", "tcp::1234"])

    
    gdb_args : list[str]

    gdb_args = ["gdb"]
    if os.path.exists(gdb_script):
        os.remove(gdb_script)
    with open(gdb_script, "x") as gdb_file:
        gdb_file.write( "target remote :1234\n" +
                       f"symbol-file {kernel_elf}\n" +
                       f"add-symbol-file {stage2_elf} 0x8000\n" +
                       f"add-symbol-file {init_elf} 0x1000\n" +
                       f"add-symbol-file {ash} 0x400000\n" +
                        "set disassembly-flavor intel\n" +
                        "b kernel_main\n"+
                        "b panic\n")
        gdb_file.close()
    gdb_args.extend([kernel_elf, "-x", gdb_script])
    # print(gdb_args)
    
else:
    active_cmd = run_bash_script

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

bash_args = ["bash", active_cmd, kernel_elf]
bash_args.extend(qemu_args)
# print(bash_args)

subprocess.run(bash_args)
