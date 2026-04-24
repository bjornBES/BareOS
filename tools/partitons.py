import dis
from hmac import new
import os
import struct
import sys
from turtle import st
from uuid import UUID
import gpt_image.disk
import gpt_image.partition
import subprocess

from numpy import byte

from tools import disk
from tools.disk import DiskPartitionSpec
    
PARTITION_TYPES = {
    "fat12": "01",
    "fat16": "06",
    "fat32": "0B",
    "fat32_lba": "0C",
    "linux": "83"
}

SECTOR_SIZE = 512
PARTITION_TABLE_OFFSET = 0x1BE
PART_ENTRY_SIZE = 128

partition_data : bytearray = bytearray(0)

def create_partition_table(disk : gpt_image.disk.Disk, start: int, size: int, ptype: int, index: int, partition: DiskPartitionSpec):
    
    if partition.in_gpt:
        partuuid : UUID = UUID(gpt_image.partition.PartitionType.EFI_SYSTEM_PARTITION.value)
        new_part = gpt_image.partition.Partition(partition.name, size * disk.sector_size, partuuid.hex)
        new_part._first_lba.value = start
        new_part._first_lba.commit()
        new_part._last_lba.value = start + size - 1
        new_part._last_lba.commit()
        new_part.partition_name = partition.name
        
        header = gpt_image.table.Header(disk.geometry)
        
        part_data = bytearray(new_part.marshal())
        part_data = part_data.ljust(header.size_of_partition_entries, b'\x00')
        
        partition.partition_data = part_data
        
        disk.table.partitions.add(new_part)
    else:
        with open(disk.image_path, "r+b") as f:
            mbr = bytearray(f.read(SECTOR_SIZE))

            status = 0x80 if partition.bootable else 0x00
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
def make_partitions(disk : gpt_image.disk.Disk, start, size, ptype, partition : DiskPartitionSpec, index):

    create_partition_table(
        disk, int(start), int(size), int(PARTITION_TYPES[ptype], 16), int(index), partition)

    print("partitions done")

