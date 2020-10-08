#pragma once
#include "nes_ppu.h"

/* Clock information here */
const float NES_MASTER_CLOCK    = 1 / (21.47727273f * 100000),  //21.477... MHz to seconds
            NES_PPU_CLOCK       = (NES_MASTER_CLOCK * 4),       // Divide MSC by 4 and 12 (in this case, multiply to increase duration in seconds)
            NES_CPU_CLOCK       = (NES_MASTER_CLOCK * 12);

/* NES Cartridge data */
typedef struct _nes_cartridge
{
    size_t  CHR_ROM_size,
            PRG_ROM_size;
    
    /* Pointer to NES address space */
    uint8_t * nes_mem;
}
_nes_cartridge;
_nes_cartridge nes_cartridge;

/* Function pointers to select method of memory access */
uint8_t (*PEEK_MAPPER)(uint16_t);
void    (*POKE_MAPPER)(uint16_t, uint8_t);

/* OAMDMA (copy from CPU address space to OAM from $XX00 - $XXFF) */
void EXEC_OAMDMA(uint8_t oam_copy_addr_hb);

/* Mapper 000 PEEK */
uint8_t PEEK_000(uint16_t addr)
{
    /* Internal NES memory */
    if (addr >= 0x0 && addr < 0x2000)
        return nes_cartridge.nes_mem[(addr & 0x07FF)];
    /* PPU Registers */
    if (addr >= 0x2000 && addr < 0x4000)
    {
        USE_REGS((addr & 0x7), 0, 0x0);
        return nes_ppu.PPU_registers[(addr & 0x7)];
    }
    /* Mirror if PRG_ROM is only 16 KiB */
    if (addr >= 0x8000)
    {
        if (nes_cartridge.PRG_ROM_size == 0x4000)
            return nes_cartridge.nes_mem[(addr & 0x3FFF) + 0x8000];
        else
            return nes_cartridge.nes_mem[addr];
    }
}

/* Mapper 000 POKE */
void POKE_000(uint16_t addr, uint8_t data)
{
    /* Internal NES memory */
    if (addr >= 0x0 && addr < 0x2000)
        nes_cartridge.nes_mem[(addr & 0x07FF)] = data;
    /* PPU Registers */
    else if (addr >= 0x2000 && addr < 0x4000)
        USE_REGS((addr & 0x7), 1, data);
    else if (addr == 0x4014)
        EXEC_OAMDMA(data);
    /* Mirror if PRG_ROM is only 16 KiB */
    else if (addr >= 0x8000)
    {
        if (nes_cartridge.PRG_ROM_size == 0x4000)
            nes_cartridge.nes_mem[(addr & 0x3FFF) + 0x8000] = data;
        else
            nes_cartridge.nes_mem[addr] = data;
    }
}

/* OAMDMA (copy from CPU address space to OAM from $XX00 - $XXFF) */
void EXEC_OAMDMA(uint8_t oam_copy_addr_hb)
{
    uint16_t oam_copy_addr = (uint16_t) oam_copy_addr_hb << 8;
    memcpy(nes_ppu.PPU_OAM_Bytes, &nes_cartridge.nes_mem[oam_copy_addr], 256);
}

/* Mapper 000 */
void mapper_000(FILE * rom)
{
    /* Set PEEK and POKE functions respectively */
    PEEK_MAPPER = PEEK_000;
    POKE_MAPPER = POKE_000;

    /* Program ROM, loaded in CPU bus in the range $8000-$FFFF */
    if (fread(&nes_cartridge.nes_mem[0x8000], sizeof(uint8_t), nes_cartridge.PRG_ROM_size, rom) != nes_cartridge.PRG_ROM_size)
    {
        fprintf(stderr, "error: Failed to copy PRG-ROM: %s. exiting\n", strerror(errno));
        return;
    }

    /* Character ROM, loaded in PPU bus */
    if (fread(nes_ppu_bus.mem, sizeof(uint8_t), nes_cartridge.CHR_ROM_size, rom) != nes_cartridge.CHR_ROM_size)
    {
        fprintf(stderr, "error: Failed to copy CHR-ROM: %s. exiting\n", strerror(errno));
        return;
    }

    printf("Successfully mapped memory (mapper_000)!\n");
}

/* Default NULL mapper */
void mapper_NULL(FILE * rom)
{
    return;
}

/* Function pointer to select correct mapper */
void (*mapper[256])(FILE*) = {
    mapper_000,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL,
    mapper_NULL
};

/* Tick function (1 CPU cycle = 3 PPU cycles) */
static inline void tick()
{
    PPU_tick();
    PPU_tick();
    PPU_tick();
    //APU_tick();
}