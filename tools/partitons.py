import os
import struct
import sys
import subprocess
    
    
PARTITION_TYPES = {
    "fat12": "01",
    "fat16": "06",
    "fat32": "0B",
    "fat32_lba": "0C",
    "linux": "83"
}

SECTOR_SIZE = 512
PARTITION_TABLE_OFFSET = 0x1BE

def create_partition_table(target: str, start: int, size: int, ptype: int, index: int, bootable: bool):
    print(f"create_partition_table({target}:{type(target)}, {start}:{type(start)}, {size}:{type(size)}, {ptype}:{type(ptype)}, {index}:{type(index)}, {bootable}:{type(bootable)})")
    with open(target, "r+b") as f:
        mbr = bytearray(f.read(SECTOR_SIZE))

        status = 0x80 if bootable else 0x00
        entry = struct.pack("<BBBBBBBBII",
            status,
            0, 0, 0,   # CHS first (ignored)
            ptype,
            0, 0, 0,   # CHS last (ignored)
            start,
            size
        )

        offset = PARTITION_TABLE_OFFSET + index * 16
        mbr[offset:offset + 16] = entry

        f.seek(0)
        f.write(mbr)

# image start size type index bootable
def make_partitons(image, start, size, ptype, index, bootable):
    print("running make")

    project_root = os.path.abspath(os.path.dirname(__file__))
    sys.path.append(project_root)

    print(f"project_root = {project_root}")

    create_partition_table(
        image, int(start), int(size), int(PARTITION_TYPES[ptype], 16), int(index), bool(bootable))

    print("partitons done")