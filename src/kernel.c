#include "nth_protocol.h"
#include "font.h"

/* Global pointer to the active framebuffer */
static NthFramebuffer *fb = 0;

/**
 * @brief Plots a 32-bit (ARGB/XRGB) pixel at coordinate (x, y).
 */
static void put_pixel(uint32_t x, uint32_t y, uint32_t color)
{
    if (!fb || x >= fb->Width || y >= fb->Height)
    {
        return;
    }
    uint32_t *pixel_addr = (uint32_t *)(fb->BaseAddress + 4 * (y * fb->PixelsPerScanLine + x));
    *pixel_addr = color;
}

/**
 * @brief Fills the entire screen with a solid color.
 */
static void clear_screen(uint32_t color)
{
    if (!fb)
        return;
    for (uint32_t y = 0; y < fb->Height; y++)
    {
        uint32_t *row = (uint32_t *)(fb->BaseAddress + 4 * (y * fb->PixelsPerScanLine));
        for (uint32_t x = 0; x < fb->Width; x++)
        {
            row[x] = color;
        }
    }
}

#define FONT_SCALE 2

/**
 * @brief Draws a single character using the embedded 8x8 font.
 */
static void draw_char(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg)
{
    if (!fb || c < 32 || c > 126)
        return;
    const uint8_t *glyph = font8x8_basic[c - 32];
    for (int row = 0; row < 8; row++)
    {
        uint8_t bits = glyph[row];
        for (int col = 0; col < 8; col++)
        {
            if (bits & (1 << (7 - col)))
            {
                for (uint32_t dy = 0; dy < FONT_SCALE; dy++)
                    for (uint32_t dx = 0; dx < FONT_SCALE; dx++)
                        put_pixel(x + (col * FONT_SCALE) + dx, y + (row * FONT_SCALE) + dy, fg);
            }
            else if (bg != 0)
            {
                for (uint32_t dy = 0; dy < FONT_SCALE; dy++)
                    for (uint32_t dx = 0; dx < FONT_SCALE; dx++)
                        put_pixel(x + (col * FONT_SCALE) + dx, y + (row * FONT_SCALE) + dy, bg);
            }
        }
    }
}

/**
 * @brief Prints a null-terminated string at (x, y). Returns the new X coordinate.
 */
static uint32_t print_string(uint32_t x, uint32_t y, const char *str, uint32_t fg, uint32_t bg)
{
    uint32_t cur_x = x;
    uint32_t cur_y = y;

    while (*str)
    {
        if (*str == '\n')
        {
            cur_x = x;
            cur_y += 12 * FONT_SCALE;
        }
        else
        {
            draw_char(cur_x, cur_y, *str, fg, bg);
            cur_x += 8 * FONT_SCALE;
        }
        str++;
    }
    return cur_x;
}

/**
 * @brief Draws a colored rectangle.
 */
static void draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color)
{
    for (uint32_t dy = 0; dy < h; dy++)
    {
        for (uint32_t dx = 0; dx < w; dx++)
        {
            put_pixel(x + dx, y + dy, color);
        }
    }
}

/**
 * @brief Converts an unsigned integer to string (decimal or hex).
 */
