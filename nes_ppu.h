#pragma once

/*
    nes_ppu.h: Header-only implementation of the PPU found inside of the NES

    Technical references:
    http://www.thealmightyguru.com/Games/Hacking/Wiki/index.php/NES_Palette
    http://nesdev.com/2C02%20technical%20reference.TXT
    https://wiki.nesdev.com/w/index.php/PPU_programmer_reference
*/

/* 
    Standard color pallete of the NES, encoded as ARGB8888 32-bit hex values 0x00RRGGBB

    From NES Hacker wiki (http://www.thealmightyguru.com/Games/Hacking/Wiki/index.php/NES_Palette): 
    "The Nintendo wasn't sophisticated enough to use the new color systems of today. No alpha 
    channels or 24-bit color to work with, no 8-bit palette, not even RGB settings. On the NES you 
    can work with a total of 64 pre-set colors (56 of which are unique), and you can only show 25 
    on the screen at one time (under normal circumstances)."

*/
const uint32_t NES_palette[64] = {
    0xFF7C7C7C,
    0xFF0000FC,
    0xFF0000BC,
    0xFF4428BC,
    0xFF940084,
    0xFFA80020,
    0xFFA81000,
    0xFF881400,
    0xFF503000,
    0xFF007800,
    0xFF006800,
    0xFF005800,
    0xFF004058,
    0xFF000000,
    0xFF000000,
    0xFF000000,
    
    0xFFBCBCBC,
    0xFF0078F8,
    0xFF0058F8,
    0xFF6844FC,
    0xFFD800CC,
    0xFFE40058,
    0xFFF83800,
    0xFFE45C10,
    0xFFAC7C00,
    0xFF00B800,
    0xFF00A800,
    0xFF00A844,
    0xFF008888,
    0xFF000000,
    0xFF000000,
    0xFF000000,

    0xFFF8F8F8,
    0xFF3CBCFC,
    0xFF6888FC,
    0xFF9878F8,
    0xFFF878F8,
    0xFFF85898,
    0xFFF87858,
    0xFFFCA044,
    0xFFF8B800,
    0xFFB8F818,
    0xFF58D854,
    0xFF58F898,
    0xFF00E8D8,
    0xFF787878,
    0xFF000000,
    0xFF000000,
    
    0xFFFCFCFC,
    0xFFA4E4FC,
    0xFFB8B8F8,
    0xFFD8B8F8,
    0xFFF8B8F8,
    0xFFF8A4C0,
    0xFFF0D0B0,
    0xFFFCE0A8,
    0xFFF8D878,
    0xFFD8F878,
    0xFFB8F8B8,
    0xFFB8F8D8,
    0xFF00FCFC,
    0xFFF8D8F8,
    0xFF000000,
    0xFF000000
};

/*
Enum of PPU registers (as an index)

Common Name	| Address |	Bits      | Notes
------------+---------+-----------+------------
PPUCTRL	    | $2000	  | VPHB SINN | NMI enable (V), PPU master/slave (P), sprite height (H), background tile select (B), sprite tile select (S), increment mode (I), nametable select (NN)
PPUMASK	    | $2001	  | BGRs bMmG | color emphasis (BGR), sprite enable (s), background enable (b), sprite left column enable (M), background left column enable (m), greyscale (G)
PPUSTATUS	| $2002	  | VSO- ---- | vblank (V), sprite 0 hit (S), sprite overflow (O); read resets write pair for $2005/$2006
OAMADDR	    | $2003	  | aaaa aaaa | OAM read/write address
OAMDATA	    | $2004	  | dddd dddd | OAM data read/write
PPUSCROLL	| $2005	  | xxxx xxxx | fine scroll position (two writes: X scroll, Y scroll)
PPUADDR	    | $2006	  | aaaa aaaa | PPU read/write address (two writes: most significant byte, least significant byte)
PPUDATA	    | $2007	  | dddd dddd | PPU data read/write
OAMDMA	    | $4014	  | aaaa aaaa | OAM DMA high address
*/
typedef enum PPU_REGS
{
    PPUCTRL     = 0x0,
    PPUMASK     = 0x1,
    PPUSTATUS   = 0x2,
    OAMADDR     = 0x3,
    OAMDATA     = 0x4,
    PPUSCROLL   = 0x5,
    PPUADDR     = 0x6,
    PPUDATA     = 0x7,
    OAMDMA      = 0x8
}
PPU_REGS;

