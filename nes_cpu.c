#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <time.h>

#include <string.h>
#include <errno.h>

#include "interface.h"
#include "nes_cpu.h"

size_t file_size;

/* init NES cpu internals */
int nes_init_cpu()
{
    nes_cpu_registers.A = 0x00;
    nes_cpu_registers.X = 0x00;
    nes_cpu_registers.Y = 0x00;
    nes_cpu_registers.S = 0x24;
    
    nes_cpu_registers.SP = 0xFD;
    nes_cpu_registers.PC = 0xFFFC;

    nes_cpu_bus.AB = 0x0000;
    nes_cpu_bus.DB = 0x0000;

    return 0;
}

/* For debug purposes only, print contents of registers */
void print_nes_cpu_trace(uint8_t opcode)
{
    printf("%04X ", nes_cpu_registers.PC);
    print_opcode(opcode);
    printf("%s\t", addr_mode_str[nes_2A02_cpu_opcode_map[opcode].AM]);
    printf("A:%02X", nes_cpu_registers.A);
    printf(" X:%02X", nes_cpu_registers.X);
    printf(" Y:%02X", nes_cpu_registers.Y);
    printf(" P:%02X",  nes_cpu_registers.S);
    printf(" SP:%02X", nes_cpu_registers.SP);
    printf(" ADDR:%04X", nes_cpu_bus.AB);
    printf(" DB:%02X ", nes_cpu_bus.DB);
    printf(" N:%d", get_flag(N));
    printf(" V:%d", get_flag(V));
    printf(" U:%d", get_flag(U));
    printf(" B:%d", get_flag(B));
    printf(" D:%d", get_flag(D));
    printf(" I:%d", get_flag(I));
    printf(" Z:%d", get_flag(Z));
    printf(" C:%d\n", get_flag(C));
}

