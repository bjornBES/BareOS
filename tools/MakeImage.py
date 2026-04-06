import os
import sys
import re
import time
import threading
from decimal import Decimal
from io import SEEK_CUR, SEEK_SET
from pathlib import Path
from shutil import copy2
from getpass import getpass
import subprocess
from pyfatfs.PyFat import PyFat
import pyfatfs
from pyfatfs.EightDotThree import EightDotThree

project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
sys.path.append(project_root)

userApps = os.path.abspath(os.path.join(project_root, "src/user_programs"))

from utility import FindIndex, GlobRecursive, IsFileName, ParseSize
from config import mountMethod, config, arch, imageType, imageFS, imageSize
from disk import DiskSpec, DiskPartitionSpec, disks
from partitons import make_partitons

SECTOR_SIZE = 512


def GetStage2Sector(stage2_path: str, image: str, partition_offset: int = 0):
    offset_bytes = partition_offset * SECTOR_SIZE
    print(f"opening file {image} with offset {offset_bytes} ({partition_offset} sectors)")
    pf = PyFat(offset=offset_bytes)
    pf.open(image, read_only=True)

    fat = pf.fat
    bpb = pf.bpb_header
    try:
        mcopy(image, stage2_path, "::/stage2.bin", partition_offset)

        image_arg = f"{image}@@{offset_bytes}" if offset_bytes else image
        print(f"image_arg = {image_arg}")
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
    print(parts[index])
    first_cluster = max(int(parts[index]) // SECTOR_SIZE, sectors_per_cluster)
    print(f"first_cluster = {first_cluster}")

    reserved = bpb.get("BPB_RsvdSecCnt")
    fats = bpb["BPB_NumFATs"]
    fat_size = bpb.get("BPB_FATSz32") if pf.fat_type == 32 else bpb.get("BPB_FATSz16")

    print(f"sectors_per_cluster = {sectors_per_cluster}")
    print(f"reserved = {reserved}")
    print(f"fats = {fats}")
    print(f"fat_size = {fat_size}")

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

def create_partition_table(target: str, partition_offset: int, partition_size : int, partition_type : str, bootable : bool = False, index : int = 0):

    make_partitons(target, partition_offset,partition_size,partition_type,index,True if bootable else False)
    
    return

def create_filesystem(target: str, filesystem, reserved_sectors=0, offset=0):
    print(f"target={target} filesystem={filesystem} reserved_sectors={reserved_sectors} offset={offset}")
    if filesystem in ['fat12', 'fat16', 'fat32']:
        reserved_sectors += 1
        if filesystem == 'fat32':
            reserved_sectors = 32
        
        fat_type = filesystem[3:]  # Extract '12', '16', or '32' from 'fat12', 'fat16', 'fat32'
        print(f"mkfs.fat -F {fat_type} -n BESOS -R {reserved_sectors} -s 2 {target} --offset {offset}")
        result = subprocess.run([
            "mkfs.fat",
            "-F", fat_type,
            "-n", "BESOS",
            "-R", str(reserved_sectors),
            "-s", "2",
            target,
            "--offset", str(offset)
        ])
        
        if result.returncode != 0:
            raise ValueError(f'Failed to create FAT{fat_type} filesystem')
    else:
        raise ValueError('Unsupported filesystem ' + filesystem)

def install_mbr(target: str, mbr: str):
    """Install MBR bootloader at sector 0.
    
    Parameters:
        target: Image file path
        mbr: MBR binary path
    """
    print(f"installing MBR {mbr}")
    map_file = Path(mbr).with_suffix('.map')
    if not map_file.exists():
        raise ValueError("Can't find map file for " + mbr)
    
    entry_offset = find_symbol_in_map_file(map_file, '__entry_start')
    if entry_offset is None:
        raise ValueError("Can't find __entry_start symbol in map file " + str(map_file))
    entry_offset_in_binary = entry_offset - 0x7c00
    
    code_end = find_symbol_in_map_file(map_file, '__code_end')
    if code_end is None:
        raise ValueError("Can't find __code_end symbol in map file " + str(map_file))
    code_end_in_binary = code_end - 0x7c00
    
    with open(mbr, 'rb') as fmbr:
        with open(target, 'r+b') as ftarget:
            print(f"entry_offset_in_binary = {entry_offset_in_binary}/{hex(entry_offset_in_binary)}")
            print(f"code_end_in_binary = {code_end_in_binary}/{hex(code_end_in_binary)}")
            
            # Write boot code
            code_size = code_end_in_binary - entry_offset_in_binary + 3
            print(f"code_size = {code_size}/{hex(code_size)}")
            fmbr.seek(0, SEEK_SET)
            ftarget.seek(0, SEEK_SET)
            code = fmbr.read(code_size)
            ftarget.write(code)
            
            ftarget.seek(0x1FE, SEEK_SET)
            ftarget.write(b'\x55\xaa')
            
            print(f"> MBR installed at sector 0")

def install_vbr(target: str, vbr: str, stage2_offset, stage2_size, partition_offset=0):
    """Install VBR bootloader at partition start sector.
    
    Preserves BPB (bytes 0x0B-0x3F) that mkfs.fat creates, overwrites boot code.
    
    Parameters:
        target: Image file path
        vbr: VBR binary path
        stage2_offset: LBA offset where stage2 is located
        stage2_size: Size of stage2 in bytes
        partition_offset: Sector offset of partition start
    """
    print(f"installing VBR {vbr}")
    print(f"> stage2 offset {stage2_offset}")
    print(f"> stage2 size {stage2_size}")
    print(f"> partition_offset {partition_offset}")
    
    map_file = Path(vbr).with_suffix('.map')
    if not map_file.exists():
        raise ValueError("Can't find map file for " + vbr)
    
    byte_offset = partition_offset * SECTOR_SIZE
    
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
            print(f"entry_offset_in_binary = {entry_offset_in_binary}/{hex(entry_offset_in_binary)}")
            print(f"stage2_location_offset_in_binary = {stage2_location_offset_in_binary}/{hex(stage2_location_offset_in_binary)}")
            print(f"code_end_in_binary = {code_end_in_binary}/{hex(code_end_in_binary)}")
            
            # Write first 3 bytes jump instruction at partition start
            # This overwrites the mkfs.fat jump
            ftarget.seek(byte_offset, SEEK_SET)
            jump_bytes = fvbr.read(3)
            print(f"Writing jump bytes: {jump_bytes.hex()} to offset {byte_offset} ({hex(byte_offset)})")
            ftarget.write(jump_bytes)
            
            # Skip mkfs.fat's BPB (bytes 0x0B-0x3F, 53 bytes)
            # Write only the boot code starting at offset 0x40
            # entry_offset should be >= 0x40 to avoid BPB
            code_start_offset = 0x40
            if entry_offset_in_binary < code_start_offset:
                raise ValueError(f"Entry point at 0x{entry_offset_in_binary:x} is before 0x40 (in BPB area)")
            
            code_size = code_end_in_binary - entry_offset_in_binary + 3
            print(f"code_size = {code_size}/{hex(code_size)}")
            print(f"Reading from vbr at offset {entry_offset_in_binary - 3}")
            print(f"Writing to image at offset {entry_offset_in_image - 3} ({hex(entry_offset_in_image - 3)})")
            fvbr.seek(entry_offset_in_binary - 3, SEEK_SET)
            ftarget.seek(entry_offset_in_image - 3, SEEK_SET)
            code = fvbr.read(code_size)
            print(f"Read {len(code)} bytes of code, first 16 bytes: {code[:16].hex()}")
            ftarget.write(code)
            
            # Read SectorsPerCluster from BPB (at byte_offset + 13)
            ftarget.seek(byte_offset + 13, SEEK_SET)
            SectorsPerCluster = int.from_bytes(ftarget.read(1))

            # Write stage2_location: [4 bytes LBA][1 byte sector count]
            realStage2_offset = stage2_offset + (SectorsPerCluster - 1)
            stage2_sectors = (stage2_size + SECTOR_SIZE - 1) // SECTOR_SIZE
            print(f"realStage2_offset = {realStage2_offset}")
            print(f"stage2_sectors = {stage2_sectors}")
            ftarget.seek(stage2_location_offset_in_image, SEEK_SET)
            ftarget.write(realStage2_offset.to_bytes(4, byteorder='little'))
            ftarget.write(stage2_sectors.to_bytes(1, byteorder='little'))
            
            # Flush and sync to disk
            ftarget.flush()
            os.fsync(ftarget.fileno())
            
            print(f"> VBR installed at sector {partition_offset}")

def mmd(image, file_dst, offset_sectors = 0):
    offset_bytes = offset_sectors * SECTOR_SIZE
    image_arg = f"{image}@@{offset_bytes}" if offset_bytes else image
    print(f"mdd(image={image}, file_dst={file_dst}, offset_sectors={offset_sectors})")
    print(f"  offset_bytes={offset_bytes}")
    subprocess.run(["mmd", 
                    "-i", image_arg,
                    file_dst], check=True)

def mcopy(image, file_src, file_dst, offset_sectors = 0):
    offset_bytes = offset_sectors * SECTOR_SIZE
    image_arg = f"{image}@@{offset_bytes}" if offset_bytes else image
    print(f"mcopy(image={image}, file_src={file_src}, file_dst={file_dst}, offset_sectors={offset_sectors})")
    print(f"  offset_bytes={offset_bytes}")
    subprocess.run(["mcopy", 
                    "-i", image_arg,
                    file_src,
                    file_dst], check=True)

def loadFiles(image, files, baseDir, imageDir = "::/", partition_offset = 0):

    print(f"loadFiles({image}, {files}, {baseDir}, {imageDir}, {partition_offset})")
    # copy rest of files
    src_root = baseDir

    print(f"> copying dirs...")
    for file in files:
        file_src = file
        file_rel = os.path.relpath(file_src, src_root)
        file_dst = os.path.join(imageDir, file_rel)
        if os.path.isdir(file_src):
            print(f"entry dir {file_dst}")
            mmd(image, file_dst, partition_offset)
            
    print(f"> copying files...")
    for file in files:
        print(f'have file {file}')
        file_src = file
        file_rel = os.path.relpath(file_src, src_root)
        file_dst = os.path.join(imageDir, file_rel)

        if not os.path.isdir(file_src):
            print('    ... copying', file_rel)
            mcopy(image, file_src, file_dst, partition_offset)
            
MAX_WORKERS = 8

def generate_image_file(target, size):
    print(f"got target as {target}, size as {size}")
    with open(target, 'wb') as fout:
        fout.write(bytes(size * SECTOR_SIZE))
        fout.close()

def generate_image_files(targets):
    filesPath = os.path.join(project_root, "files")
    if not os.path.exists(filesPath):
        os.mkdir(filesPath)
    threads = []
    
    for targetTuple in targets:
        target, size = targetTuple
        t = threading.Thread(target=generate_image_file, args=(target, size))
        threads.append(t)
    for t in threads:
        t.start()
    for t in threads:
        t.join()
                
def calculate_files_partition_size(files):
    total_bytes = sum(os.path.getsize(f) for f in files)
    sectors_needed = (total_bytes + SECTOR_SIZE - 1) // SECTOR_SIZE
    # add some extra for FAT metadata
    return sectors_needed + 128  # +128 sectors for FAT overhead

def build_disk_from_disk(disk : DiskSpec, buildDir : str, filesDir : str):
    print(f"======================================")
    print(f"doing disk {disk.name} with {disk.__str__()}")
    print(f"======================================")
    mbr = os.path.join(buildDir, disk.MBRBootFile) if disk.MBRBootFile != "" else ""
    image = os.path.join(buildDir, disk.image_path)
    build_disk(image, disk, mbr, disk.filesystem, disk.size_sectors)
    offset = 0
    index = 0
    last_partition = None
    s2_offset = 0
    stage2_size = 0
    
    for partition in disk.partitions:
        print(f"======================================")
        print(f"doing partition {partition.name} with {partition.__str__()}")
        print(f"======================================")
        print(f"got target as {partition.root_dir} size as {partition.size_sectors}")
        print(f"buildDir={buildDir}, filesDir={filesDir}")
        
        rootDir = os.path.join(filesDir, partition.root_dir)
        files = GlobRecursive('*', rootDir)
        print(f"got {files} from {rootDir}")
        
        stage2 = os.path.join(buildDir, partition.stage2) if partition.bootable else ""
        kernel = os.path.join(buildDir, partition.kernel) if partition.bootable else ""
        vbr = os.path.join(buildDir, partition.VBRBootFile) if partition.VBRBootFile != "" else ""

        if last_partition != None:
            if last_partition.size_sectors == 0:
                print(f"expected sectors {calculate_files_partition_size(files)}")
                return
            offset += last_partition.size_sectors

        s2_offset2 = build_partition(image, files, partition.bin_files, partition, index, stage2, kernel, rootDir, partition.filesystem, partition.size_sectors, offset, vbr if partition.bootable else "")
        
        if (s2_offset2 != 0): s2_offset = s2_offset2
        
        if s2_offset != 0 and stage2 != "":
            stage2_size = Path(stage2).stat().st_size
        
        last_partition = partition
        index += 1
    
    # Install MBR to sector 0 AFTER all partitions are built
    print(f"s2_offset={s2_offset} stage2_size={stage2_size}")
    if s2_offset != 0 and stage2_size > 0 and mbr != "":
        stage2_sectors = (stage2_size + SECTOR_SIZE - 1) // SECTOR_SIZE
        print(f"> installing MBR...")
        install_mbr(image, mbr)

def build_disk(image, disk : DiskSpec, mbr : str = "", imageFileSystem : str = imageFS, size_sectors : int = 0):
    if (size_sectors == 0):
        size_sectors = (ParseSize(imageSize) + SECTOR_SIZE - 1) // SECTOR_SIZE
    
    print(f"{image}, {size_sectors}")
    arg = [ (image, size_sectors), ]
    generate_image_files(arg)
    
    # Do not create filesystem at root when using partitions - each partition will create its own
    # Only create root filesystem if no partitions are defined
    print(f"try to format the disk")
    if not disk.partitions:
        print(f"formatting the disk with {image}, {imageFileSystem}")
        create_filesystem(image, imageFileSystem, offset=0)
    
    
def build_partition(image, files, bin_files : list[str], partition : DiskPartitionSpec, index : int, stage2 : str = "", kernel : str = "", basePath : str = "", imageFileSystem : str = imageFS, size_sectors : int = 0, sector_offset : int = 0, vbr : str = ""):
    if (size_sectors == 0):
        size_sectors = ((ParseSize(imageSize) + SECTOR_SIZE - 1) // SECTOR_SIZE) - sector_offset
    file_system = imageFileSystem
    partition_offset = sector_offset + partition.partition_offset
    partition.partition_offset = partition_offset
    
    # create partition table
    print(f"> creating partition table...")
    create_partition_table(image, partition_offset, size_sectors, file_system, partition.bootable, index)

    # Create filesystem
    print(f"> formatting file using {file_system}...")
    create_filesystem(image, file_system, offset=partition_offset)
    
    partition.size_sectors = size_sectors
    stage2_offset = 0
    
    # For bootable partitions: copy stage2, get offset, and install VBR if provided
    if partition.bootable and stage2 != "":
        print(f"stage2 = {stage2}")
        
        # Now copy stage2 and get its sector offset
        print(f"> copying stage2...")
        first_data_sector = GetStage2Sector(stage2, image, partition_offset)
        stage2_size = Path(stage2).stat().st_size
        stage2_sectors = (stage2_size + SECTOR_SIZE - 1) // SECTOR_SIZE
        stage2_offset = first_data_sector + 1 + partition_offset
        print(f"> stage2 offset {stage2_offset}")
        
        # Install VBR if provided BEFORE loading files (so it doesn't get overwritten)
        if vbr != "" and os.path.exists(vbr):
            print(f"> installing VBR...")
            install_vbr(image, vbr, stage2_offset, stage2_size, partition_offset)
    
    try:
        if kernel != "":
            print(f"> copying kernel...")
            mmd(image, "boot", partition_offset)
            mcopy(image, kernel, "::/boot/kernel.elf", partition_offset)
            

        loadFiles(image, files, basePath, "::/", partition_offset)

        # load bin files
        if bin_files.__len__() > 0:
            user_path = os.path.join(project_root, f"build/{arch}_{config}", "user")
            mmd(image, "bin", partition_offset)
            for file in bin_files:
                file_src = os.path.join(user_path, file)
                file_name = os.path.basename(file)
                file_dst = os.path.join("::/bin", file_name)
                print(f'    ... copying {file_src} to {file_dst}')
                if not os.path.isdir(file_src):
                    mcopy(image, file_src, file_dst, partition_offset)
                
    finally:
        print("> cleaning up...")
    
    return stage2_offset
    
    

files_dir = os.path.join(project_root, "files")

if not os.path.exists(files_dir):
    os.mkdir(files_dir)

buildsPath = os.path.join(project_root, "build")
if not os.path.exists(buildsPath):
    exit(-1)

stage1mbr = os.path.join(project_root, f"build/{arch}_{config}/stage1/mbr.bin")
stage1vbr = os.path.join(project_root, f"build/{arch}_{config}/stage1/vbr.bin")
stage2 = os.path.join(project_root, f"build/{arch}_{config}/stage2/stage2.bin")
kernel = os.path.join(project_root, f"build/{arch}_{config}/kernel/kernel.elf")

disks.append(DiskSpec("main", "image.img", 0, imageFS, [
    DiskPartitionSpec("boot", "root", 4096, "fat32", 4096, True, [], stage1vbr, stage2, kernel),
    DiskPartitionSpec("User", "user", 4096, "fat32", 0, False, ["init.elf"])
    ], stage1mbr))

for disk in disks:
    build_disk_from_disk(disk, f"build/{arch}_{config}", files_dir)