/* 
PPU implementation 

Nametable:          Describes the layout of the background tiles, is dynamic
Pattern table:      Contains the shape for each background tile 
Attribute table:    Describes the pallete for each 2x2 tile (8x8 pixel) group on the screen
*/
typedef struct _nes_ppu
{
    /* Internal memory */

    uint8_t * PPU_Nametable[4];             /* Pointers to the 4 nametables */
    uint8_t * PPU_Attribtable[4];           /* Pointers to the 4 attribute tables ($40 in size) */    
    uint8_t * PPU_Pallete_Data[2];             /* Pointer to PPU pallete data (0 -> BG, 1-> FG) */
    
    union 
    {
        uint8_t     * PPU_Pattern_bytes[2]; /* Access via row data (2*8 = 16 bits) or byte stream */
        uint16_t    * PPU_Pattern_row[2];
    };

    uint32_t    screen_buffer[340 * 260];   /* All of the on-screen buffer, only visible portion is drawn in SDL */
    uint16_t    s, c, v, h;                 /* Scanlines and cycles, Vertical/Horizontal Indices for the screen */

    uint8_t PPU_registers[9];               /* Registers of the PPU */
    uint8_t PPU_Status;                     /* Status register of the PPU */
    bool    clear_vblank,                   /* Flag to set/clear vblank bit on next tick */
            set_scroll_addr_latch,          /* Flags to set address latch for PPUSCROLL and PPUADDR */
            set_PPU_addr_latch;

    /* Background */

    uint16_t    PPU_current_vram_addr,      /* Current VRAM address (15-bits) */
                PPU_temp_vram_addr;         /* Temporary VRAM address (15-bits) */
    uint8_t     PPU_fine_x;                 /* Fine x scroll (3 bits) */
    bool        PPU_first_sec_w;            /* First or second write toggle (1-bit) */

    union 
    {
        uint16_t    PPU_bg_u16_s[8];        /* 2 16-bit shift registers - These contain the pattern table data for 
                                            two tiles. Every 8 cycles, the data for the next tile is loaded into the 
                                            upper 8 bits of this shift register. Meanwhile, the pixel to render is 
                                            fetched from one of the lower 8 bits. */
        uint8_t     PPU_bg_u8_s[8];
    };

    /* Foreground */

    /* OAM access stuff, ala https://wiki.nesdev.com/w/index.php/PPU_programmer_reference#OAM */
    struct //OAM
    {
        union
        {
            uint32_t    PPU_OAM_Pixel[64];      /* OAM data (access as 32-bit pixel or byte stream like a normal person) */
            uint8_t     PPU_OAM_Bytes[256];
        };
        uint8_t OAM_addr;                       /* Current OAM address */

        union                                   /* Current selected pixel */
        {
            uint32_t    Current_Pixel;
            struct      {uint8_t X_pos, Attrib, Tile_no, Y_pos;};
        };

        union
        {
            uint32_t    PPU_SecOAM_Pixel[8];    /* OAM data (access as 32-bit pixel or byte stream like a normal person) */
            uint8_t     PPU_SecOAM_Bytes[32];
        };
    };

    uint8_t PPU_fg_s[8];                        /* 8 pairs of 8-bit shift regs to hold sprite data from the pattern table, 
                                                to be rendered on the current scanline*/
    uint8_t PPU_fg_attrib_l[8];                 /* Latches to hold attribute bytes for up to 8 sprites */
    uint8_t PPU_fg_hpos_c[8];                   /* Horizontal positions for up to 8 sprites */

    bool    pre_render_scanline_set;
}
_nes_ppu;
_nes_ppu nes_ppu;