/* 
Function to load ROM of NES game 

iNES format (information from https://wiki.nesdev.com/w/index.php/INES):
-----
An iNES file consists of the following sections, in order:

Header (16 bytes)
Trainer, if present (0 or 512 bytes)
PRG ROM data (16384 * x bytes)
CHR ROM data, if present (8192 * y bytes)
PlayChoice INST-ROM, if present (0 or 8192 bytes)
PlayChoice PROM, if present (16 bytes Data, 16 bytes CounterOut) (this is often missing, see PC10 ROM-Images for details)
Some ROM-Images additionally contain a 128-byte (or sometimes 127-byte) title at the end of the file.

The format of the header is as follows:

0-3: Constant $4E $45 $53 $1A ("NES" followed by MS-DOS end-of-file)
4: Size of PRG ROM in 16 KB units
5: Size of CHR ROM in 8 KB units (Value 0 means the board uses CHR RAM)
6: Flags 6 - Mapper, mirroring, battery, trainer
7: Flags 7 - Mapper, VS/Playchoice, NES 2.0
8: Flags 8 - PRG-RAM size (rarely used extension)
9: Flags 9 - TV system (rarely used extension)
10: Flags 10 - TV system, PRG-RAM presence (unofficial, rarely used extension)
11-15: Unused padding (should be filled with zero, but some rippers put their name across bytes 7-15)

NES 2.0 Format (similar to iNES, information from https://wiki.nesdev.com/w/index.php/NES_2.0):
-----

*/
int nes_load_rom(const char * filename, _nes_cartridge * cart)
{
    /* Flags to check file format, string for telling which format */
    bool iNES = false, NES_20 = false;
    const char * format;

    cart->PRG_ROM_size = 0,
    cart->CHR_ROM_size = 0;

    /* Mapper ID */
    uint8_t mapper_ID = 0;

    /* Debug info to measure # of bytes copied */
    size_t bytes_copied = 0;

    /* If the file pointer is empty, something went wrong with opening it. */
    FILE * rom;
    rom = fopen(filename, "rb");
    if (rom == NULL) 
    {
        fprintf(stderr, "error: failed to open %s for writing: %s\n", filename, strerror(errno));
        return -1;
    }
    else
    {
        printf("Successfully opened rom %s!\n", filename);
    }

    /* Get the size of the rom */
    fseek(rom, 0, SEEK_END);
    file_size = ftell(rom);
    rewind(rom);

    /* Buffer for the header of the ROM */
    uint8_t * header = malloc(16 * sizeof(uint8_t));
    if(!fread(header, sizeof(uint8_t), 16, rom))
    {
        fprintf(stderr, "error: Failed to copy ROM header! exiting");
        return -1;
    }

    /* Set cartridge address space to the NES address space */
    cart->nes_mem = nes_cpu_mem.mem;

    /* Check if iNES or NES 2.0 format, shameful copy/paste from https://wiki.nesdev.com/w/index.php/NES_2.0 */
    if ((uint32_t)(header[0] << 24 | header[1] << 16 | header[2] << 8 | header[3]) == 0x4E45531A)
    {
        if( (header[7] & 0xC) == 0x8 )
        {
            NES_20 = true;
            format = "NES 2.0";
        }
        else
        {
            iNES = true;
            format = "iNES";

            /* Check if last 4 bits of flags 10 are not equal to 0, see https://wiki.nesdev.com/w/index.php/INES#Flags_10*/
            if(header[10] & 0xF0 != 0)
            {
                fprintf(stderr, "error: Higher 4 bits of Flags 10 not equal to 0. Not loading ROM.\n");
                return -1;
            }

            /* Get sizes */
            cart->PRG_ROM_size = header[4] * 0x4000; /* Size in 16 KiB units */
            cart->CHR_ROM_size = header[5] * 0x2000; /* Size in  8 KiB units */

            //PRG_RAM_size = (header[8] > 0) ? (header[8] * 0x2000) : 0x2000; /* Size in 8 KiB units */
        
            /* Get mapper number */
            mapper_ID = (uint8_t)(header[7] & 0xF0) | ((header[6] & 0xF0) >> 4);

            /* Flags 6 and 7 combined into a single byte for your viewing pleasure B-) */
            uint8_t flags = (uint8_t)(header[7] & 0x0F) << 4 | ((header[6] & 0x0F));

            /* TO-DO: check other flags in 6 & 7 */

            /* Check for trainer, load it into address space $7000 */
            if (flags & 0x4)
                fread(&nes_cpu_mem.mem[0x7000], sizeof(uint8_t), 0x200, rom);

            /* Load rom according to which mapper is being used */
            if(mapper_ID > 0)
            {
                fprintf(stderr, "error: Unimplemented mapper number %d. exiting.\n", mapper_ID);
                return -1;
            }
            else
            {
                /* Call the mapper based on the ID */
                (*mapper[mapper_ID])(rom);
            }
            
            /* TO-DO: load CHR-ROM/RAM into memory and map memory accordingly. */
        }

        printf("Format:\t%s\n", format);
    }
    else
    {
        fprintf(stderr, "error: Unknown format! Exiting\n");
        return -1;
    }

    /* Finally, clear the file pointer and return 0 */
    fclose(rom);

    nes_cpu_registers.PC = (uint16_t)PEEK(nes_cpu_registers.PC + 1) << 8 | PEEK(nes_cpu_registers.PC);
    //nes_cpu_registers.PC = 0x8000;
    return 0;
}

typedef union _PPU_pix
{
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
}
_PPU_pix;

/* PPU pattern table dump for debug purposes */
static inline void PPU_pattern_table_dump(Display * disp, bool page)
{
    const uint32_t gray_scale_pix[] = {
        0xFF000000,
        0xFF444444,
        0xFFCCCCCC,
        0xFFFFFFFF
    };

    uint32_t pix_seq[8];

    _PPU_pix pix_row;

    for (size_t i = 0; i < 128; i++)
    {
        for (size_t j = 0; j < 16; j++)
        {
            uint8_t t_row = (i >> 3);
            uint8_t hi = nes_ppu.PPU_Pattern_bytes[page][(t_row << 8) | (j << 4) + 8 + (i & 0x7)],
                    lo = nes_ppu.PPU_Pattern_bytes[page][(t_row << 8) | (j << 4) + (i & 0x7)];
            
            pix_row.row = conv_to_pix_row(hi, lo);

            pix_seq[0] = gray_scale_pix[pix_row.pix0];
            pix_seq[1] = gray_scale_pix[pix_row.pix1];
            pix_seq[2] = gray_scale_pix[pix_row.pix2];
            pix_seq[3] = gray_scale_pix[pix_row.pix3];
            pix_seq[4] = gray_scale_pix[pix_row.pix4];
            pix_seq[5] = gray_scale_pix[pix_row.pix5];
            pix_seq[6] = gray_scale_pix[pix_row.pix6];
            pix_seq[7] = gray_scale_pix[pix_row.pix7];

            write_ARGB8888_arr_to_display(disp, (j*8), i, pix_seq, 8, 1);
        }
    }
}

