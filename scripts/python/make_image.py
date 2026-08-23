import os
from pathlib import Path
import re
import struct
import subprocess
import threading
import zlib

from io import SEEK_CUR, SEEK_SET

import gpt_image
from gpt_image.disk import Disk
from pyfatfs.PyFatFS import PyFat

from utility import find_index, glob_recursive, is_file_name, parse_size
from config import config, arch, image_type, image_fs, image_size
from disk import *
from partitions import make_partitions

project_root_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

def get_stage2_sector(stage2_path: str, partition : disk_partition_spec, image: str, disk : disk_spec, partition_offset: int = 0):
    offset_bytes = partition_offset * disk.sector_size
    print(f"opening file {image} with offset {offset_bytes} ({partition_offset} sectors)")
    pf = PyFat(offset=offset_bytes)
    pf.fat_type = partition.filesystem[3:]
    pf.open(image, read_only=True)
    
    print(f"fat type = {pf.fat_type}")

    fat = pf.fat
    bpb = pf.bpb_header
    try:
        mcopy(image, disk, stage2_path, "::/stage2.bin", partition_offset)

        image_arg = f"{image}@@{offset_bytes}" if offset_bytes else image
        # print(f"image_arg = {image_arg}")
        out = subprocess.run(
            ["mdir", "-w", "-i", image_arg, "::/stage2.bin"],
            stdout=subprocess.PIPE,
            text=True,
            check=True
        ).stdout
    except subprocess.CalledProcessError:
        pf.close()
        return 0

    sectors_per_cluster = bpb["BPB_SecPerClus"]
    parts = out.strip().split()
    index = parts.index("stage2.bin") + 3
    # print(parts[index])
    first_cluster = max(int(parts[index]) // disk.sector_size, sectors_per_cluster)
    # print(f"first_cluster = {first_cluster}")

    reserved = bpb.get("BPB_RsvdSecCnt")
    fats = bpb["BPB_NumFATs"]
    fat_size = bpb.get("BPB_FATSz32") if pf.fat_type == 32 else bpb.get("BPB_FATSz16")

    # print(f"sectors_per_cluster = {sectors_per_cluster}")
    # print(f"reserved = {reserved}")
    # print(f"fats = {fats}")
    # print(f"fat_size = {fat_size}")

    first_data_sector = reserved + fats * fat_size
    first_sector = first_data_sector + (first_cluster - 2) * sectors_per_cluster
    print(f"first_data_sector = {first_data_sector}")
    print(f"first_sector = {first_sector}")

    pf.close()
    return first_sector


def find_symbol_in_map_file(map_file: Path, symbol: str):
    with map_file.open('r') as fmap:
        for line in fmap:
            if symbol in line:
                match = re.search('0x([0-9a-fA-F]+)', line)
                if match is not None:
                    return int(match.group(1), base=16)
    return None

def create_filesystem(partition : disk_partition_spec, disk : disk_spec, target: str, filesystem, reserved_sectors=0, offset=0):
    # print(f"target={target} filesystem={filesystem} reserved_sectors={reserved_sectors} offset={offset}")
    if filesystem in ['fat12', 'fat16', 'fat32']:
        reserved_sectors += 1
        if filesystem == 'fat32':
            reserved_sectors = 32
        
        fat_type = filesystem[3:]  # Extract '12', '16', or '32' from 'fat12', 'fat16', 'fat32'
        print(f"mkfs.fat -F {fat_type} -n BARE-OS -R {reserved_sectors} {target} --offset {offset}")
        result = subprocess.run([
            "mkfs.fat",
            "-F", fat_type,
            "-n", "BARE-OS",
            "-s", "2",
            "-R", str(reserved_sectors),
            target,
            "--offset", str(offset),
        ])
        
        if result.returncode != 0:
            raise ValueError(f'Failed to create FAT{fat_type} filesystem')
    elif filesystem in ["ext2", "ext3", "ext4"]:
        reserved_sectors += 1
        
        # print(f"mkfs.{filesystem} -E offset={offset} -L BARE-OS")
        result = subprocess.run([
            f"mkfs.{filesystem}",
            "-E", f"offset={offset * disk.sector_size}",
            "-L", "BARE-OS",
            target,
            f"{partition.partition_size_bytes // 1024}k"
        ])
        
        if result.returncode != 0:
            raise ValueError(f'Failed to create {filesystem} filesystem')
    else:
        raise ValueError('Unsupported filesystem ' + filesystem)

def install_mbr(target: str, disk : disk_spec, mbr: str):
    """Install MBR bootloader at sector 0.
    
    Parameters:
        target: Image file path
        mbr: MBR binary path
    """
    # print(f"installing MBR {mbr}")
    map_file = Path(mbr).with_suffix('.map')
    if not map_file.exists():
        raise ValueError("Can't find map file for " + mbr)
    
    entry_offset = find_symbol_in_map_file(map_file, '__entry_start')
    if entry_offset is None:
        raise ValueError("Can't find __entry_start symbol in map file " + str(map_file))
    entry_offset_in_binary = entry_offset - 0x600
    
    code_end = find_symbol_in_map_file(map_file, '__code_end')
    if code_end is None:
        raise ValueError("Can't find __code_end symbol in map file " + str(map_file))
    code_end_in_binary = 0x1B8
    
    with open(mbr, 'rb') as fmbr:
        with open(target, 'r+b') as ftarget:
            # print(f"entry_offset_in_binary = {entry_offset_in_binary}/{hex(entry_offset_in_binary)}")
            # print(f"code_end_in_binary = {code_end_in_binary}/{hex(code_end_in_binary)}")
            
            # Write boot code
            code_size = code_end_in_binary - entry_offset_in_binary + 3
            # print(f"code_size = {code_size}/{hex(code_size)}")
            fmbr.seek(0, SEEK_SET)
            ftarget.seek(0, SEEK_SET)
            code = fmbr.read(code_size)
            ftarget.write(code)
            
            ftarget.seek(0x1FE, SEEK_SET)
            ftarget.write(b'\x55\xaa')
            
            # print(f"> MBR installed at sector 0")

def install_vbr(target: str, disk : disk_spec, vbr: str, stage2_offset, stage2_size, partition_offset=0):
    """Install VBR bootloader at partition start sector.
    
    Preserves BPB (bytes 0x0B-0x3F) that mkfs.fat creates, overwrites boot code.
    
    Parameters:
        target: Image file path
        vbr: VBR binary path
        stage2_offset: LBA offset where stage2 is located
        stage2_size: Size of stage2 in bytes
        partition_offset: Sector offset of partition start
    """
    # print(f"installing VBR {vbr}")
    # print(f"> stage2 offset {stage2_offset}")
    # print(f"> stage2 size {stage2_size}")
    # print(f"> partition_offset {partition_offset}")
    
    map_file = Path(vbr).with_suffix('.map')
    if not map_file.exists():
        raise ValueError("Can't find map file for " + vbr)
    
    byte_offset = partition_offset * disk.sector_size
    
    entry_offset = find_symbol_in_map_file(map_file, '__entry_start')
    if entry_offset is None:
        raise ValueError("Can't find __entry_start symbol in map file " + str(map_file))
    entry_offset_in_binary = entry_offset - 0x7e00  # Both MBR and VBR linked at 0x7C00
    entry_offset_in_image = entry_offset_in_binary + byte_offset
    
    stage2_location_offset = find_symbol_in_map_file(map_file, 'stage2_location')
    if stage2_location_offset is None:
        raise ValueError("Can't find stage2_location symbol in map file " + str(map_file))
    stage2_location_offset_in_binary = stage2_location_offset - 0x7e00  # Both MBR and VBR linked at 0x7C00
    stage2_location_offset_in_image = stage2_location_offset_in_binary + byte_offset
    
    code_end = find_symbol_in_map_file(map_file, '__code_end')
    if code_end is None:
        raise ValueError("Can't find __code_end symbol in map file " + str(map_file))
    code_end_in_binary = code_end - 0x7e00  # Both MBR and VBR linked at 0x7C00
    
    with open(vbr, 'rb') as fvbr:
        with open(target, 'r+b') as ftarget:
            # print(f"entry_offset_in_binary = {entry_offset_in_binary}/{hex(entry_offset_in_binary)}")
            # print(f"stage2_location_offset_in_binary = {stage2_location_offset_in_binary}/{hex(stage2_location_offset_in_binary)}")
            # print(f"code_end_in_binary = {code_end_in_binary}/{hex(code_end_in_binary)}")
            
            # Write first 3 bytes jump instruction at partition start
            # This overwrites the mkfs.fat jump
            ftarget.seek(byte_offset, SEEK_SET)
            jump_bytes = fvbr.read(3)
            # print(f"Writing jump bytes: {jump_bytes.hex()} to offset {byte_offset} ({hex(byte_offset)})")
            ftarget.write(jump_bytes)
            
            # Skip mkfs.fat's BPB (bytes 0x0B-0x3F, 53 bytes)
            # Write only the boot code starting at offset 0x40
            # entry_offset should be >= 0x40 to avoid BPB
            code_start_offset = 0x40
            if entry_offset_in_binary < code_start_offset:
                raise ValueError(f"Entry point at 0x{entry_offset_in_binary:x} is before 0x40 (in BPB area)")
            
            code_size = code_end_in_binary - entry_offset_in_binary + 3
            # print(f"code_size = {code_size}/{hex(code_size)}")
            # print(f"Reading from vbr at offset {entry_offset_in_binary - 3}")
            # print(f"Writing to image at offset {entry_offset_in_image - 3} ({hex(entry_offset_in_image - 3)})")
            fvbr.seek(entry_offset_in_binary - 3, SEEK_SET)
            ftarget.seek(entry_offset_in_image - 3, SEEK_SET)
            code = fvbr.read(code_size)
            # print(f"Read {len(code)} bytes of code, first 16 bytes: {code[:16].hex()}")
            ftarget.write(code)
            
            # Read SectorsPerCluster from BPB (at byte_offset + 13)
            ftarget.seek(byte_offset + 13, SEEK_SET)
            SectorsPerCluster = int.from_bytes(ftarget.read(1))

            # Write stage2_location: [4 bytes LBA][1 byte sector count]
            realStage2_offset = stage2_offset + (SectorsPerCluster - 1)
            stage2_sectors = (stage2_size + disk.sector_size - 1) // disk.sector_size
            # print(f"realStage2_offset = {realStage2_offset}")
            # print(f"stage2_sectors = {stage2_sectors}")
            ftarget.seek(stage2_location_offset_in_image, SEEK_SET)
            ftarget.write(realStage2_offset.to_bytes(4, byteorder='little'))
            ftarget.write(stage2_sectors.to_bytes(1, byteorder='little'))
            
            # Flush and sync to disk
            ftarget.flush()
            os.fsync(ftarget.fileno())
            
            # print(f"> VBR installed at sector {partition_offset}")

def mmd(image, disk : disk_spec, file_dst, offset_sectors = 0):
    offset_bytes = offset_sectors * disk.sector_size
    image_arg = f"{image}@@{offset_bytes}" if offset_bytes else image
    # print(f"mdd(image={image}, file_dst={file_dst}, offset_sectors={offset_sectors})")
    subprocess.run(["mmd", 
                    "-i", image_arg,
                    file_dst], check=True)

def mcopy(image, disk : disk_spec, file_src, file_dst, offset_sectors = 0):
    offset_bytes = offset_sectors * disk.sector_size
    image_arg = f"{image}@@{offset_bytes}" if offset_bytes else image
    print(f"mcopy(image={image}, file_src={file_src}, file_dst={file_dst}, offset_sectors={offset_sectors})")
    subprocess.run(["mcopy", 
                    "-i", image_arg,
                    file_src,
                    file_dst], check=True)

def loadFiles(image, disk : disk_spec, files, baseDir, imageDir = "::/", partition_offset = 0):

    # print(f"loadFiles({image}, {files}, {baseDir}, {imageDir}, {partition_offset})")
    # copy rest of files
    src_root = baseDir

    # print(f"> copying dirs...")
    for file in files:
        file_src = file
        file_rel = os.path.relpath(file_src, src_root)
        file_dst = os.path.join(imageDir, file_rel)
        if os.path.isdir(file_src):
            # print(f"entry dir {file_dst}")
            mmd(image, disk, file_dst, partition_offset)
            
    # print(f"> copying files...")
    for file in files:
        # print(f'have file {file}')
        file_src = file
        file_rel = os.path.relpath(file_src, src_root)
        file_dst = os.path.join(imageDir, file_rel)

        if not os.path.isdir(file_src):
            # print('    ... copying', file_rel)
            mcopy(image, disk, file_src, file_dst, partition_offset)
            
MAX_WORKERS = 8

def generate_image_file(target, disk : disk_spec, size):
    # print(f"got target as {target}, size as {size}")
    with open(target, 'wb') as fout:
        fout.write(bytes(size * disk.sector_size))
        fout.close()

def generate_image_files(targets, disk : disk_spec):
    filesPath = os.path.join(project_root_path, "files")
    if not os.path.exists(filesPath):
        os.mkdir(filesPath)
    threads = []
    
    for targetTuple in targets:
        target, size = targetTuple
        t = threading.Thread(target=generate_image_file, args=(target, disk, size))
        threads.append(t)
    for t in threads:
        t.start()
    for t in threads:
        t.join()
                
def calculate_files_partition_size(files, disk : disk_spec):
    total_bytes = sum(os.path.getsize(f) for f in files)
    sectors_needed = (total_bytes + disk.sector_size - 1) // disk.sector_size
    # add some extra for FAT metadata
    return sectors_needed + 128  # +128 sectors for FAT overhead


def build_disk(_disk : Disk, disk : disk_spec, mbr : str = "", imageFileSystem : str = image_fs, size_sectors : int = 0):
    if (size_sectors == 0):
        size_sectors = (disk.disk_size + disk.sector_size - 1) // disk.sector_size
        disk.disk_size_sectors = size_sectors
    
    _disk.create(size_sectors * disk.sector_size)
    _disk.table.update()
    
    # Do not create filesystem at root when using partitions - each partition will create its own
    # Only create root filesystem if no partitions are defined
    # print(f"try to format the disk")
    if not disk.partitions:
        # print(f"formatting the disk with {_disk.image_path}, {imageFileSystem}")
        partition : disk_partition_spec = disk_partition_spec("", "", 0, image_fs, disk.disk_size, True, False)
        partition.partition_size_sectors = partition.partition_size_bytes // disk.sector_size
        create_filesystem(partition, disk, disk.image_path, imageFileSystem, offset=0)
    
    
def build_partition(image, _disk : Disk, disk : disk_spec, files, bin_files : list[str], partition : disk_partition_spec, index : int):
    imageFileSystem : str = partition.filesystem

    size_sectors = partition.partition_size_sectors
    file_system = imageFileSystem
    partition_offset = partition.partition_offset
    
    # create partition table
    # print(f"> creating partition table...")
    make_partitions(_disk, disk, partition_offset, size_sectors, file_system, partition, index)

    # Create filesystem
    # print(f"> formatting file using {file_system}...")
    create_filesystem(partition, disk, image, file_system, offset=partition_offset)
    
    partition.partition_size_sectors = size_sectors
    stage2_offset = 0
    
    # For bootable partitions: copy stage2, get offset, and install VBR if provided
    if partition.bootable and partition.stage2 != "":
        # print(f"stage2 = {stage2}")
        
        # Now copy stage2 and get its sector offset
        # print(f"> copying stage2...")
        first_data_sector = get_stage2_sector(partition.stage2, partition, image, disk, partition_offset)
        stage2_size = Path(partition.stage2).stat().st_size
        stage2_sectors = (stage2_size + disk.sector_size - 1) // disk.sector_size
        stage2_offset = first_data_sector + 1 + partition_offset
        print(f"> stage2 offset {stage2_offset}")
        
        # Install VBR if provided BEFORE loading files (so it doesn't get overwritten)
        if partition.vbr_file != "" and os.path.exists(partition.vbr_file):
            # print(f"> installing VBR...")
            install_vbr(image, disk, partition.vbr_file, stage2_offset, stage2_size, partition_offset)
    
    try:
        if partition.kernel != "":
            print(f"> copying kernel {partition.kernel}...")
            mmd(image, disk, "boot", partition_offset)
            mcopy(image, disk, partition.kernel, "::/boot/kernel.elf", partition_offset)
            
        # load bin files
        loadFiles(image, disk, files, partition.root_dir, "::/", partition_offset)
        
        if bin_files.__len__() > 0:
            user_path = os.path.join(project_root_path, f"build/{arch}_{config}", "user")
            for file in bin_files:
                file_src = os.path.join(user_path, file)
                file_name = os.path.basename(file)
                file_dst = os.path.join("::/bin", file_name)
                # print(f'    ... copying {file_src} to {file_dst}')
                if not os.path.isdir(file_src):
                    mcopy(image, disk, file_src, file_dst, partition_offset)
                
    finally:
        print("> cleaning up...")
    
    return stage2_offset


def build_disk_from_disk(disk : disk_spec, build_dir : str, root_fs_dir : str):
    disk_mbr_file = os.path.join(build_dir, disk.mbr_file) if disk.mbr_file != "" else ""
    image = os.path.join(build_dir, disk.image_path)
    if (os.path.exists(image)):
        os.remove(image)
    
    _disk : Disk = Disk(image, disk.sector_size)
    build_disk(_disk, disk, disk_mbr_file, disk.filesystem, disk.disk_size_sectors)
    first_sector_offset = 0
    index = 0
    last_partition = None
    s2_offset = 0
    stage2_size = 0
    total_size_sectors = 0

    
    for partition in disk.partitions:
        
        partition.root_dir = os.path.join(root_fs_dir, partition.root_dir)
        files = glob_recursive('*', partition.root_dir)

        partition.stage2 = os.path.join(build_dir, partition.stage2) if partition.bootable and partition.stage2 != "" else ""
        partition.kernel = os.path.join(build_dir, partition.kernel) if partition.bootable and partition.kernel != "" else ""
        partition.vbr_file = os.path.join(build_dir, partition.vbr_file) if partition.bootable and partition.vbr_file != "" else ""
        
        if partition.partition_size_bytes == 0 and partition.partition_size_sectors == 0:
            # print(f"total_size_sectors =    {hex(total_size_sectors)}")
            # print(f"disk.disk_size =        {hex(disk.disk_size_sectors)}")
            # print(f"maybe =                 {hex(disk.disk_size_sectors - total_size_sectors + first_sector_offset)}")
            # if last_partition != None:
            #     print(f"last.sector_size =      {hex(last_partition.partition_size_sectors)}")
            #     print(f"last.offset =           {hex(last_partition.partition_offset)}")
            partition.partition_size_sectors = disk.disk_size_sectors - total_size_sectors + first_sector_offset
            partition.partition_size_bytes = partition.partition_size_sectors * disk.sector_size
            
        elif partition.partition_size_bytes == 0:
            partition.partition_size_sectors = partition.partition_size_sectors - total_size_sectors
            partition.partition_size_bytes = partition.partition_size_sectors * disk.sector_size
            
        elif partition.partition_size_sectors == 0:
            partition.partition_size_sectors = ((partition.partition_size_bytes + disk.sector_size - 1) // disk.sector_size)
            
        else:
            partition.partition_size_sectors = ((partition.partition_size_bytes + disk.sector_size - 1) // disk.sector_size)
            
        if last_partition != None:
            partition.partition_offset = last_partition.partition_size_sectors + last_partition.partition_offset
        else:
            first_sector_offset = partition.partition_offset
            total_size_sectors = partition.partition_offset

        print(f"partition {index} = {{")
        print(f"\t partition name:\"{partition.name}\"")
        print(f"\t partition rootfs dir:\"{partition.root_dir}\"")
        print(f"\t partition offset:{hex(partition.partition_offset)}")
        print(f"\t partition filesystem:\"{partition.filesystem}\"")
        print(f"\t partition size in bytes:{partition.partition_size_bytes}")
        print(f"\t partition size in sectors:{hex(partition.partition_size_sectors)}")
        print(f"\t partition is bootable:\"{partition.bootable}\"")
        print(f"\t partition in gpt:\"{partition.in_gpt}\"")
        print(f"\t partition in vbr file:\"{partition.vbr_file}\"")
        print(f"\t partition in stage2 file:\"{partition.stage2}\"")
        print(f"\t partition in kernel file:\"{partition.kernel}\"")
        print(f"\t partition calculated size in sectors from files:{calculate_files_partition_size(files, disk)}")
        print(f"}}")
        print(f"total size in sectors right now: {hex(total_size_sectors)}")
        
        total_size_sectors += partition.partition_size_sectors

        if last_partition != None:
            last_partition_size_sector = last_partition.partition_size_sectors + last_partition.partition_offset
            if last_partition_size_sector == 0:
                print(f"expected sectors {calculate_files_partition_size(files, disk)}")
                return

        s2_offset2 = build_partition(image, _disk, disk, files, partition.bin_files, partition, index)
        if (s2_offset2 != 0): s2_offset = s2_offset2
        
        if s2_offset != 0 and partition.stage2 != "":
            stage2_size = Path(partition.stage2).stat().st_size
        
        last_partition = partition
        index += 1
    
    # Install MBR to sector 0 AFTER all partitions are built
    # print(f"s2_offset={s2_offset} stage2_size={stage2_size}")
    if s2_offset != 0 and stage2_size > 0 and disk_mbr_file != "":
        stage2_sectors = (stage2_size + disk.sector_size - 1) // disk.sector_size
        # print(f"> installing MBR...")
        install_mbr(image, disk, disk_mbr_file)
        
    if disk.use_mbr:
        header = gpt_image.table.Header(_disk.geometry)
        header.number_of_partition_entries = len(disk.partitions)
        
        # first marshal with crc=0
        header_bytes = bytearray(header.marshal())
        
        # print(f"_disk.sector_size = {_disk.sector_size}")
        # print(f"num entries in table: {len(_disk.table.partitions.entries)}")
        part_array = b"".join(p.partition_data for p in disk.partitions)
        
        # compute partition array CRC first
        part_array = part_array.ljust(128 * len(disk.partitions), b"\x00")
        # print(part_array)
    
        with open(_disk.image_path, "r+b") as f:
            f.seek(2 * _disk.sector_size)
            f.write(part_array)
    
        part_crc = zlib.crc32(part_array) & 0xFFFFFFFF
        
        
        struct.pack_into("<I", header_bytes, 88, part_crc)
        struct.pack_into("<I", header_bytes, 16, 0)
        header_crc = zlib.crc32(bytes(header_bytes[:92])) & 0xFFFFFFFF
        struct.pack_into("<I", header_bytes, 16, header_crc)
        
        print(f"{header_bytes}")
        with open(_disk.image_path, "r+b") as f:
            f.seek(1 * _disk.sector_size)  # LBA1
            f.write(header_bytes)
    # print(f"> Done")


root_fs_path = os.path.join(project_root_path, "rootfs")

if (not os.path.exists(root_fs_path)):
    os.mkdir(root_fs_path)
    
build_path = os.path.join(project_root_path, "build")
if (not os.path.exists(build_path)):
    exit(1)
    
arch_build_path = os.path.join(build_path, f"{arch}_{config}")

stage1_mbr = os.path.join(arch_build_path, "stage1/mbr.bin")
stage1_vbr = os.path.join(arch_build_path, "stage1/vbr.bin")
stage2_bin = os.path.join(arch_build_path, "stage2/stage2.bin")
# kernel_bin = os.path.join(arch_build_path, "kernel/kernel.elf")

disks.append(disk_spec("main", os.path.join(arch_build_path, "image.iso"), parse_size(image_size), 512, image_fs, False, [
    disk_partition_spec("user", "user", 2048, "fat32", parse_size("120m"), False, False, []), ## 120 mb fat32 partition
    disk_partition_spec("boot", "root", 0, "fat32", parse_size("10m"), True, False, [], stage1_vbr, stage2_bin, ""), ## 10 mb fat32 partition
    disk_partition_spec("test", "test", 0, "ext2", 0, False, False, []), ## 120 mb ext2 partition
    ], stage1_mbr))

for disk in disks:
    build_disk_from_disk(disk, arch_build_path, root_fs_path)