/* 
NES PPU bus (from https://wiki.nesdev.com/w/index.php/PPU_memory_map)

Address range   Size    Device

$0000-$0FFF 	$1000 	Pattern table 0
$1000-$1FFF 	$1000 	Pattern table 1
$2000-$23FF 	$0400 	Nametable 0
$2400-$27FF 	$0400 	Nametable 1
$2800-$2BFF 	$0400 	Nametable 2
$2C00-$2FFF 	$0400 	Nametable 3
$3000-$3EFF 	$0F00 	Mirrors of $2000-$2EFF
$3F00-$3F1F 	$0020 	Palette RAM indexes ($3F00-$3F0F is background pallete, 
                                             $3F10-$3F1F is foreground pallete)
$3F20-$3FFF 	$00E0 	Mirrors of $3F00-$3F1F
*/
typedef struct _nes_ppu_bus
{
    uint8_t mem[0x4000];
    /* 
    To save on pins, the lower 8 pins of AB were multiplexed with the DB, not so with this emulator.
    The lower 8 bits of the Address bus are stored somewhere before the data bus is written to, so
    the multiplexing ion this case is unnecessary
    */
    uint16_t    AB;         /* Address, data bus */ 
    uint8_t     DB;
    bool        RW;         /* Flags to indicate read or write */
}
_nes_ppu_bus;
_nes_ppu_bus nes_ppu_bus;

/* An easier way of accessing pixel data (Optional) */

/* Stores current tile information */
typedef struct _ppu_tile
{
    uint8_t nt_byte, at_byte, pt_lo, pt_hi, t_row;
    uint8_t pal_seq[4];

    union{
        uint16_t row;
        struct 
        {
            uint8_t pix7 : 2;
            uint8_t pix6 : 2;
            uint8_t pix5 : 2;
            uint8_t pix4 : 2;
            uint8_t pix3 : 2;
            uint8_t pix2 : 2;
            uint8_t pix1 : 2;
            uint8_t pix0 : 2;
        };
    };
}
_ppu_tile;
_ppu_tile current_tile;

/* Read from PPU memory */
static inline uint8_t PPU_PEEK(uint16_t addr)
{
    /* Name table mirrors */
    if (addr >= 0x2000 && addr < 0x3F00)
        return nes_ppu_bus.mem[(addr & 0x0EFF) + 0x2000];
    /* Pallete mirrors */
    if (addr >= 0x3F00 && addr < 0x4000)
        return nes_ppu_bus.mem[(addr & 0x1F) + 0x3F00];
    return nes_ppu_bus.mem[addr];    
}

/* Write to PPU memory */
static inline void PPU_POKE(uint16_t addr, uint8_t data)
{
    /* Name table mirrors */
    if (addr >= 0x2000 && addr < 0x3F00)
        nes_ppu_bus.mem[(addr & 0x0EFF) + 0x2000] = data;
    /* Pallete mirrors */
    else if (addr >= 0x3F00 && addr < 0x4000)
        nes_ppu_bus.mem[(addr & 0x1F) + 0x3F00] = data;
    else
        nes_ppu_bus.mem[addr] = data;
}

/* All PPU reg operations */
static inline void EXEC_PPUCTRL     (void);
static inline void EXEC_PPUMASK     (void);
static inline void EXEC_PPUSTATUS   (void);
static inline void EXEC_OAMADDR     (void);
static inline void EXEC_OAMDATA     (void);
static inline void EXEC_PPUSCROLL   (void);
static inline void EXEC_PPUADDR     (void);
static inline void EXEC_PPUDATA     (void);

/* Function pointer to execute PPU reg operations */
void (*REG_EXEC[8])(void) = {
    EXEC_PPUCTRL,
    EXEC_PPUMASK,
    EXEC_PPUSTATUS,
    EXEC_OAMADDR,
    EXEC_OAMDATA,
    EXEC_PPUSCROLL,
    EXEC_PPUADDR,
    EXEC_PPUDATA
};

