
class disk_partition_spec:
    def __init__(
        self,
        name : str, # Name of the Partition
        root_dir : str, # The dir in rootfs that this partition will have
        partition_offset : int, # partition offset in sectors
        filesystem : str, # the file system type of this partition
        size_bytes : int, # size of the partition in bytes
        bootable : bool, # is the partition bootable
        in_gpt : bool, # should this partition be in the gpt (true) or the mbr (false)
        bin_files : list[str] = [], # files outside the rootfs that needs to be include
        vbr_file : str = "", # path to this partitions VBR
        stage2 : str = "", # path to this partitions stage2
        kernel : str = "" # path to this partitions kernel
        
    ):
        # Name of the Partition
        self.name: str = name
        
        # The dir in rootfs that this partition will have
        self.root_dir: str = root_dir
        
        # partition offset in sectors
        self.partition_offset: int = partition_offset
        
        # the file system type of this partition
        self.filesystem: str = filesystem
        
        # size of the partition in bytes
        self.partition_size_bytes: int = size_bytes 

        # size of the partition in sectors
        self.partition_size_sectors : int = 0 
        
        # is the partition bootable
        self.bootable: bool = bootable
        
        # should this partition be in the gpt (true) or the mbr (false)
        self.in_gpt: bool = in_gpt
        
        # path to this partitions VBR
        self.vbr_file: str = vbr_file
        
        # path to this partitions stage2
        self.stage2: str = stage2
        
        # path to this partitions kernel
        self.kernel: str = kernel
        
        # files outside the rootfs that needs to be include
        self.bin_files: list[str] = bin_files
        
        self.partition_data : bytearray = bytearray(0)

    def __str__(self):
        return f"{self.name}, {self.root_dir} {self.partition_offset} {self.filesystem} {self.partition_size_bytes} {self.bootable}"


class disk_spec:
    def __init__(
        self,
        name : str,
        image_path : str,
        disk_size : int,
        sector_size : int,
        filesystem : str,
        use_mbr : bool,
        partitions : list[disk_partition_spec],
        mbr_file : str,
        
    ):
        self.name = name
        self.image_path = image_path
        self.sector_size = sector_size
        self.disk_size = disk_size
        self.disk_size_sectors = disk_size // sector_size
        self.filesystem = filesystem
        self.partitions = partitions
        self.mbr_file = mbr_file
        self.use_mbr = use_mbr
    
    def __str__(self):
        return f"{self.name}, {self.image_path} {self.disk_size} {self.filesystem} {self.partitions} {self.mbr_file}"

disks = [
    
]
