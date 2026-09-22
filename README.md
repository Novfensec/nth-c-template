# nth C Kernel Template

A clean, freestanding C template for building 64-bit kernels compliant with the **nth Boot Protocol**.

### Overview

`nth-c-template` provides a lightweight, battery-included starting point for operating system developers who want to write kernels booted by the [`nth`](https://github.com/Novfensec/nth) bootloader.

It includes:
- **`include/nth_protocol.h`**: Complete definitions for the handoff protocol structures (`NthBootInfo`, `NthFramebuffer`, `NthMemoryDescriptor`).
- **`src/kernel.c`**: An entry point demonstration showing screen initialization, GOP linear framebuffer rendering, memory map iteration, and ACPI RSDP discovery.
- **`src/font.h`**: An embedded 8x8 bitmap font for instant text rendering without external dependencies.
- **`linker.ld`**: A standard 64-bit ELF linker script configured for 1 MiB load base.
- **`CMakeLists.txt` & `Makefile`**: Modern CMake build system with a lightweight Makefile wrapper.

## Machine State at Kernel Handoff

When the `nth` bootloader jumps to your kernel entry point (`kernel_main`), the machine is configured as follows:

| Component | State |
| :--- | :--- |
| **CPU Mode** | 64-bit Long Mode |
| **Paging** | Enabled (Identity-mapped physical memory) |
| **Interrupts** | Disabled (`cli`) |
| **Graphics** | Linear GOP Framebuffer initialized |
| **Boot Services** | UEFI Boot Services have exited (`ExitBootServices`) |
| **Registers** | System V AMD64 ABI: `%rdi` contains pointer to `NthBootInfo` |

## The Handoff Protocol

The `NthBootInfo` structure is passed in `%rdi`:

```c
#include "nth_protocol.h"

void kernel_main(NthBootInfo *boot_info) {
    if (!boot_info) {
        for (;;) { __asm__ volatile("hlt"); }
    }

    // Framebuffer graphics
    NthFramebuffer *fb = boot_info->Framebuffer;
    uint32_t width = fb->Width;
    uint32_t height = fb->Height;

    // UEFI Memory Map
    void *mmap = boot_info->MemoryMap;
    uint64_t map_size = boot_info->MapSize;
    uint64_t desc_size = boot_info->DescriptorSize;

    // ACPI RSDP
    void *rsdp = boot_info->Rsdp;

    while (1) {
        __asm__ volatile("hlt");
    }
}
```

### Protocol Header (`include/nth_protocol.h`)

```c
typedef struct {
    uint64_t BaseAddress;       // Physical address of linear GOP framebuffer
    uint64_t BufferSize;        // Size of framebuffer in bytes
    uint32_t Width;             // Horizontal resolution in pixels
    uint32_t Height;            // Vertical resolution in pixels
    uint32_t PixelsPerScanLine; // Pixels per scanline (including pitch/padding)
} NthFramebuffer;

typedef struct {
    NthFramebuffer *Framebuffer;
    void *MemoryMap;           // Pointer to the UEFI Memory Map
    uint64_t MapSize;          // Total size of memory map in bytes
    uint64_t DescriptorSize;   // Size of each memory descriptor entry
    void *Rsdp;                // Pointer to ACPI RSDP table (if found)
} NthBootInfo;
```

## Getting Started

### Prerequisites

You need CMake (>= 3.16) and a working C compiler capable of emitting freestanding 64-bit ELF binaries (`gcc` or `x86_64-elf-gcc`).

#### Debian / Ubuntu / WSL
```bash
sudo apt update
sudo apt install -y build-essential cmake gcc
```

### Building the Kernel

Configure and compile the kernel using **CMake**:

```bash
cmake -B build
cmake --build build
```

*(Alternatively, you can simply run `make` which wraps the CMake commands).*

The resulting kernel executable is located at `build/kernel.elf`.

To clean build artifacts:
```bash
rm -rf build
# or: make clean
```

## Booting with `nth`

1. Copy `build/kernel.elf` into your bootable media or ISO root (e.g., inside `esp/` or alongside `BOOTX64.EFI`).
2. Run your bootloader image script (such as `mk_iso.sh` in the `nth` repository).
3. Start the virtual machine with QEMU:
   ```bash
   qemu-system-x86_64 -bios /path/to/OVMF.fd -cdrom nth_os.iso
   ```

## Directory Structure

```
nth-c-template/
├── .gitignore          # Git ignore patterns for build objects
├── CMakeLists.txt      # Primary CMake configuration
├── Makefile            # Convenient wrapper for CMake
├── linker.ld           # ELF64 linker script (1 MiB base)
├── README.md           # Documentation & protocol guide
├── include/
│   └── nth_protocol.h  # Protocol types and structures
└── src/
    ├── font.h          # Minimal embedded 8x8 font
    └── kernel.c        # Entry point and demo implementation
```

## License

This template is released under the permissive [MIT License](LICENSE) or Unlicense—feel free to use it as the foundation for your own operating system!