/* 
    Function to successfully use PPU registers within range $2000 to $2007 

    RW = 0 -> Read mode
    RW = 1 -> Write mode

    Function list is encoded as a string, meaning if the index of the string
    matches an 'x', the function pointer will execute the function 
    corresponding to the index, and if '_' then do nothing

    "__x_x__x" is all functions that read and "xx_xxxxx" is all functions 
    that write
*/
static inline void USE_REGS(PPU_REGS reg, bool RW, uint8_t data)
{
    nes_ppu_bus.DB == data;
    const char * function_list = (RW == 0) ? "__x_x__x" : "xx_xxxxx";

    if(function_list[reg] == 'x') 
        (*REG_EXEC[reg])();
}

/* Init the PPU */
static inline void nes_init_ppu()
{
    /* Set pointers to each nametable */
    nes_ppu.PPU_Nametable[0] = &nes_ppu_bus.mem[0x2000];
    nes_ppu.PPU_Nametable[1] = &nes_ppu_bus.mem[0x2400];
    nes_ppu.PPU_Nametable[2] = &nes_ppu_bus.mem[0x2800];
    nes_ppu.PPU_Nametable[3] = &nes_ppu_bus.mem[0x2C00];

    /* Set pointers to each attribute tables */
    nes_ppu.PPU_Attribtable[0] = &nes_ppu_bus.mem[0x23C0];
    nes_ppu.PPU_Attribtable[1] = &nes_ppu_bus.mem[0x27C0];
    nes_ppu.PPU_Attribtable[2] = &nes_ppu_bus.mem[0x2BC0];
    nes_ppu.PPU_Attribtable[3] = &nes_ppu_bus.mem[0x2FC0];

    /* Set pointers to pattern tables */
    nes_ppu.PPU_Pattern_bytes[0] = &nes_ppu_bus.mem[0x0000];
    nes_ppu.PPU_Pattern_bytes[1] = &nes_ppu_bus.mem[0x1000];

    /* Set pointers to BG/FG pallete indexes */
    nes_ppu.PPU_Pallete_Data[0] = &nes_ppu_bus.mem[0x3F00];
    nes_ppu.PPU_Pallete_Data[1] = &nes_ppu_bus.mem[0x3F10];

    /* Set status register */
    nes_ppu.PPU_Status = 0xA0;

    /* Set pre-render scanline flag to true*/
    nes_ppu.pre_render_scanline_set = true;

    /* Finally, set indices accordingly */
    nes_ppu.v = 0;
    nes_ppu.h = 0;
    nes_ppu.s = -1;
    nes_ppu.c = 0;
}

/*
PPUCTRL: PPU Control Register (write)

7  bit  0
---- ----
VPHB SINN
|||| ||||
|||| ||++- Base nametable address
|||| ||    (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00)
|||| |+--- VRAM address increment per CPU read/write of PPUDATA
|||| |     (0: add 1, going across; 1: add 32, going down)
|||| +---- Sprite pattern table address for 8x8 sprites
||||       (0: $0000; 1: $1000; ignored in 8x16 mode)
|||+------ Background pattern table address (0: $0000; 1: $1000)
||+------- Sprite size (0: 8x8 pixels; 1: 8x16 pixels)
|+-------- PPU master/slave select
|          (0: read backdrop from EXT pins; 1: output color on EXT pins)
+--------- Generate an NMI at the start of the
           vertical blanking interval (0: off; 1: on)

*/
static inline void EXEC_PPUCTRL()
{
    nes_ppu.PPU_registers[PPUCTRL] = nes_ppu_bus.DB;
}

