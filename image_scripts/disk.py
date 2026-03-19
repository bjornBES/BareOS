
class DiskPartitionSpec:
    def __init__(
        self,
        name : str,
        root_dir : str,
        partition_offset : int,
        filesystem : str,
        size_sectors : int,
        bootable : bool,
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
        self.VBRBootFile = VBRBootFile
        self.stage2 = stage2
        self.kernel = kernel
        self.bin_files = bin_files


class DiskSpec:
    def __init__(
        self,
        name : str,
        image_path : str,
        size_sectors : int,
        filesystem : str,
        partitions : list[DiskPartitionSpec],
        MBRBootFile : str,
        
    ):
        self.name = name
        self.image_path = image_path
        self.size_sectors = size_sectors
        self.filesystem = filesystem
        self.partitions = partitions
        self.MBRBootFile = MBRBootFile

disks = [
    
]