static void utoa(uint64_t val, char *buf, int base)
{
    char temp[64];
    int i = 0;
    if (val == 0)
    {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    while (val > 0)
    {
        int rem = val % base;
        temp[i++] = (rem < 10) ? ('0' + rem) : ('A' + rem - 10);
        val /= base;
    }
    int j = 0;
    while (i > 0)
    {
        buf[j++] = temp[--i];
    }
    buf[j] = '\0';
}

/**
 * @brief Kernel Entry Point called by the nth bootloader.
 *
 * Calling convention: System V AMD64 ABI
 * The boot_info structure pointer is passed in register %rdi.
 */
void kernel_main(NthBootInfo *boot_info)
{
    if (!boot_info)
    {
        while (1)
        {
            __asm__ volatile("hlt");
        }
    }

    fb = boot_info->Framebuffer;

    if (fb && fb->BaseAddress != 0)
    {
        /* Clear screen with dark slate background (0x00121820) */
        clear_screen(0x00121820);

        /* Top header banner (0x001E293B) */
        draw_rect(0, 0, fb->Width, 48 * FONT_SCALE, 0x001E293B);
        draw_rect(0, 48 * FONT_SCALE, fb->Width, 2 * FONT_SCALE, 0x0038BDF8); // Cyan accent bar

        print_string(24 * FONT_SCALE, 18 * FONT_SCALE, "nth Boot Protocol - C Kernel Template", 0x0038BDF8, 0);

        /* Card background */
        draw_rect(24 * FONT_SCALE, 70 * FONT_SCALE, 680 * FONT_SCALE, 320 * FONT_SCALE, 0x001E293B);

        print_string(44 * FONT_SCALE, 90 * FONT_SCALE, "System Initialization Report:", 0x00F1F5F9, 0);

        /* Display Resolution */
        char buf[64];
        uint32_t px = print_string(44 * FONT_SCALE, 120 * FONT_SCALE, "Framebuffer: ", 0x0094A3B8, 0);
        utoa(fb->Width, buf, 10);
        px = print_string(px, 120 * FONT_SCALE, buf, 0x00F8FAFC, 0);
        px = print_string(px, 120 * FONT_SCALE, " x ", 0x0094A3B8, 0);
        utoa(fb->Height, buf, 10);
        print_string(px, 120 * FONT_SCALE, buf, 0x00F8FAFC, 0);

        px = print_string(44 * FONT_SCALE, 140 * FONT_SCALE, "Scanline:    ", 0x0094A3B8, 0);
        utoa(fb->PixelsPerScanLine, buf, 10);
        px = print_string(px, 140 * FONT_SCALE, buf, 0x00F8FAFC, 0);
        print_string(px, 140 * FONT_SCALE, " pixels", 0x0094A3B8, 0);

        /* Parse UEFI Memory Map */
        uint64_t total_usable_bytes = 0;
        uint64_t total_descriptors = 0;

        if (boot_info->MemoryMap && boot_info->DescriptorSize > 0)
        {
            uint8_t *map_ptr = (uint8_t *)boot_info->MemoryMap;
            uint64_t count = boot_info->MapSize / boot_info->DescriptorSize;
            total_descriptors = count;

            for (uint64_t i = 0; i < count; i++)
            {
                NthMemoryDescriptor *desc = (NthMemoryDescriptor *)(map_ptr + i * boot_info->DescriptorSize);
                if (desc->Type == NthEfiConventionalMemory)
                {
                    total_usable_bytes += desc->NumberOfPages * 4096;
                }
            }
        }

        px = print_string(44 * FONT_SCALE, 170 * FONT_SCALE, "Memory Map:  ", 0x0094A3B8, 0);
        utoa(total_descriptors, buf, 10);
        px = print_string(px, 170 * FONT_SCALE, buf, 0x00F8FAFC, 0);
        print_string(px, 170 * FONT_SCALE, " descriptors", 0x0094A3B8, 0);

        px = print_string(44 * FONT_SCALE, 190 * FONT_SCALE, "Usable RAM:  ", 0x0094A3B8, 0);
        utoa(total_usable_bytes / (1024 * 1024), buf, 10);
        px = print_string(px, 190 * FONT_SCALE, buf, 0x0034D399, 0); // Green accent
        print_string(px, 190 * FONT_SCALE, " MiB", 0x0094A3B8, 0);

        /* ACPI RSDP Pointer */
        px = print_string(44 * FONT_SCALE, 220 * FONT_SCALE, "ACPI RSDP:   0x", 0x0094A3B8, 0);
        if (boot_info->Rsdp)
        {
            utoa((uint64_t)boot_info->Rsdp, buf, 16);
            print_string(px, 220 * FONT_SCALE, buf, 0x00F8FAFC, 0);
        }
        else
        {
            print_string(px, 220 * FONT_SCALE, "Not Found", 0x00EF4444, 0);
        }

        /* Status message */
        print_string(44 * FONT_SCALE, 260 * FONT_SCALE, "Status: Kernel booted successfully via nth protocol!", 0x0038BDF8, 0);
        print_string(44 * FONT_SCALE, 280 * FONT_SCALE, "Ready for kernel development. CPU halted.", 0x0064748B, 0);
    }

    /* Infinite halt loop */
    while (1)
    {
        __asm__ volatile("hlt");
    }
}
