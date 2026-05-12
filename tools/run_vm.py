import os
import sys
import subprocess
import shutil

from config import arch, config, imageType

def run_in_terminal(command: list[str]):
    terminal_cmd = []
    qemu_command = ""
    for arg in command:
        qemu_command += arg + " "

    qemu_command = qemu_command.strip()
    if shutil.which("gnome-terminal"):
        terminal_cmd = ["gnome-terminal", "--", "bash", "-c"] + command
    elif shutil.which("konsole"):
        terminal_cmd = ["konsole", "-e", "bash", "-c", f"\"{qemu_command}\"&"]
    elif shutil.which("xterm"):
        terminal_cmd =["xterm", "-e", "bash", "-c"] + command
    else:
        raise RuntimeError("No supported terminal emulator found")
    
    line_command = ""
    for arg in terminal_cmd:
        line_command += arg + " "

    print(line_command)
    subprocess.Popen(terminal_cmd)

debug = False
if sys.argv.__len__() > 1:
    debug = True


project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
sys.path.append(project_root)

buildsPath = os.path.join(project_root, "build")
if not os.path.exists(buildsPath):
    exit(-1)

image = os.path.join(project_root, f"build/{arch}_{config}/image.iso")
kernel_elf = os.path.join(project_root, f"build/{arch}_{config}/kernel/kernel.elf")
stage2_elf = os.path.join(project_root, f"build/{arch}_{config}/stage2/stage2.elf")
init_elf = os.path.join(project_root, f"build/{arch}_{config}/user/init.elf")
libcore = os.path.join(project_root, f"build/{arch}_{config}/libcore.a")
libc = os.path.join(project_root, f"build/{arch}_{config}/user/libc.a")


gdb_script = os.path.join(project_root, f".vscode/.gdb_script.gdb")

if arch == "i686":
    qemu_command="qemu-system-i386"
else:
    qemu_command=f"qemu-system-{arch}"

qemu_args = [qemu_command]

if debug:
    qemu_args.append("-S")
    qemu_args.extend(["-gdb", "tcp::1234"])

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

if debug:
    # run_in_terminal(qemu_args)
    
    gdb_args : list[str]
    #if shutil.which("gf2-gdb"):
    #    gdb_args = ["gf2-gdb"]
    #else:
    gdb_args = ["gdb"]
    if os.path.exists(gdb_script):
        os.remove(gdb_script)
    with open(gdb_script, "x") as gdb_file:
        gdb_file.write( "target remote :1234\n" +
                       f"symbol-file {kernel_elf}\n" +
                       f"add-symbol-file {stage2_elf} 0x8000\n" +
                        "set disassembly-flavor intel\n" +
                        "b main\n"+
                        "b panic\n")
        gdb_file.close()
    gdb_args.extend([kernel_elf, "-x", gdb_script])
    print(gdb_args)
    
    
    subprocess.run(gdb_args, stdout=subprocess.PIPE, text=True)
    
else:
    subprocess.run(qemu_args, stdout=subprocess.PIPE, text=True)