/*
PPUMASK: PPU mask register (write)

7  bit  0
---- ----
BGRs bMmG
|||| ||||
|||| |||+- Greyscale (0: normal color, 1: produce a greyscale display)
|||| ||+-- 1: Show background in leftmost 8 pixels of screen, 0: Hide
|||| |+--- 1: Show sprites in leftmost 8 pixels of screen, 0: Hide
|||| +---- 1: Show background
|||+------ 1: Show sprites
||+------- Emphasize red
|+-------- Emphasize green
+--------- Emphasize blue
*/
static inline void EXEC_PPUMASK()
{
    nes_ppu.PPU_registers[PPUMASK] = nes_ppu_bus.DB;
}

/* */ 
static inline void EXEC_PPUSTATUS()
{
    nes_ppu.clear_vblank        = 1;   /* Clear vblank bit on next tick */
    nes_ppu.PPU_fine_x          = 0x0; /* Clear address latches and flags */
    nes_ppu_bus.AB              = 0x0;
    //nes_ppu.set_addr_latch      = false;
    nes_ppu.set_PPU_addr_latch  = false;
}

static inline void EXEC_OAMADDR()
{
    nes_ppu.OAM_addr = nes_ppu_bus.DB;
}

static inline void EXEC_OAMDATA()
{
    if (nes_ppu_bus.RW == 1)
    {
        nes_ppu.PPU_OAM_Bytes[nes_ppu.OAM_addr] = nes_ppu_bus.DB;
        
    }
    else
    {
        nes_ppu_bus.DB = nes_ppu.PPU_OAM_Bytes[nes_ppu.OAM_addr];
    }
}

static inline void EXEC_PPUSCROLL()
{
    if (nes_ppu.set_scroll_addr_latch == false)
    {
        nes_ppu.PPU_current_vram_addr   = (uint16_t) nes_ppu_bus.DB << 8;
        nes_ppu.set_scroll_addr_latch   = true;
    }
    else
    {
        nes_ppu.PPU_fine_x |= nes_ppu_bus.DB;
    }
    //nes_ppu.PPU_registers[PPUSCROLL] = nes_ppu_bus.DB;
}

static inline void EXEC_PPUADDR()
{
    if (nes_ppu.set_PPU_addr_latch == false)
    {
        nes_ppu_bus.AB                  = (uint16_t) nes_ppu_bus.DB << 8;
        nes_ppu.set_PPU_addr_latch      = true;
    }
    else
    {
        nes_ppu_bus.AB |= nes_ppu_bus.DB;
    }
    nes_ppu.PPU_registers[PPUSCROLL] = nes_ppu_bus.DB;
}

static inline void EXEC_PPUDATA()
{
    if (nes_ppu_bus.RW == 1)
        PPU_POKE(nes_ppu_bus.AB, nes_ppu_bus.DB);
    else
        nes_ppu_bus.DB = PPU_PEEK(nes_ppu_bus.AB);
}

/* 
PPU tick

From https://wiki.nesdev.com/w/index.php/PPU_rendering:

"The PPU renders 262 scanlines per frame. Each scanline lasts for 341 PPU clock 
cycles (113.667 CPU clock cycles; 1 CPU cycle = 3 PPU cycles)""

From Bisqwit, Source: https://bisqwit.iki.fi/jutut/kuvat/programming_examples/nesemu1/nesemu1.cc

         x=0                 x=256      x=340
     ___|____________________|__________|
y=-1    | pre-render scanline| prepare  | >
     ___|____________________| sprites _| > Graphics
y=0     | visible area       | for the  | > processing
        | - this is rendered | next     | > scanlines
y=239   |   on the screen.   | scanline | >
     ___|____________________|______
y=240   | idle
     ___|_______________________________
y=241   | vertical blanking (idle)
        | 20 scanlines long
y=260___|____________________|__________|
nes emulator
Each cycle on the 2A02 (NES CPU) is about 3 PPU cycles
*/

static inline void PPU_plot_row(uint16_t x, uint16_t y, uint32_t * data); /* Plot pixel row */

/* Return nametable byte */
static inline uint8_t get_nametable_byte(uint8_t i)
{
    uint8_t nt_x = nes_ppu.h >> 3,
            nt_y = nes_ppu.v >> 3;

    return nes_ppu.PPU_Nametable[(i & 0x3)][(nt_y * 0x1E + nt_x) & 0x3FF];
}

