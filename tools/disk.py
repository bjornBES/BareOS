
class DiskPartitionSpec:
    def __init__(
        self,
        name : str,
        root_dir : str,
        partition_offset : int,
        filesystem : str,
        size_sectors : int,
        bootable : bool,
        in_gpt : bool,
        bin_files : list[str] = [],
        VBRBootFile : str = "",
        stage2 : str = "",
        kernel : str = ""
        
    ):
        self.name = name
        self.root_dir = root_dir
        self.partition_offset = partition_offset
        self.filesystem = filesystem
        self.size_sectors = size_sectors
        self.bootable = bootable
        self.in_gpt = in_gpt
        self.VBRBootFile = VBRBootFile
        self.stage2 = stage2
        self.kernel = kernel
        self.bin_files = bin_files
        self.partition_data = bytearray(0)

    def __str__(self):
        return f"{self.name}, {self.root_dir} {self.partition_offset} {self.filesystem} {self.size_sectors} {self.bootable}"


class DiskSpec:
    def __init__(
        self,
        name : str,
        image_path : str,
        size_sectors : int,
        filesystem : str,
        use_mbr : bool,
        partitions : list[DiskPartitionSpec],
        MBRBootFile : str,
        
    ):
        self.name = name
        self.image_path = image_path
        self.size_sectors = size_sectors
        self.filesystem = filesystem
        self.partitions = partitions
        self.MBRBootFile = MBRBootFile
        self.use_mbr = use_mbr
    
    def __str__(self):
        return f"{self.name}, {self.image_path} {self.size_sectors} {self.filesystem} {self.partitions} {self.MBRBootFile}"

disks = [
    
]