/* PPU pallete table dump for debug purposes */

void PPU_pallete_table_dump(Display * disp)
{
    uint32_t    bg_seq[128],
                fg_seq[128];

    /* Print background pallete */
    for (size_t i = 0; i < 16; i++)
    {
        write_ARGB8888_pixel_to_display(disp, (i * 4),      247, NES_palette[nes_ppu.PPU_Pallete_Data[0][i]]);
        write_ARGB8888_pixel_to_display(disp, (i * 4 + 1),  247, NES_palette[nes_ppu.PPU_Pallete_Data[0][i]]);
        write_ARGB8888_pixel_to_display(disp, (i * 4 + 2),  247, NES_palette[nes_ppu.PPU_Pallete_Data[0][i]]);
        write_ARGB8888_pixel_to_display(disp, (i * 4 + 3),  247, NES_palette[nes_ppu.PPU_Pallete_Data[0][i]]);   
    }

    /* Print foreground pallete */
    for (size_t i = 0; i < 16; i++)
    {
        write_ARGB8888_pixel_to_display(disp, 128 + (i * 4),      247, NES_palette[nes_ppu.PPU_Pallete_Data[1][i]]);
        write_ARGB8888_pixel_to_display(disp, 128 + (i * 4 + 1),  247, NES_palette[nes_ppu.PPU_Pallete_Data[1][i]]);
        write_ARGB8888_pixel_to_display(disp, 128 + (i * 4 + 2),  247, NES_palette[nes_ppu.PPU_Pallete_Data[1][i]]);
        write_ARGB8888_pixel_to_display(disp, 128 + (i * 4 + 3),  247, NES_palette[nes_ppu.PPU_Pallete_Data[1][i]]);
    }    
}

/* NES screen */
/*
void PPU_write(Display * disp)
{
    for (size_t i = 0; i < 241; i++)
        write_ARGB8888_arr_to_display(disp, 0, i, &(nes_ppu.screen_buffer[]))
}
*/

