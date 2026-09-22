#ifndef NTH_PROTOCOL_H
#define NTH_PROTOCOL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Linear GOP Framebuffer provided by the nth bootloader.
     */
    typedef struct
    {
        uint64_t BaseAddress;       /**< 64-bit physical address of the framebuffer */
        uint64_t BufferSize;        /**< Total size of framebuffer in bytes */
        uint32_t Width;             /**< Horizontal resolution in pixels */
        uint32_t Height;            /**< Vertical resolution in pixels */
        uint32_t PixelsPerScanLine; /**< Number of pixels per scanline (including alignment/padding) */
    } NthFramebuffer;

    /**
     * @brief Standard UEFI Memory Types.
     */
    typedef enum
    {
        NthEfiReservedMemoryType,
        NthEfiLoaderCode,
        NthEfiLoaderData,
        NthEfiBootServicesCode,
        NthEfiBootServicesData,
        NthEfiRuntimeServicesCode,
        NthEfiRuntimeServicesData,
        NthEfiConventionalMemory,
        NthEfiUnusableMemory,
        NthEfiACPIReclaimMemory,
        NthEfiACPIMemoryNVS,
        NthEfiMemoryMappedIO,
        NthEfiMemoryMappedIOPortSpace,
        NthEfiPalCode,
        NthEfiPersistentMemory,
        NthEfiMaxMemoryType
    } NthMemoryType;

    /**
     * @brief Memory Descriptor structure representing an entry in the UEFI Memory Map.
     */
    typedef struct
    {
        uint32_t Type; /**< NthMemoryType */
        uint32_t Pad;
        uint64_t PhysicalStart; /**< Physical starting address of the memory region */
        uint64_t VirtualStart;  /**< Virtual starting address of the memory region */
        uint64_t NumberOfPages; /**< Number of 4KiB pages in this region */
        uint64_t Attribute;     /**< Memory attribute flags */
    } __attribute__((packed)) NthMemoryDescriptor;

    /**
     * @brief The primary handoff structure passed by the nth bootloader to kernel_main.
     * Passed as the first argument in the System V AMD64 ABI (%rdi).
     */
    typedef struct
    {
        NthFramebuffer *Framebuffer; /**< Pointer to framebuffer information, or NULL if unavailable */
        void *MemoryMap;             /**< Pointer to the UEFI Memory Map buffer */
        uint64_t MapSize;            /**< Total size of the memory map in bytes */
        uint64_t DescriptorSize;     /**< Size of each memory descriptor entry in bytes */
        void *Rsdp;                  /**< Pointer to the ACPI RSDP table (ACPI 2.0+) */
    } NthBootInfo;

#ifdef __cplusplus
}
#endif

#endif /* NTH_PROTOCOL_H */
