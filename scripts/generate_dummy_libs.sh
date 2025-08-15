#!/bin/bash

# Script to generate dummy native libraries for APK size inflation

set -e

TARGET_SIZE=4718592  # ~4.5 MB per library
JNILIBS_DIR="app/src/main/jniLibs"

echo "Generating dummy native libraries for APK size inflation..."

# Library names to create
LIBS=(
    "libqt_stub_gui.so"
    "libqt_stub_location.so" 
    "libqt_stub_network.so"
    "libqt_stub_positioning.so"
    "libqt_stub_multimedia.so"
    "libqt_stub_qml.so"
)

# ABIs to support
ABIS=(
    "armeabi-v7a"
    "arm64-v8a" 
    "x86_64"
)

# Function to create a minimal valid ELF library
create_dummy_lib() {
    local abi="$1"
    local libname="$2" 
    local target_size="$3"
    local output_file="$4"
    
    echo "Creating $libname for $abi ($target_size bytes)..."
    
    # Generate ELF library using Python
    python3 -c "
import struct
import os

abi = '$abi'
libname = '$libname'
target_size = $target_size
output_file = '$output_file'

# ELF header structure depends on architecture
if abi == 'armeabi-v7a':
    # 32-bit ARM ELF
    elf_class = 1  # ELFCLASS32
    elf_data = 1   # ELFDATA2LSB (little endian)
    elf_machine = 40  # EM_ARM
    elf_header_size = 52
    program_header_size = 32
    section_header_size = 40
elif abi == 'arm64-v8a':
    # 64-bit ARM ELF  
    elf_class = 2  # ELFCLASS64
    elf_data = 1   # ELFDATA2LSB
    elf_machine = 183  # EM_AARCH64
    elf_header_size = 64
    program_header_size = 56
    section_header_size = 64
elif abi == 'x86_64':
    # 64-bit x86 ELF
    elf_class = 2  # ELFCLASS64
    elf_data = 1   # ELFDATA2LSB  
    elf_machine = 62  # EM_X86_64
    elf_header_size = 64
    program_header_size = 56
    section_header_size = 64
else:
    raise ValueError(f'Unsupported ABI: {abi}')

with open(output_file, 'wb') as f:
    # ELF Header
    elf_header = bytearray(elf_header_size)
    
    # ELF Magic number
    elf_header[0:4] = b'\\x7fELF'
    elf_header[4] = elf_class  # EI_CLASS
    elf_header[5] = elf_data   # EI_DATA  
    elf_header[6] = 1          # EI_VERSION
    elf_header[7] = 0          # EI_OSABI (System V)
    # EI_PAD bytes 8-15 remain zero
    
    if elf_class == 1:  # 32-bit
        struct.pack_into('<H', elf_header, 16, 3)  # e_type: ET_DYN (shared object)
        struct.pack_into('<H', elf_header, 18, elf_machine)  # e_machine
        struct.pack_into('<I', elf_header, 20, 1)  # e_version
        struct.pack_into('<I', elf_header, 24, 0x8000)  # e_entry
        struct.pack_into('<I', elf_header, 28, elf_header_size)  # e_phoff  
        struct.pack_into('<I', elf_header, 32, 0)  # e_shoff (no sections)
        struct.pack_into('<I', elf_header, 36, 0)  # e_flags
        struct.pack_into('<H', elf_header, 40, elf_header_size)  # e_ehsize
        struct.pack_into('<H', elf_header, 42, program_header_size)  # e_phentsize
        struct.pack_into('<H', elf_header, 44, 1)  # e_phnum (1 program header)
        struct.pack_into('<H', elf_header, 46, section_header_size)  # e_shentsize
        struct.pack_into('<H', elf_header, 48, 0)  # e_shnum
        struct.pack_into('<H', elf_header, 50, 0)  # e_shstrndx
    else:  # 64-bit
        struct.pack_into('<H', elf_header, 16, 3)  # e_type: ET_DYN
        struct.pack_into('<H', elf_header, 18, elf_machine)  # e_machine
        struct.pack_into('<I', elf_header, 20, 1)  # e_version
        struct.pack_into('<Q', elf_header, 24, 0x8000)  # e_entry
        struct.pack_into('<Q', elf_header, 32, elf_header_size)  # e_phoff
        struct.pack_into('<Q', elf_header, 40, 0)  # e_shoff
        struct.pack_into('<I', elf_header, 48, 0)  # e_flags
        struct.pack_into('<H', elf_header, 52, elf_header_size)  # e_ehsize
        struct.pack_into('<H', elf_header, 54, program_header_size)  # e_phentsize  
        struct.pack_into('<H', elf_header, 56, 1)  # e_phnum
        struct.pack_into('<H', elf_header, 58, section_header_size)  # e_shentsize
        struct.pack_into('<H', elf_header, 60, 0)  # e_shnum
        struct.pack_into('<H', elf_header, 62, 0)  # e_shstrndx
    
    f.write(elf_header)
    
    # Program Header (PT_LOAD)
    program_header = bytearray(program_header_size)
    data_start = elf_header_size + program_header_size
    data_size = target_size - data_start
    
    if elf_class == 1:  # 32-bit
        struct.pack_into('<I', program_header, 0, 1)  # p_type: PT_LOAD
        struct.pack_into('<I', program_header, 4, data_start)  # p_offset
        struct.pack_into('<I', program_header, 8, 0x8000)  # p_vaddr
        struct.pack_into('<I', program_header, 12, 0x8000)  # p_paddr  
        struct.pack_into('<I', program_header, 16, data_size)  # p_filesz
        struct.pack_into('<I', program_header, 20, data_size)  # p_memsz
        struct.pack_into('<I', program_header, 24, 5)  # p_flags: PF_R | PF_X
        struct.pack_into('<I', program_header, 28, 0x1000)  # p_align
    else:  # 64-bit
        struct.pack_into('<I', program_header, 0, 1)  # p_type: PT_LOAD
        struct.pack_into('<I', program_header, 4, 5)  # p_flags: PF_R | PF_X
        struct.pack_into('<Q', program_header, 8, data_start)  # p_offset
        struct.pack_into('<Q', program_header, 16, 0x8000)  # p_vaddr
        struct.pack_into('<Q', program_header, 24, 0x8000)  # p_paddr
        struct.pack_into('<Q', program_header, 32, data_size)  # p_filesz  
        struct.pack_into('<Q', program_header, 40, data_size)  # p_memsz
        struct.pack_into('<Q', program_header, 48, 0x1000)  # p_align
    
    f.write(program_header)
    
    # Add .note section with description
    note_desc = f'Dummy stub library {libname} for {abi} - placeholder for Qt integration'
    note_padding = (4 - (len(note_desc) % 4)) % 4
    note_data = struct.pack('<III', 4, len(note_desc) + note_padding, 0) + b'PIXH' + note_desc.encode('utf-8') + b'\\x00' * note_padding
    f.write(note_data)
    
    # Fill remaining space with structured padding data
    current_pos = f.tell()
    remaining = target_size - current_pos
    
    # Create padding that varies to avoid compression
    import random
    random.seed(hash(libname + abi) & 0x7FFFFFFF)
    
    block_size = 4096
    blocks = remaining // block_size
    for i in range(blocks):
        block = bytearray(random.randint(0, 255) for _ in range(block_size))
        f.write(block)
    
    # Fill any remaining bytes
    final_remaining = remaining % block_size
    if final_remaining > 0:
        final_block = bytearray(random.randint(0, 255) for _ in range(final_remaining))
        f.write(final_block)

print(f'Generated {output_file} ({os.path.getsize(output_file)} bytes)')
"
}

# Generate libraries for all ABIs
for abi in "${ABIS[@]}"; do
    echo "Generating libraries for $abi..."
    abi_dir="$JNILIBS_DIR/$abi"
    mkdir -p "$abi_dir"
    
    for lib in "${LIBS[@]}"; do
        create_dummy_lib "$abi" "$lib" "$TARGET_SIZE" "$abi_dir/$lib"
    done
    echo
done

echo "Dummy library generation complete!"
echo
echo "Library sizes per ABI:"
for abi in "${ABIS[@]}"; do
    echo "$abi:"
    ls -lh "$JNILIBS_DIR/$abi/"
    echo "Total for $abi: $(du -sh "$JNILIBS_DIR/$abi" | cut -f1)"
    echo
done

echo "Overall jniLibs size:"
du -sh "$JNILIBS_DIR"