/* Return attribute table byte */
static inline uint8_t get_attrib_table_byte(uint8_t i)
{
    uint8_t at_x = nes_ppu.h >> 4,
            at_y = nes_ppu.v >> 4;

    return nes_ppu.PPU_Attribtable[(i & 0x3)][(at_y * 0x8 + at_x) & 0x3F];
}

/* 
    Decode the pixel row by mapping the attribute byte with its corresponding entry
    in the pallete table and the pixel value (0-3) to its corresponding color

    Attribute entry:

            |       
            |       
       TL   |   TR    
            |       
    --------+--------  <- How the byte is structured, correesponds with a 2x2 tile block in the pattern table
            |       
            |       
       BL   |   BR    
            |       
*/
static inline void decode_pixel_row(uint8_t attr_byte)
{
    /* Get pallete table entry for top left/right and bottom left/right quadrants */
    uint8_t TL = (attr_byte & 0x03),
            TR = (attr_byte & 0x0C) >> 2,
            BL = (attr_byte & 0x30) >> 4,
            BR = (attr_byte & 0xC0) >> 6;
    
    /* 
    See which tile quadrant we're in, set the pallete table index to the corresponding value in the quadrant.
    ( 0 , 0 ) or (0x0) -> Top-left
    ( 0 , 1 ) or (0x1) -> Bottom-left
    ( 1 , 0 ) or (0x2) -> Top-right
    ( 1 , 1 ) or (0x3) -> Bottom-right
    */
    uint8_t q_xy = ((nes_ppu.h - 1) & 0x08) >> 2 | ((nes_ppu.v - 1) & 0x08) >> 3;
    uint32_t decoded_pixels[8];

    uint8_t pat_index = 0x00;
    switch (q_xy)
    {
        case 0x0: pat_index = TL; break;
        case 0x1: pat_index = BL; break;
        case 0x2: pat_index = TR; break;
        case 0x3: pat_index = BR; break;
    }

    /* Begin mapping of pixel colors */
    decoded_pixels[0] = NES_palette[(pat_index * 4) + current_tile.pix0];
    decoded_pixels[1] = NES_palette[(pat_index * 4) + current_tile.pix1];
    decoded_pixels[2] = NES_palette[(pat_index * 4) + current_tile.pix2];
    decoded_pixels[3] = NES_palette[(pat_index * 4) + current_tile.pix3];
    decoded_pixels[4] = NES_palette[(pat_index * 4) + current_tile.pix4];
    decoded_pixels[5] = NES_palette[(pat_index * 4) + current_tile.pix5];
    decoded_pixels[6] = NES_palette[(pat_index * 4) + current_tile.pix6];
    decoded_pixels[7] = NES_palette[(pat_index * 4) + current_tile.pix7];
    
    PPU_plot_row(nes_ppu.h, nes_ppu.v, (uint32_t *)decoded_pixels);
}

/* Convert the high and lo byte of the pixel row to the corresponding pixel value (0-3) */
static inline uint16_t conv_to_pix_row(uint8_t h, uint8_t l)
{
    return ((uint16_t) (((h & 0x80) << 8)  |
                        ((h & 0x40) << 7)  |
                        ((h & 0x20) << 6)  |
                        ((h & 0x10) << 5)  |
                        ((h & 0x08) << 4)  |
                        ((h & 0x04) << 3)  |
                        ((h & 0x02) << 2)  |
                        ((h & 0x01) << 1)) |
            (uint16_t) (((l & 0x80) << 7)  |
                        ((l & 0x40) << 6)  |
                        ((l & 0x20) << 5)  |
                        ((l & 0x10) << 4)  |
                        ((l & 0x08) << 3)  |
                        ((l & 0x04) << 2)  |
                        ((l & 0x02) << 1)  |
                        ((l & 0x01))));
}