/* Finally, the "meat and potatoes" of the emulator, the interpreter! */
void interpret(Display * disp, Display * PPU_debug)
{
    uint8_t opcode, no_of_breaks = 0; 
    int exit_code = 0;
    while( exit_code == 0 )
    {
        /* Fetch opcode from memory */
        opcode = PEEK(nes_cpu_registers.PC);
        
        get_operand_AM(nes_2A02_cpu_opcode_map[opcode].AM);
        print_nes_cpu_trace(opcode);

        /* Decode and execute the opcode */
        switch (opcode)
        {
            case BRK_IMP:   
                BRK();
                nes_cpu_registers.Cycles = 7;
                break;
            case ORA_INDX:  
                ORA();
                nes_cpu_registers.Cycles = 6;
                break;
            case ORA_ZP:    
                ORA(); 
                nes_cpu_registers.Cycles = 3; 
                break;
            case ASL_ZP:    
                ASL(); 
                nes_cpu_registers.Cycles = 5; 
                break;
            case PHP_IMP:   
                PHP();
                nes_cpu_registers.Cycles = 3; 
                break;
            case ORA_IMM:   
                ORA(); 
                nes_cpu_registers.Cycles = 2; 
                break;
            case ASL_ACC:   
                ASL();
                nes_cpu_registers.A = nes_cpu_bus.DB;
                 
                nes_cpu_registers.Cycles = 2; 
                break;
            case ORA_ABS:   
                ORA(); 
                nes_cpu_registers.Cycles = 4; 
                break;
            case ASL_ABS:   
                ASL();
                nes_cpu_registers.Cycles = 6; 
                break;
            case BPL_REL:   
                BPL();
                nes_cpu_registers.Cycles = 2; 
                break;
            case ORA_INDY:  
                ORA();
                nes_cpu_registers.Cycles += 5; 
                break;
            case ORA_ZPX:   
                ORA();
                nes_cpu_registers.Cycles = 4; 
                break;
            case ASL_ZPX:   
                ASL();
                nes_cpu_registers.Cycles = 6; 
                break;
            case CLC_IMP:   
                CLC();
                nes_cpu_registers.Cycles = 2; 
                break;
            case ORA_ABSY:  
                ORA();
                nes_cpu_registers.Cycles += 4; 
                break;
            case ORA_ABSX:  
                ORA();
                nes_cpu_registers.Cycles += 4; 
                break;
            case ASL_ABSX:  
                ASL();
                nes_cpu_registers.Cycles = 7; 
                break;
            case JSR_ABS:   
                JSR();
                nes_cpu_registers.Cycles = 6; 
                break;
            case AND_INDX:  
                AND();
                nes_cpu_registers.Cycles = 6; 
                break;
            case BIT_ZP:    
                BIT();
                nes_cpu_registers.Cycles = 3; 
                break;
            case AND_ZP:    
                AND();
                nes_cpu_registers.Cycles = 3; 
                break;
            case ROL_ZP:    
                ROL();
                nes_cpu_registers.Cycles = 5; 
                break;
            case PLP_IMP:   
                PLP();
                nes_cpu_registers.Cycles = 4; 
                break;
            case AND_IMM:   
                AND();
                nes_cpu_registers.Cycles = 2; 
                break;
            case ROL_ACC:   
                ROL(); 
                nes_cpu_registers.A = nes_cpu_bus.DB;
                 
                nes_cpu_registers.Cycles = 2; 
                break;
            case BIT_ABS:   
                BIT();
                nes_cpu_registers.Cycles = 4; 
                break;
            case AND_ABS:   
                AND();
                nes_cpu_registers.Cycles = 4; 
                break;
            case ROL_ABS:   
                ROL();
                nes_cpu_registers.Cycles = 6; 
                break;
            case BMI_REL:   
                BMI();
                nes_cpu_registers.Cycles = 2; 
                break;
            case AND_INDY:  
                AND();
                nes_cpu_registers.Cycles += 5; 
                break;
            case AND_ZPX:   
                AND();
                nes_cpu_registers.Cycles = 4; 
                break;
            case ROL_ZPX:   
                ROL();
                nes_cpu_registers.Cycles = 6; 
                break;
            case SEC_IMP:   
                SEC();
                nes_cpu_registers.Cycles = 2; 
                break;
            case AND_ABSY:  
                AND();
                nes_cpu_registers.Cycles += 4; 
                break;
            case AND_ABSX:  
                AND();
                nes_cpu_registers.Cycles += 4; 
                break;
            case ROL_ABSX:  
                ROL();
                nes_cpu_registers.Cycles = 7; 
                break;
            case RTI_IMP:   
                RTI();
                nes_cpu_registers.Cycles = 6;
                break;
            case EOR_INDX:  
                EOR();
                nes_cpu_registers.Cycles = 6; 
                break;
            case EOR_ZP:    
                EOR();
                nes_cpu_registers.Cycles = 3; 
                break;
            case LSR_ZP:    
                LSR();
                nes_cpu_registers.Cycles = 5; 
                break;
            case PHA_IMP:  
                PHA();
                nes_cpu_registers.Cycles = 3;
                break;
            case EOR_IMM:   
                EOR();
                nes_cpu_registers.Cycles = 2; 
                break;
            case LSR_ACC:   
                LSR(); 
                nes_cpu_registers.A = nes_cpu_bus.DB;
                 
                nes_cpu_registers.Cycles = 2; 
                break;
            case JMP_ABS:   
                JMP();
                nes_cpu_registers.Cycles = 3; 
                break;
            case EOR_ABS:   
                EOR();
                nes_cpu_registers.Cycles = 4; 
                break;
            case LSR_ABS:   
                LSR();
                nes_cpu_registers.Cycles = 6; 
                break;
            case BVC_REL:   
                BVC();
                nes_cpu_registers.Cycles += 2; 
                break;
            case EOR_INDY:  
                EOR();
                nes_cpu_registers.Cycles += 5; 
                break;
            case EOR_ZPX:   
                EOR();
                nes_cpu_registers.Cycles = 4; 
                break;
            case LSR_ZPX:   
                LSR();
                nes_cpu_registers.Cycles = 6; 
                break;
            case CLI_IMP:   
                CLI();
                nes_cpu_registers.Cycles = 2;
                break;
            case EOR_ABSY:  
                EOR();
                nes_cpu_registers.Cycles += 4; 
                break;
            case EOR_ABSX:  
                EOR();
                nes_cpu_registers.Cycles += 4; 
                break;
            case LSR_ABSX:  
                LSR();
                nes_cpu_registers.Cycles = 7; 
                break;
            case RTS_IMP:   
                RTS();
                nes_cpu_registers.Cycles = 6;
                break;
            case ADC_INDX:  
                ADC();
                nes_cpu_registers.Cycles = 6; 
                break;
            case ADC_ZP:    
                ADC();
                nes_cpu_registers.Cycles = 3; 
                break;
            case ROR_ZP:    
                ROR();
                nes_cpu_registers.Cycles = 5; 
                break;
            case PLA_IMP:   
                PLA();
                nes_cpu_registers.Cycles = 4;
                break;
            case ADC_IMM:   
                ADC();
                nes_cpu_registers.Cycles = 2; 
                break;
            case ROR_ACC:   
                ROR(); 
                nes_cpu_registers.A = nes_cpu_bus.DB;
                 
                nes_cpu_registers.Cycles = 2; 
                break;
            case JMP_IND: 
                JMP();
                nes_cpu_registers.Cycles = 5; 
                break;
            case ADC_ABS:   
                ADC();
                nes_cpu_registers.Cycles = 4; 
                break;
            case ROR_ABS:   
                ROR();
                nes_cpu_registers.Cycles = 6; 
                break;
            case BVS_REL:   
                BVS();
                nes_cpu_registers.Cycles += 2; 
                break;
            case ADC_INDY:  
                ADC();
                nes_cpu_registers.Cycles += 5; 
                break;
            case ADC_ZPX:   
                ADC();
                nes_cpu_registers.Cycles = 4; 
                break;
            case ROR_ZPX:   
                ROR();
                nes_cpu_registers.Cycles = 6; 
                break;
            case SEI_IMP:   
                SEI();
                nes_cpu_registers.Cycles = 2;
                break;
            case ADC_ABSY:  
                ADC();
                nes_cpu_registers.Cycles += 4; 
                break;
            case ADC_ABSX:  
                ADC();
                nes_cpu_registers.Cycles += 4; 
                break;
            case ROR_ABSX:  
                ROR();
                nes_cpu_registers.Cycles = 7; 
                break;
            case STA_INDX:  
                STA();
                nes_cpu_registers.Cycles = 6; 
                break;
            case STY_ZP:    
                STY();
                nes_cpu_registers.Cycles = 3; 
                break;
            case STA_ZP:    
                STA();
                nes_cpu_registers.Cycles = 3; 
                break;
            case STX_ZP:    
                STX();
                nes_cpu_registers.Cycles = 3; 
                break;
            case DEY_IMP:   
                DEY();
                nes_cpu_registers.Cycles = 2;
                break;
            case TXA_IMP:   
                TXA();
                nes_cpu_registers.Cycles = 2;
                break;
            case STY_ABS:   
                STY();
                nes_cpu_registers.Cycles = 4; 
                break;
            case STA_ABS:   
                STA();
                nes_cpu_registers.Cycles = 4; 
                break;
            case STX_ABS:   
                STX();
                nes_cpu_registers.Cycles = 4; 
                break;
            case BCC_REL:   
                BCC();
                nes_cpu_registers.Cycles = 2; 
                break;
            case STA_INDY:  
                STA();
                nes_cpu_registers.Cycles = 6; 
                break;
            case STY_ZPX:   
                STY();
                nes_cpu_registers.Cycles = 4; 
                break;
            case STA_ZPX:   
                STA();
                nes_cpu_registers.Cycles = 4; 
                break;
            case STX_ZPY:   
                STX();
                nes_cpu_registers.Cycles = 4; 
                break;
            case TYA_IMP:   
                TYA();
                nes_cpu_registers.Cycles = 2;
                break;
            case STA_ABSY:  
                STA();
                nes_cpu_registers.Cycles = 5; 
                break;
            case TXS_IMP:   
                TXS();
                nes_cpu_registers.Cycles = 2;
                break;
            case STA_ABSX:  
                STA();
                nes_cpu_registers.Cycles = 5; 
                break;
            case LDY_IMM:   
                 
                LDY();
                nes_cpu_registers.Cycles = 2; 
                break;
            case LDA_INDX:  
                 
                LDA();
                nes_cpu_registers.Cycles = 6; 
                break;
            case LDX_IMM:   
                 
                LDX();
                nes_cpu_registers.Cycles = 2; 
                break;
            case LDY_ZP:    
                 
                LDY();
                nes_cpu_registers.Cycles = 3; 
                break;
            case LDA_ZP:    
                 
                LDA();
                nes_cpu_registers.Cycles = 3; 
                break;
            case LDX_ZP:    
                 
                LDX();
                nes_cpu_registers.Cycles = 3; 
                break;
            case TAY_IMP:   
                
                TAY();
                nes_cpu_registers.Cycles = 2;
                break;
            case LDA_IMM:   
                 
                LDA();
                nes_cpu_registers.Cycles = 2; 
                break;
            case TAX_IMP:   
                
                TAX();
                nes_cpu_registers.Cycles = 2; 
                break;
            case LDY_ABS:   
                 
                LDY();
                nes_cpu_registers.Cycles = 4; 
                break;
            case LDA_ABS:   
                 
                LDA();
                nes_cpu_registers.Cycles = 4; 
                break;
            case LDX_ABS:   
                 
                LDX();
                nes_cpu_registers.Cycles = 4; 
                break;
            case BCS_REL:   
                 
                BCS();
                nes_cpu_registers.Cycles = 2; 
                break;
            case LDA_INDY:  
                 
                LDA();
                nes_cpu_registers.Cycles += 5; 
                break;
            case LDY_ZPX:   
                 
                LDY();
                nes_cpu_registers.Cycles = 4; 
                break;
            case LDA_ZPX:   
                 
                LDA();
                nes_cpu_registers.Cycles = 4; 
                break;
            case LDX_ZPY:   
                 
                LDX();
                nes_cpu_registers.Cycles = 4; 
                break;
            case CLV_IMP:   
                 
                CLV();
                nes_cpu_registers.Cycles = 2; 
                break;
            case LDA_ABSY:  
                 
                LDA();
                nes_cpu_registers.Cycles += 4; 
                break;
            case TSX_IMP:   
                TSX();
                nes_cpu_registers.Cycles = 2;
                break;
            case LDY_ABSX:  
                 
                LDY();
                nes_cpu_registers.Cycles += 4; 
                break;
            case LDA_ABSX:  
                 
                LDA();
                nes_cpu_registers.Cycles += 4; 
                break;
            case LDX_ABSY:  
                 
                LDX();
                nes_cpu_registers.Cycles += 4; 
                break;
            case CPY_IMM:   
                 
                CPY();
                nes_cpu_registers.Cycles = 2; 
                break;
            case CMP_INDX:  
                CMP();
                nes_cpu_registers.Cycles = 6; 
                break;
            case CPY_ZP:    
                 
                CPY();
                nes_cpu_registers.Cycles = 3; 
                break;
            case CMP_ZP:    
                 
                CMP();
                nes_cpu_registers.Cycles = 3; 
                break;
            case DEC_ZP:    
                 
                DEC();
                nes_cpu_registers.Cycles = 5; 
                break;
            case INY_IMP:   
                 
                INY();
                nes_cpu_registers.Cycles = 2; 
                break;
            case CMP_IMM:   
                 
                CMP();
                nes_cpu_registers.Cycles = 2; 
                break;
            case DEX_IMP:   
                 
                DEX();
                nes_cpu_registers.Cycles = 2; 
                break;
            case CPY_ABS:   
                 
                CPY();
                nes_cpu_registers.Cycles = 4; 
                break;
            case CMP_ABS:   
                 
                CMP();
                nes_cpu_registers.Cycles = 4; 
                break;
            case DEC_ABS:   
                DEC();
                nes_cpu_registers.Cycles = 6; 
                break;
            case BNE_REL:   
                BNE();
                nes_cpu_registers.Cycles = 2; 
                break;
            case CMP_INDY:  
                CMP();
                nes_cpu_registers.Cycles += 5; 
                break;
            case CMP_ZPX:   
                CMP();
                nes_cpu_registers.Cycles = 4; 
                break;
            case DEC_ZPX:   
                DEC();
                nes_cpu_registers.Cycles = 6; 
                break;
            case CLD_IMP:   
                CLD();
                nes_cpu_registers.Cycles = 2; 
                break;
            case CMP_ABSY:  
                CMP();
                nes_cpu_registers.Cycles += 4; 
                break;
            case CMP_ABSX:  
                CMP();
                nes_cpu_registers.Cycles += 4; 
                break;
            case DEC_ABSX:  
                DEC();
                nes_cpu_registers.Cycles = 7; 
                break;
            case CPX_IMM:   
                CPX();
                nes_cpu_registers.Cycles = 2; 
                break;
            case SBC_INDX:  
                SBC();
                nes_cpu_registers.Cycles = 6; 
                break;
            case CPX_ZP:    
                CPX();
                nes_cpu_registers.Cycles = 3; 
                break;
            case SBC_ZP:    
                SBC();
                nes_cpu_registers.Cycles = 3; 
                break;
            case INC_ZP:    
                INC();
                nes_cpu_registers.Cycles = 5; 
                break;
            case INX_IMP:   
                INX();
                nes_cpu_registers.Cycles = 2; 
                break;
            case SBC_IMM:   
                SBC();
                nes_cpu_registers.Cycles = 2; 
                break;
            case NOP_IMP:   
                NOP();
                nes_cpu_registers.Cycles = 2; 
                break;
            case CPX_ABS:   
                CPX();
                nes_cpu_registers.Cycles = 4; 
                break;
            case SBC_ABS:   
                SBC();
                nes_cpu_registers.Cycles = 4; 
                break;
            case INC_ABS:   
                INC();
                nes_cpu_registers.Cycles = 6; 
                break;
            case BEQ_REL:   
                BEQ();
                nes_cpu_registers.Cycles = 2; 
                break;
            case SBC_INDY:  
                SBC();
                nes_cpu_registers.Cycles += 5; 
                break;
            case SBC_ZPX:   
                SBC();
                nes_cpu_registers.Cycles = 4; 
                break;
            case INC_ZPX:   
                INC();
                nes_cpu_registers.Cycles = 6; 
                break;
            case SED_IMP:   
                SED();
                nes_cpu_registers.Cycles = 2;
                break;
            case SBC_ABSY:  
                SBC();
                nes_cpu_registers.Cycles += 4; 
                break;
            case SBC_ABSX:  
                SBC();
                nes_cpu_registers.Cycles += 4; 
                break;
            case INC_ABSX:   
                INC();
                nes_cpu_registers.Cycles = 7; 
                break;
            default:
                fprintf(stderr, "error: unknown opcode 0x%02X\n", opcode);
        }
        
        /* Increment the program counter accordingly */
        nes_cpu_registers.PC += PC_offset;
        
        /* The clock of the emulator, for timing purposes */
        tick();
    
        
        /* Check if time to update display */
        if ( nes_ppu.s == 240 && nes_ppu.c == 0 )
        {
            for (size_t i = 0; i < 240; i++)
                write_ARGB8888_arr_to_display(disp, 0, i, &nes_ppu.screen_buffer[(i * 360) + 1], 256, 1);
            
            push_to_display(disp);
        }

        on_event(&exit_code);

        //PPU_pattern_table_dump(PPU_debug, 0);   /* Debug functions to display pattern + pallete table data of PPU */
        //PPU_pallete_table_dump(PPU_debug);
        //push_to_display(PPU_debug);

        update_display(disp);
        //update_display(PPU_debug);

        if ( Break_and_die )  {  return;  }
    }
}

/* Driver code */
int main(int argc, char** argv)
{
    /* TO-DO: 
        2. add checks for page boundary crossing
        3. add option to dump NES memory 
    */

    /* Zero out registers, init CPU, init PPU */
    nes_init_cpu();
    nes_init_ppu();

    /* Check if only one argument after file name */    
    if (argc != 2) 
    {
        fprintf(stderr, "error: Invalid usage. USAGE:\n./nes_cpu [FILE]\n");
        return -1;
    }
    else
    {
        /* Load the rom into NES memory */  
        if (nes_load_rom(argv[1], &nes_cartridge) != 0) 
        {
            return -1;
        }
    }

    /* Create a new display */
    Display nes_window;
    //Display PPU_debug;
    init_display(&nes_window, argv[1], 256, 240);
    //create_display(&PPU_debug, "PPU pattern table dump", 256, 256);

    /* Init opcode table */
    nes_2A02_init_map();

    /* Begin interpreter */
    interpret(&nes_window, NULL);

    free_display(&nes_window);
    //free_display(&PPU_debug);
    
    SDL_Quit();

    print_zp();
}