#!/bin/bash

# Script to generate large binary asset files with truly non-compressible data

set -e

ASSETS_DIR="app/src/main/assets/payload"
TARGET_SIZE=4500000  # ~4.5 MB per file

echo "Generating large binary asset files with improved non-compressible data..."

# Function to generate a truly non-compressible binary file
generate_binary_file() {
    local filename="$1"
    local target_size="$2"
    
    echo "Creating $filename ($target_size bytes)..."
    
    # Generate structured data with headers and truly random content
    python3 -c "
import struct
import os
import random

filename = '$filename'
target_size = $target_size

# Block structure: 256-byte header + 3840 bytes random data = 4096 bytes per block
block_size = 4096
header_size = 256
data_size = block_size - header_size
num_blocks = target_size // block_size

# Use different seeds for each file to ensure variety
file_seeds = {
    'map_tiles_chunk_01.bin': 12345,
    'map_tiles_chunk_02.bin': 67890, 
    'telemetry_capture_01.bin': 24680,
    'telemetry_capture_02.bin': 13579,
    'qt_stub_resources.bin': 98765
}

base_seed = file_seeds.get(os.path.basename(filename), 11111)
random.seed(base_seed)

with open(filename, 'wb') as f:
    for block_idx in range(num_blocks):
        # Create a 256-byte structured header with varying content
        header = bytearray(header_size)
        
        # Block header structure (little endian):
        # 0-3: Magic number
        # 4-7: Block index 
        # 8-11: Block size
        # 12-15: Random seed for this block
        # 16-31: Timestamp-like data
        # 32-63: GPS coordinates (8 doubles)
        # 64-127: Sensor data (16 floats)
        # 128-255: Random padding
        
        struct.pack_into('<I', header, 0, 0x50495848)  # 'PIXH' magic
        struct.pack_into('<I', header, 4, block_idx)
        struct.pack_into('<I', header, 8, block_size)
        
        block_seed = base_seed + block_idx * 7919  # Large prime for good distribution
        struct.pack_into('<I', header, 12, block_seed)
        
        # Varying timestamp
        struct.pack_into('<Q', header, 16, 1609459200000 + block_idx * random.randint(50, 200))
        
        # Slightly randomized GPS coordinates  
        base_lat, base_lon = 37.7749, -122.4194
        for i in range(4):  # 4 coordinate pairs
            lat = base_lat + random.uniform(-0.01, 0.01) + (block_idx * 0.00001)
            lon = base_lon + random.uniform(-0.01, 0.01) + (block_idx * 0.00001)
            struct.pack_into('<d', header, 32 + i*16, lat)
            struct.pack_into('<d', header, 40 + i*16, lon)
        
        # Random sensor data  
        for i in range(16):
            value = random.uniform(-1000.0, 1000.0)
            struct.pack_into('<f', header, 64 + i*4, value)
        
        # Fill rest of header with random data
        for i in range(128, header_size):
            header[i] = random.randint(0, 255)
        
        # Generate truly random data block using the block seed
        random.seed(block_seed)
        data = bytearray(random.randint(0, 255) for _ in range(data_size))
        
        # Write header and data
        f.write(header)
        f.write(data)
        
        # Reset to main sequence for next iteration
        random.seed(base_seed + (block_idx + 1) * 7919)
    
    # Pad to exact target size if needed
    current_size = f.tell()
    if current_size < target_size:
        padding = target_size - current_size
        # Pad with random data, not zeros
        random.seed(base_seed + 99999)
        padding_data = bytearray(random.randint(0, 255) for _ in range(padding))
        f.write(padding_data)

print(f'Generated {filename} ({os.path.getsize(filename)} bytes)')
"
}

# Generate the 5 asset files
mkdir -p "$ASSETS_DIR"
cd "$ASSETS_DIR"

# Remove old files
rm -f *.bin

generate_binary_file "map_tiles_chunk_01.bin" $TARGET_SIZE
generate_binary_file "map_tiles_chunk_02.bin" $TARGET_SIZE  
generate_binary_file "telemetry_capture_01.bin" $TARGET_SIZE
generate_binary_file "telemetry_capture_02.bin" $TARGET_SIZE
generate_binary_file "qt_stub_resources.bin" $TARGET_SIZE

echo
echo "Asset file generation complete:"
ls -lh *.bin

echo
echo "Total asset size:"
du -sh .

echo
echo "Verifying files are not highly compressible..."
for file in *.bin; do
    original_size=$(stat -f%z "$file" 2>/dev/null || stat -c%s "$file" 2>/dev/null || echo "unknown")
    compressed_size=$(gzip -c "$file" | wc -c)
    if [ "$original_size" != "unknown" ]; then
        compression_ratio=$(echo "scale=2; $compressed_size * 100 / $original_size" | bc -l 2>/dev/null || echo "unknown")
        echo "$file: $original_size -> $compressed_size bytes (${compression_ratio}% of original)"
    else
        echo "$file: compression check completed"
    fi
done