/* 
Each tick of the PPU (1 cycle's worth of data here) 

Relevant sources:
https://wiki.nesdev.com/w/index.php/PPU_rendering#Frame_timing_diagram
https://wiki.nesdev.com/w/images/d/d1/Ntsc_timing.png

*/
static inline void PPU_tick()
{
    /* Properly reset cycle counter if > 341 */
    nes_ppu.c %= 340; 

    /* Clear v-blank flag */
    if (nes_ppu.clear_vblank == true)
        nes_ppu.PPU_registers[PPUSTATUS] &= 0x7F;

    /* Actual rendering stuff */
    if (nes_ppu.s == -1 || nes_ppu.s == 261)        /* Pre-render scanline, fetch 2x tile data for the next scanline here */
        nes_ppu.pre_render_scanline_set = true;
    else if (nes_ppu.s == 240)                      /* Post-render idle scanline */
    {}    // idling
    else if (nes_ppu.s > 240)                       /* V-Blank line */
    {
        nes_ppu.PPU_registers[PPUSTATUS] |= 0x80; 
        nes_ppu.v = 0;
    }
    if (nes_ppu.c == 0)                             /* Idle cycle */
        nes_ppu.c++;
    else if (nes_ppu.c >= 1 && nes_ppu.c <= 17)     /* 2x tile data for the scanline are retrieved from the shift regs */
    {
        /* Check if we're in the first or second tile fetch */
        uint8_t i = ((nes_ppu.c - 1) & 0xF) >> 3; 

        if (nes_ppu.pre_render_scanline_set == false && nes_ppu.s < 240)
        {
            switch ((nes_ppu.c - 1) & 0x7)
            {
                /* NOTE: i need to fix this union to only give the lower 8 bits when accessing PPU_bg_u8_s, this line of code doesn't make much sense otherwise */
                case 1: current_tile.nt_byte = (uint8_t)nes_ppu.PPU_bg_u16_s[(i * 3) + 0]; break;   /* Fetch */
                case 3: current_tile.at_byte = (uint8_t)nes_ppu.PPU_bg_u16_s[(i * 3) + 1]; break;   /* Fetch */
                case 5: current_tile.t_row = (nes_ppu.v & 0x7);                     
                        current_tile.pt_lo = (nes_ppu.PPU_bg_u16_s[(i * 3) + 2] & 0xFF00) >> 4;
                        break;                                                                      /* Get row index and lo byte for the tile */
                case 7: current_tile.pt_hi = (nes_ppu.PPU_bg_u16_s[(i * 3) + 2] & 0x00FF);
                        current_tile.row = conv_to_pix_row(current_tile.pt_lo, current_tile.pt_hi);
                        decode_pixel_row(current_tile.at_byte);
                        nes_ppu.h += 8;
                        break;                                                                      /* Get hi byte for the tile and render to screen */
            }
        }
    }
    else if (nes_ppu.c >= 257 && nes_ppu.c <= 320)  /* Where the first two tiles for the next sprite scanline are stored */
    {
        
    }
    else if (nes_ppu.c >= 321 && nes_ppu.c <= 336)  /* Where the first two tiles for the next scanline are stored */
    {
        /* Check if we're in the first or second tile fetch */
        uint8_t i = ((nes_ppu.c - 1) & 0xF) >> 3; 
        
        /* 
        Fetch nametable address                 (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00), 
        and background pattern table address    (0: $0000; 1: $1000)
        */
        uint8_t nt_i = (nes_ppu.PPU_registers[PPUCTRL] & 0x03),
                pt_i = (nes_ppu.PPU_registers[PPUCTRL] & 0x10) >> 4;

        switch ((nes_ppu.c - 1) & 0x7)
        {
            case 1: nes_ppu.PPU_bg_u16_s[(i * 3) + 0] = get_nametable_byte(nt_i);    break; /* Fetch */
            case 3: nes_ppu.PPU_bg_u16_s[(i * 3) + 1] = get_attrib_table_byte(0);    break; /* Fetch */
            case 5: current_tile.t_row = (nes_ppu.v & 0x7);                     
                    nes_ppu.PPU_bg_u16_s[(i * 3) + 2] = nes_ppu.PPU_Pattern_row[pt_i][(uint16_t)(((current_tile.nt_byte << 4) + current_tile.t_row) >> 1) & 0x1FF];
                    break;                                                                  /* Get row index and lo byte for the tile */
            case 7: nes_ppu.h += 8;
                    break;                                                                  /* Get hi byte for the tile and render to screen */
        }
    }
    else if (nes_ppu.c >= 337 && nes_ppu.c <= 340)  /* Useless */
    {
        if (nes_ppu.c == 340 && nes_ppu.pre_render_scanline_set)
        {
            nes_ppu.s = 0;
            nes_ppu.pre_render_scanline_set = false;
        }
        else
        {
            nes_ppu.s += (nes_ppu.c == 340) ? 1 : 0;
        }
    } 
    else
    {
        // Every 8th cycle, the horizontal index is updated
        if (nes_ppu.pre_render_scanline_set == false && nes_ppu.s < 240)
        {
            /* Fetch nametable address (0 = $2000; 1 = $2400; 2 = $2800; 3 = $2C00) */
            uint8_t nt_i = (nes_ppu.PPU_registers[PPUCTRL] & 0x3),
                    pt_i = (nes_ppu.PPU_registers[PPUCTRL] & 0x10) >> 4;

            switch ((nes_ppu.c - 1) & 0x7)
            {
                case 1: current_tile.nt_byte = get_nametable_byte(nt_i);    break;  /* Fetch */
                case 3: current_tile.at_byte = get_attrib_table_byte(0);    break;  /* Fetch */
                case 5: current_tile.t_row = (nes_ppu.v & 0x7);                     
                        current_tile.pt_lo = nes_ppu.PPU_Pattern_bytes[pt_i][(uint16_t)((current_tile.nt_byte << 4) + current_tile.t_row)];
                        break;                                                      /* Get row index and lo byte for the tile */
                case 7: current_tile.pt_hi = nes_ppu.PPU_Pattern_bytes[pt_i][(uint16_t)((current_tile.nt_byte << 4) + 8 + current_tile.t_row)];
                        current_tile.row = conv_to_pix_row(current_tile.pt_lo, current_tile.pt_hi);
                        decode_pixel_row(current_tile.at_byte);
                        nes_ppu.h += 8;
                        break;                                                      /* Get hi byte for the tile and render to screen */
            }
        }
    }
    nes_ppu.c++;

    /* 
    This line of code is weird but basically converting the PPU clock (~5.3693181825 MHz) to msec 
    reveals that, at the 537'th cycle, the number of elapsed msec is approximately 1 (~1.000126984 
    msec to be precise). Check if we are at that precious waiting period, then wait.
    */
    if ((nes_ppu.s * nes_ppu.c) == 537)
    {
        //wait(1);
    }
}

/* Plot pixel (unused) */
static inline void PPU_plot_pixel(uint16_t x, uint16_t y, uint32_t data)
{
    x %= 340;
    y %= 260;
    nes_ppu.screen_buffer[340 * y + x] = data;
}

/* Plot row */
static inline void PPU_plot_row(uint16_t x, uint16_t y, uint32_t * data)
{
    x %= 340;
    y %= 260;
    memcpy((void *)&nes_ppu.screen_buffer[(y * 360) + x], (void *)data, 8*sizeof(uint32_t));
}

/* Horizontal (row) fill (unused)  */
static inline void PPU_hfill(uint16_t y, uint32_t * data)
{
    y %= 260;
    memcpy((void*)&nes_ppu.screen_buffer[340 * y + 1], (void*)data, 256*sizeof(uint32_t));
}

/* Vertical (col) fill (unused) */
static inline void PPU_vfill(uint16_t x, uint32_t * data)
{
    x %= 340;
    for(size_t i = 1; i < 241; i++)
        nes_ppu.screen_buffer[340 * i + x] = data[i-1];
}
