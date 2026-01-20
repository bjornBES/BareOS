import os
import sys
import subprocess
    
if len(sys.argv) < 7:
    print("Usage: partitons.py image start size type index bootable")
    exit(1)

PARTITION_TYPES = {
    "fat12": "01",
    "fat16": "06",
    "fat32": "0B",
    "fat32_lba": "0C",
    "linux": "83"
}


image = sys.argv[1]
start = sys.argv[2]
size = sys.argv[3]
ptype = sys.argv[4]
index = sys.argv[5]
bootable = sys.argv[6]

print("running make")

project_root = os.path.abspath(os.path.dirname(__file__))
sys.path.append(project_root)

print(f"project_root = {project_root}")
makePath = os.path.join(project_root, "c")
subprocess.run(["make", "-C", makePath])

binPath = os.path.join(makePath, "partitons")
subprocess.run([
    binPath,
    image,
    start,
    size,
    PARTITION_TYPES[ptype],
    index,
    bootable
], check=True)

print("partitons done")