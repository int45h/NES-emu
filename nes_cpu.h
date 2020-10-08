#pragma once

#include <stdbool.h> 
#include <stdint.h>

/* Program Counter Offset, how much to increment it by after using the appropriate addressing mode */ 
int8_t PC_offset;

/* Break flag (just give up and die when you hit a BRK) */
bool Break_and_die = false;

/* Address and Data Bus of 6502 CPU, IRQ, NMI, and RES pins */
typedef struct _6502_cpu_bus
{
    uint16_t AB;
    uint8_t DB;
    bool IRQ;
    bool NMI;
    bool RES;
}
_6502_cpu_bus;

/* 
Memory with zero page and ram access

Memory map (from https://wiki.nesdev.com/w/index.php/CPU_memory_map)

Address range   Size    Device

$0000-$07FF 	$0800 	2KB internal RAM 
$0800-$0FFF 	$0800 	
$1000-$17FF 	$0800   Mirrors of $0000-$07FF
$1800-$1FFF 	$0800
$2000-$2007 	$0008 	NES PPU registers
$2008-$3FFF 	$1FF8 	Mirrors of $2000-2007 (repeats every 8 bytes)
$4000-$4017 	$0018 	NES APU and I/O registers
$4018-$401F 	$0008 	APU and I/O functionality that is normally disabled. See CPU Test Mode.
$4020-$FFFF 	$BFE0 	Cartridge space: PRG ROM, PRG RAM, and mapper registers (See Note)

($4020-$4FFF is rarely used, $5000-$5FFF is rarely used but is used in some cartridges as bank
switching registers, $6000-$7FFF is often cartridge WRAM, $8000-$FFFF is the main cartridge 
address space.)
*/
typedef struct _6502_cpu_mem
{
    union
    {
        uint8_t mem[0x10000];
        uint8_t ram[0x800];
        uint8_t zp[0x100];
    };

    uint8_t APU_IO_Regs[24];
}
_6502_cpu_mem;

/* Registers for the 6502 */
typedef struct _6502_cpu_registers
{
    uint8_t A;
    uint8_t X, Y;
    uint8_t SP;
    uint8_t S;

    uint16_t PC;
    uint16_t Cycles;
}
_6502_cpu_registers;

/* Flags for the NES 6502 CPU, the NES 6502 lacks decimal mode */
typedef enum nes_cpu_flags
{
    N = 0x80,
    V = 0x40,
    U = 0x20,
    B = 0x10,
    D = 0x08,
    I = 0x04,
    Z = 0x02,
    C = 0x01
}
nes_cpu_flags;

/* Address modes for the NES CPU */
typedef enum nes_cpu_addr_modes
{
    ABSX,
    ABSY,
    INDX,
    INDY,
    ZPX,
    ZPY,
    ACC,
    IMM,
    ZP,
    ABS,
    REL,
    IND,
    IMP,
    NONE
}
nes_cpu_addr_modes;

/* String containing all addressing modes */
const char * addr_mode_str[14] = {
    "ABSX",
    "ABSY",
    "INDX",
    "INDY",
    "ZPX",
    "ZPY",
    "ACC",
    "IMM",
    "ZP",
    "ABS",
    "REL",
    "IND",
    "IMP",
    "NONE"
};

/* Opcodes for the NES (Incomplete) */
typedef enum nes_cpu_opcodes
{
    BRK_IMP = 0x00,     
    ORA_INDX,           
    ORA_ZP = 0x05,      
    ASL_ZP,         
    PHP_IMP = 0x08,     
    ORA_IMM,
    ASL_ACC,
    ORA_ABS = 0x0D,
    ASL_ABS,
    BPL_REL = 0x10,
    ORA_INDY,       
    ORA_ZPX = 0x15, 
    ASL_ZPX,        
    CLC_IMP = 0x18, 
    ORA_ABSY,           
    ORA_ABSX = 0x1D,
    ASL_ABSX,
    JSR_ABS = 0x20,
    AND_INDX,
    BIT_ZP = 0x24,
    AND_ZP,         
    ROL_ZP,         
    PLP_IMP = 0x28, 
    AND_IMM,        
    ROL_ACC,            
    BIT_ABS = 0x2C,
    AND_ABS,
    ROL_ABS,
    BMI_REL = 0x30,
    AND_INDY,       
    AND_ZPX = 0x35, 
    ROL_ZPX,        
    SEC_IMP = 0x38, 
    AND_ABSY,       
    AND_ABSX = 0x3D,    
    ROL_ABSX,
    RTI_IMP = 0x40,
    EOR_INDX,
    EOR_ZP = 0x45,
    LSR_ZP,
    PHA_IMP = 0x48, 
    EOR_IMM,        
    LSR_ACC,        
    JMP_ABS = 0x4C, 
    EOR_ABS,            
    LSR_ABS,
    BVC_REL = 0x50,
    EOR_INDY,
    EOR_ZPX = 0x55,
    LSR_ZPX,
    CLI_IMP = 0x58, 
    EOR_ABSY,       
    EOR_ABSX = 0x5D,
    LSR_ABSX,       
    RTS_IMP = 0x60,  
    ADC_INDX,           
    ADC_ZP = 0x65,
    ROR_ZP,
    PLA_IMP = 0x68,
    ADC_IMM,
    ROR_ACC,         
    JMP_IND = 0x6C,
    ADC_ABS,         
    ROR_ABS,         
    BVS_REL = 0x70,     
    ADC_INDY,
    ADC_ZPX = 0x75,
    ROR_ZPX,
    SEI_IMP = 0x78,
    ADC_ABSY,
    ADC_ABSX = 0x7D, 
    ROR_ABSX,        
    STA_INDX = 0x81, 
    STY_ZP = 0x84,   
    STA_ZP,             
    STX_ZP,
    DEY_IMP = 0x88,
    TXA_IMP = 0x8A,
    STY_ABS = 0x8C,
    STA_ABS,
    STX_ABS,         
    BCC_REL = 0x90,  
    STA_INDY,        
    STY_ZPX = 0x94,  
    STA_ZPX,            
    STX_ZPY,
    TYA_IMP = 0x98,
    STA_ABSY,
    TXS_IMP,
    STA_ABSX = 0x9D,
    LDY_IMM = 0xA0,  
    LDA_INDX,        
    LDX_IMM,         
    LDY_ZP = 0xA4,   
    LDA_ZP,             
    LDX_ZP,
    TAY_IMP = 0xA8,
    LDA_IMM,
    TAX_IMP,
    LDY_ABS = 0xAC,
    LDA_ABS,         
    LDX_ABS,         
    BCS_REL = 0xB0,  
    LDA_INDY,        
    LDY_ZPX = 0xB4,     
    LDA_ZPX,
    LDX_ZPY,
    CLV_IMP = 0xB8,
    LDA_ABSY,
    TSX_IMP,
    LDY_ABSX = 0xBC, 
    LDA_ABSX,        
    LDX_ABSY,        
    CPY_IMM = 0xC0,  
    CMP_INDX,       
    CPY_ZP = 0xC4,      
    CMP_ZP,         
    DEC_ZP,
    INY_IMP = 0xC8,
    CMP_IMM,
    DEX_IMP,        
    CPY_ABS = 0xCC, 
    CMP_ABS,        
    DEC_ABS,        
    BNE_REL = 0xD0,     
    CMP_INDY,
    CMP_ZPX,
    DEC_ZPX,
    CLD_IMP = 0xD8,
    CMP_ABSY,
    CMP_ABSX = 0xDD,
    DEC_ABSX,       
    CPX_IMM = 0xE0, 
    SBC_INDX,       
    CPX_ZP = 0xE4,      
    SBC_ZP,
    INC_ZP,
    INX_IMP = 0xE8,
    SBC_IMM,
    NOP_IMP,
    CPX_ABS = 0xEC, 
    SBC_ABS,        
    INC_ABS,        
    BEQ_REL = 0xF0, 
    SBC_INDY,           
    SBC_ZPX = 0xF5,
    INC_ZPX,
    SED_IMP = 0xF8,
    SBC_ABSY,
    SBC_ABSX = 0xFD,
    INC_ABSX
}
nes_cpu_opcodes;

/* Opcode map (includes opcode, addressing mode, and mnemonic) */
typedef struct _2A02_cpu_opcode_map
{
    uint8_t     AM;
    const char  * mnemonic;
}
_2A02_cpu_opcode_map;
_2A02_cpu_opcode_map nes_2A02_cpu_opcode_map[256];

/* Instantiating structs for CPU bus, memory, and registers*/
_6502_cpu_bus        nes_cpu_bus;
_6502_cpu_mem        nes_cpu_mem;
_6502_cpu_registers  nes_cpu_registers;

/* Cartridge data here UwU */
#include "nes_cartridge.h"

/* Initialize opcode map */
static inline void nes_2A02_init_map()
{
    /* init to all NULL values */
    for (size_t i = 0; i < 256; i++)
    {
        nes_2A02_cpu_opcode_map[i] = (_2A02_cpu_opcode_map){NONE, "???"};
    }

    /* Set adressing mode and mnemonic for each opcode */
    nes_2A02_cpu_opcode_map[BRK_IMP] =  (_2A02_cpu_opcode_map){.AM = IMP,     .mnemonic = "BRK"};
    nes_2A02_cpu_opcode_map[ORA_INDX] = (_2A02_cpu_opcode_map){.AM = INDX,    .mnemonic = "ORA"};
    nes_2A02_cpu_opcode_map[ORA_ZP] =   (_2A02_cpu_opcode_map){.AM = ZP,      .mnemonic = "ORA"};
    nes_2A02_cpu_opcode_map[ASL_ZP] =   (_2A02_cpu_opcode_map){.AM = ZP,      .mnemonic = "ASL"};
    nes_2A02_cpu_opcode_map[PHP_IMP] =  (_2A02_cpu_opcode_map){.AM = IMP,     .mnemonic = "PHP"};
    nes_2A02_cpu_opcode_map[ORA_IMM] =  (_2A02_cpu_opcode_map){.AM = IMM,     .mnemonic = "ORA"};
    nes_2A02_cpu_opcode_map[ASL_ACC] =  (_2A02_cpu_opcode_map){.AM = ACC,     .mnemonic = "ASL"};
    nes_2A02_cpu_opcode_map[ORA_ABS] =  (_2A02_cpu_opcode_map){.AM = ABS,     .mnemonic = "ORA"};
    nes_2A02_cpu_opcode_map[ASL_ABS] =  (_2A02_cpu_opcode_map){.AM = ABS,     .mnemonic = "ASL"};
    nes_2A02_cpu_opcode_map[BPL_REL] =  (_2A02_cpu_opcode_map){.AM = REL,     .mnemonic = "BPL"};
    nes_2A02_cpu_opcode_map[ORA_INDY] = (_2A02_cpu_opcode_map){.AM = INDY,    .mnemonic = "ORA"};
    nes_2A02_cpu_opcode_map[ORA_ZPX] =  (_2A02_cpu_opcode_map){.AM = ZPX,     .mnemonic = "ORA"};
    nes_2A02_cpu_opcode_map[ASL_ZPX] =  (_2A02_cpu_opcode_map){.AM = ZPX,     .mnemonic = "ASL"};
    nes_2A02_cpu_opcode_map[CLC_IMP] =  (_2A02_cpu_opcode_map){.AM = IMP,     .mnemonic = "CLC"};
    nes_2A02_cpu_opcode_map[ORA_ABSY] = (_2A02_cpu_opcode_map){.AM = ABSY,    .mnemonic = "ORA"};
    nes_2A02_cpu_opcode_map[ORA_ABSX] = (_2A02_cpu_opcode_map){.AM = ABSX,    .mnemonic = "ORA"};
    nes_2A02_cpu_opcode_map[ASL_ABSX] = (_2A02_cpu_opcode_map){.AM = ABSX,    .mnemonic = "ASL"};
    nes_2A02_cpu_opcode_map[JSR_ABS] =  (_2A02_cpu_opcode_map){.AM = ABS,     .mnemonic = "JSR"};
    nes_2A02_cpu_opcode_map[AND_INDX] = (_2A02_cpu_opcode_map){.AM = INDX,    .mnemonic = "AND"};
    nes_2A02_cpu_opcode_map[BIT_ZP] =   (_2A02_cpu_opcode_map){.AM = ZP,      .mnemonic = "BIT"};
    nes_2A02_cpu_opcode_map[AND_ZP] =   (_2A02_cpu_opcode_map){.AM = ZP,      .mnemonic = "AND"};
    nes_2A02_cpu_opcode_map[ROL_ZP] =   (_2A02_cpu_opcode_map){.AM = ZP,      .mnemonic = "ROL"};
    nes_2A02_cpu_opcode_map[PLP_IMP] =  (_2A02_cpu_opcode_map){.AM = IMP,     .mnemonic = "PLP"};
    nes_2A02_cpu_opcode_map[AND_IMM] =  (_2A02_cpu_opcode_map){.AM = IMM,     .mnemonic = "AND"};
    nes_2A02_cpu_opcode_map[ROL_ACC] =  (_2A02_cpu_opcode_map){.AM = ACC,     .mnemonic = "ROL"};
    nes_2A02_cpu_opcode_map[BIT_ABS] =  (_2A02_cpu_opcode_map){.AM = ABS,     .mnemonic = "BIT"};
    nes_2A02_cpu_opcode_map[AND_ABS] =  (_2A02_cpu_opcode_map){.AM = ABS,     .mnemonic = "AND"};
    nes_2A02_cpu_opcode_map[ROL_ABS] =  (_2A02_cpu_opcode_map){.AM = ABS,     .mnemonic = "ROL"};
    nes_2A02_cpu_opcode_map[BMI_REL] =  (_2A02_cpu_opcode_map){.AM = REL,     .mnemonic = "BMI"};
    nes_2A02_cpu_opcode_map[AND_INDY] = (_2A02_cpu_opcode_map){.AM = INDY,    .mnemonic = "AND"};
    nes_2A02_cpu_opcode_map[AND_ZPX] =  (_2A02_cpu_opcode_map){.AM = ZPX,     .mnemonic = "AND"};
    nes_2A02_cpu_opcode_map[ROL_ZPX] =  (_2A02_cpu_opcode_map){.AM = ZPX,     .mnemonic = "ROL"};
    nes_2A02_cpu_opcode_map[SEC_IMP] =  (_2A02_cpu_opcode_map){.AM = IMP,     .mnemonic = "SEC"};
    nes_2A02_cpu_opcode_map[AND_ABSY] = (_2A02_cpu_opcode_map){.AM = ABSY,    .mnemonic = "AND"};
    nes_2A02_cpu_opcode_map[AND_ABSX] = (_2A02_cpu_opcode_map){.AM = ABSX,    .mnemonic = "AND"};
    nes_2A02_cpu_opcode_map[ROL_ABSX] = (_2A02_cpu_opcode_map){.AM = ABSX,    .mnemonic = "ROL"};
    nes_2A02_cpu_opcode_map[RTI_IMP] =  (_2A02_cpu_opcode_map){.AM = IMP,     .mnemonic = "RTI"};
    nes_2A02_cpu_opcode_map[EOR_INDX] = (_2A02_cpu_opcode_map){.AM = INDX,    .mnemonic = "EOR"};
    nes_2A02_cpu_opcode_map[EOR_ZP] =   (_2A02_cpu_opcode_map){.AM = ZP,      .mnemonic = "EOR"};
    nes_2A02_cpu_opcode_map[LSR_ZP] =   (_2A02_cpu_opcode_map){.AM = ZP,      .mnemonic = "LSR"};
    nes_2A02_cpu_opcode_map[PHA_IMP] =  (_2A02_cpu_opcode_map){.AM = IMP,     .mnemonic = "PHA"};
    nes_2A02_cpu_opcode_map[EOR_IMM] =  (_2A02_cpu_opcode_map){.AM = IMM,     .mnemonic = "EOR"};
    nes_2A02_cpu_opcode_map[LSR_ACC] =  (_2A02_cpu_opcode_map){.AM = ACC,     .mnemonic = "LSR"};
    nes_2A02_cpu_opcode_map[JMP_ABS] =  (_2A02_cpu_opcode_map){.AM = ABS,     .mnemonic = "JMP"};
    nes_2A02_cpu_opcode_map[EOR_ABS] =  (_2A02_cpu_opcode_map){.AM = ABS,     .mnemonic = "EOR"};
    nes_2A02_cpu_opcode_map[LSR_ABS] =  (_2A02_cpu_opcode_map){.AM = ABS,     .mnemonic = "LSR"};
    nes_2A02_cpu_opcode_map[BVC_REL] =  (_2A02_cpu_opcode_map){.AM = REL,     .mnemonic = "BVC"};
    nes_2A02_cpu_opcode_map[EOR_INDY] = (_2A02_cpu_opcode_map){.AM = INDY,    .mnemonic = "EOR"};
    nes_2A02_cpu_opcode_map[EOR_ZPX] =  (_2A02_cpu_opcode_map){.AM = ZPX,     .mnemonic = "EOR"};
    nes_2A02_cpu_opcode_map[LSR_ZPX] =  (_2A02_cpu_opcode_map){.AM = ZPX,     .mnemonic = "LSR"};
    nes_2A02_cpu_opcode_map[CLI_IMP] =  (_2A02_cpu_opcode_map){.AM = IMP,     .mnemonic = "CLI"};
    nes_2A02_cpu_opcode_map[EOR_ABSY] = (_2A02_cpu_opcode_map){.AM = ABSY,    .mnemonic = "EOR"};
    nes_2A02_cpu_opcode_map[EOR_ABSX] = (_2A02_cpu_opcode_map){.AM = ABSX,    .mnemonic = "EOR"};
    nes_2A02_cpu_opcode_map[LSR_ABSX] = (_2A02_cpu_opcode_map){.AM = ABSX,    .mnemonic = "LSR"};
    nes_2A02_cpu_opcode_map[RTS_IMP] =  (_2A02_cpu_opcode_map){.AM = IMP,     .mnemonic = "RTS"};
    nes_2A02_cpu_opcode_map[ADC_INDX] = (_2A02_cpu_opcode_map){.AM = INDX,    .mnemonic = "ADC"};
    nes_2A02_cpu_opcode_map[ADC_ZP] =   (_2A02_cpu_opcode_map){.AM = ZP,      .mnemonic = "ADC"};
    nes_2A02_cpu_opcode_map[ROR_ZP] =   (_2A02_cpu_opcode_map){.AM = ZP,      .mnemonic = "ROR"};
    nes_2A02_cpu_opcode_map[PLA_IMP] =  (_2A02_cpu_opcode_map){.AM = IMP,     .mnemonic = "PLA"};
    nes_2A02_cpu_opcode_map[ADC_IMM] =  (_2A02_cpu_opcode_map){.AM = IMM,     .mnemonic = "ADC"};
    nes_2A02_cpu_opcode_map[ROR_ACC] =  (_2A02_cpu_opcode_map){.AM = ACC,     .mnemonic = "ROR"};
    nes_2A02_cpu_opcode_map[JMP_IND] =  (_2A02_cpu_opcode_map){.AM = IND,     .mnemonic = "JMP"};
    nes_2A02_cpu_opcode_map[ADC_ABS] =  (_2A02_cpu_opcode_map){.AM = ABS,     .mnemonic = "ADC"};
    nes_2A02_cpu_opcode_map[ROR_ABS] =  (_2A02_cpu_opcode_map){.AM = ABS,     .mnemonic = "ROR"};
    nes_2A02_cpu_opcode_map[BVS_REL] =  (_2A02_cpu_opcode_map){.AM = REL,     .mnemonic = "BVS"};
    nes_2A02_cpu_opcode_map[ADC_INDY] = (_2A02_cpu_opcode_map){.AM = INDY,    .mnemonic = "ADC"};
    nes_2A02_cpu_opcode_map[ADC_ZPX] =  (_2A02_cpu_opcode_map){.AM = ZPX,     .mnemonic = "ADC"};
    nes_2A02_cpu_opcode_map[ROR_ZPX] =  (_2A02_cpu_opcode_map){.AM = ZPX,     .mnemonic = "ROR"};
    nes_2A02_cpu_opcode_map[SEI_IMP] =  (_2A02_cpu_opcode_map){.AM = IMP,     .mnemonic = "SEI"};
    nes_2A02_cpu_opcode_map[ADC_ABSY] = (_2A02_cpu_opcode_map){.AM = ABSY,    .mnemonic = "ADC"};
    nes_2A02_cpu_opcode_map[ADC_ABSX] = (_2A02_cpu_opcode_map){.AM = ABSX,    .mnemonic = "ADC"};
    nes_2A02_cpu_opcode_map[ROR_ABSX] = (_2A02_cpu_opcode_map){.AM = ABSX,    .mnemonic = "ROR"};
    nes_2A02_cpu_opcode_map[STA_INDX] = (_2A02_cpu_opcode_map){.AM = INDX,    .mnemonic = "STA"};
    nes_2A02_cpu_opcode_map[STY_ZP] =   (_2A02_cpu_opcode_map){.AM = ZP,      .mnemonic = "STY"};
    nes_2A02_cpu_opcode_map[STA_ZP] =   (_2A02_cpu_opcode_map){.AM = ZP,      .mnemonic = "STA"};
    nes_2A02_cpu_opcode_map[STX_ZP] =   (_2A02_cpu_opcode_map){.AM = ZP,      .mnemonic = "STX"};
    nes_2A02_cpu_opcode_map[DEY_IMP] =  (_2A02_cpu_opcode_map){.AM = IMP,     .mnemonic = "DEY"};
    nes_2A02_cpu_opcode_map[TXA_IMP] =  (_2A02_cpu_opcode_map){.AM = IMP,     .mnemonic = "TXA"};
    nes_2A02_cpu_opcode_map[STY_ABS] =  (_2A02_cpu_opcode_map){.AM = ABS,     .mnemonic = "STY"};
    nes_2A02_cpu_opcode_map[STA_ABS] =  (_2A02_cpu_opcode_map){.AM = ABS,     .mnemonic = "STA"};
    nes_2A02_cpu_opcode_map[STX_ABS] =  (_2A02_cpu_opcode_map){.AM = ABS,     .mnemonic = "STX"};
    nes_2A02_cpu_opcode_map[BCC_REL] =  (_2A02_cpu_opcode_map){.AM = REL,     .mnemonic = "BCC"};
    nes_2A02_cpu_opcode_map[STA_INDY] = (_2A02_cpu_opcode_map){.AM = INDY,    .mnemonic = "STA"};
    nes_2A02_cpu_opcode_map[STY_ZPX] =  (_2A02_cpu_opcode_map){.AM = ZPX,     .mnemonic = "STY"};
    nes_2A02_cpu_opcode_map[STA_ZPX] =  (_2A02_cpu_opcode_map){.AM = ZPX,     .mnemonic = "STA"};
    nes_2A02_cpu_opcode_map[STX_ZPY] =  (_2A02_cpu_opcode_map){.AM = ZPY,     .mnemonic = "STX"};
    nes_2A02_cpu_opcode_map[TYA_IMP] =  (_2A02_cpu_opcode_map){.AM = IMP,     .mnemonic = "TYA"};
    nes_2A02_cpu_opcode_map[STA_ABSY] = (_2A02_cpu_opcode_map){.AM = ABSY,    .mnemonic = "STA"};
    nes_2A02_cpu_opcode_map[TXS_IMP] =  (_2A02_cpu_opcode_map){.AM = IMP,     .mnemonic = "TXS"};
    nes_2A02_cpu_opcode_map[STA_ABSX] = (_2A02_cpu_opcode_map){.AM = ABSX,    .mnemonic = "STA"};
    nes_2A02_cpu_opcode_map[LDY_IMM] =  (_2A02_cpu_opcode_map){.AM = IMM,     .mnemonic = "LDY"};
    nes_2A02_cpu_opcode_map[LDA_INDX] = (_2A02_cpu_opcode_map){.AM = INDX,    .mnemonic = "LDA"};
    nes_2A02_cpu_opcode_map[LDX_IMM] =  (_2A02_cpu_opcode_map){.AM = IMM,     .mnemonic = "LDX"};
    nes_2A02_cpu_opcode_map[LDY_ZP] =   (_2A02_cpu_opcode_map){.AM = ZP,      .mnemonic = "LDY"};
    nes_2A02_cpu_opcode_map[LDA_ZP] =   (_2A02_cpu_opcode_map){.AM = ZP,      .mnemonic = "LDA"};
    nes_2A02_cpu_opcode_map[LDX_ZP] =   (_2A02_cpu_opcode_map){.AM = ZP,      .mnemonic = "LDX"};
    nes_2A02_cpu_opcode_map[TAY_IMP] =  (_2A02_cpu_opcode_map){.AM = IMP,     .mnemonic = "TAY"};
    nes_2A02_cpu_opcode_map[LDA_IMM] =  (_2A02_cpu_opcode_map){.AM = IMM,     .mnemonic = "LDA"};
    nes_2A02_cpu_opcode_map[TAX_IMP] =  (_2A02_cpu_opcode_map){.AM = IMP,     .mnemonic = "TAX"};
    nes_2A02_cpu_opcode_map[LDY_ABS] =  (_2A02_cpu_opcode_map){.AM = ABS,     .mnemonic = "LDY"};
    nes_2A02_cpu_opcode_map[LDA_ABS] =  (_2A02_cpu_opcode_map){.AM = ABS,     .mnemonic = "LDA"};
    nes_2A02_cpu_opcode_map[LDX_ABS] =  (_2A02_cpu_opcode_map){.AM = ABS,     .mnemonic = "LDX"};
    nes_2A02_cpu_opcode_map[BCS_REL] =  (_2A02_cpu_opcode_map){.AM = REL,     .mnemonic = "BCS"};
    nes_2A02_cpu_opcode_map[LDA_INDY] = (_2A02_cpu_opcode_map){.AM = INDY,    .mnemonic = "LDA"};
    nes_2A02_cpu_opcode_map[LDY_ZPX] =  (_2A02_cpu_opcode_map){.AM = ZPX,     .mnemonic = "LDY"};
    nes_2A02_cpu_opcode_map[LDA_ZPX] =  (_2A02_cpu_opcode_map){.AM = ZPX,     .mnemonic = "LDA"};
    nes_2A02_cpu_opcode_map[LDX_ZPY] =  (_2A02_cpu_opcode_map){.AM = ZPY,     .mnemonic = "LDX"};
    nes_2A02_cpu_opcode_map[CLV_IMP] =  (_2A02_cpu_opcode_map){.AM = IMP,     .mnemonic = "CLV"};
    nes_2A02_cpu_opcode_map[LDA_ABSY] = (_2A02_cpu_opcode_map){.AM = ABSY,    .mnemonic = "LDA"};
    nes_2A02_cpu_opcode_map[TSX_IMP] =  (_2A02_cpu_opcode_map){.AM = IMP,     .mnemonic = "TSX"};
    nes_2A02_cpu_opcode_map[LDY_ABSX] = (_2A02_cpu_opcode_map){.AM = ABSX,    .mnemonic = "LDY"};
    nes_2A02_cpu_opcode_map[LDA_ABSX] = (_2A02_cpu_opcode_map){.AM = ABSX,    .mnemonic = "LDA"};
    nes_2A02_cpu_opcode_map[LDX_ABSY] = (_2A02_cpu_opcode_map){.AM = ABSY,    .mnemonic = "LDX"};
    nes_2A02_cpu_opcode_map[CPY_IMM] =  (_2A02_cpu_opcode_map){.AM = IMM,     .mnemonic = "CPY"};
    nes_2A02_cpu_opcode_map[CMP_INDX] = (_2A02_cpu_opcode_map){.AM = INDX,    .mnemonic = "CMP"};
    nes_2A02_cpu_opcode_map[CPY_ZP] =   (_2A02_cpu_opcode_map){.AM = ZP,      .mnemonic = "CPY"};
    nes_2A02_cpu_opcode_map[CMP_ZP] =   (_2A02_cpu_opcode_map){.AM = ZP,      .mnemonic = "CMP"};
    nes_2A02_cpu_opcode_map[DEC_ZP] =   (_2A02_cpu_opcode_map){.AM = ZP,      .mnemonic = "DEC"};
    nes_2A02_cpu_opcode_map[INY_IMP] =  (_2A02_cpu_opcode_map){.AM = IMP,     .mnemonic = "INY"};
    nes_2A02_cpu_opcode_map[CMP_IMM] =  (_2A02_cpu_opcode_map){.AM = IMM,     .mnemonic = "CMP"};
    nes_2A02_cpu_opcode_map[DEX_IMP] =  (_2A02_cpu_opcode_map){.AM = IMP,     .mnemonic = "DEX"};
    nes_2A02_cpu_opcode_map[CPY_ABS] =  (_2A02_cpu_opcode_map){.AM = ABS,     .mnemonic = "CPY"};
    nes_2A02_cpu_opcode_map[CMP_ABS] =  (_2A02_cpu_opcode_map){.AM = ABS,     .mnemonic = "CMP"};
    nes_2A02_cpu_opcode_map[DEC_ABS] =  (_2A02_cpu_opcode_map){.AM = ABS,     .mnemonic = "DEC"};
    nes_2A02_cpu_opcode_map[BNE_REL] =  (_2A02_cpu_opcode_map){.AM = REL,     .mnemonic = "BNE"};
    nes_2A02_cpu_opcode_map[CMP_INDY] = (_2A02_cpu_opcode_map){.AM = INDY,    .mnemonic = "CMP"};
    nes_2A02_cpu_opcode_map[CMP_ZPX] =  (_2A02_cpu_opcode_map){.AM = ZPX,     .mnemonic = "CMP"};
    nes_2A02_cpu_opcode_map[DEC_ZPX] =  (_2A02_cpu_opcode_map){.AM = ZPX,     .mnemonic = "DEC"};
    nes_2A02_cpu_opcode_map[CLD_IMP] =  (_2A02_cpu_opcode_map){.AM = IMP,     .mnemonic = "CLD"};
    nes_2A02_cpu_opcode_map[CMP_ABSY] = (_2A02_cpu_opcode_map){.AM = ABSY,    .mnemonic = "CMP"};
    nes_2A02_cpu_opcode_map[CMP_ABSX] = (_2A02_cpu_opcode_map){.AM = ABSX,    .mnemonic = "CMP"};
    nes_2A02_cpu_opcode_map[DEC_ABSX] = (_2A02_cpu_opcode_map){.AM = ABSX,    .mnemonic = "DEC"};
    nes_2A02_cpu_opcode_map[CPX_IMM] =  (_2A02_cpu_opcode_map){.AM = IMM,     .mnemonic = "CPX"};
    nes_2A02_cpu_opcode_map[SBC_INDX] = (_2A02_cpu_opcode_map){.AM = INDX,    .mnemonic = "SBC"};
    nes_2A02_cpu_opcode_map[CPX_ZP] =   (_2A02_cpu_opcode_map){.AM = ZP,      .mnemonic = "CPX"};
    nes_2A02_cpu_opcode_map[SBC_ZP] =   (_2A02_cpu_opcode_map){.AM = ZP,      .mnemonic = "SBC"};
    nes_2A02_cpu_opcode_map[INC_ZP] =   (_2A02_cpu_opcode_map){.AM = ZP,      .mnemonic = "INC"};
    nes_2A02_cpu_opcode_map[INX_IMP] =  (_2A02_cpu_opcode_map){.AM = IMP,     .mnemonic = "INX"};
    nes_2A02_cpu_opcode_map[SBC_IMM] =  (_2A02_cpu_opcode_map){.AM = IMM,     .mnemonic = "SBC"};
    nes_2A02_cpu_opcode_map[NOP_IMP] =  (_2A02_cpu_opcode_map){.AM = IMP,     .mnemonic = "NOP"};
    nes_2A02_cpu_opcode_map[CPX_ABS] =  (_2A02_cpu_opcode_map){.AM = ABS,     .mnemonic = "CPX"};
    nes_2A02_cpu_opcode_map[SBC_ABS] =  (_2A02_cpu_opcode_map){.AM = ABS,     .mnemonic = "SBC"};
    nes_2A02_cpu_opcode_map[INC_ABS] =  (_2A02_cpu_opcode_map){.AM = ABS,     .mnemonic = "INC"};
    nes_2A02_cpu_opcode_map[BEQ_REL] =  (_2A02_cpu_opcode_map){.AM = REL,     .mnemonic = "BEQ"};
    nes_2A02_cpu_opcode_map[SBC_INDY] = (_2A02_cpu_opcode_map){.AM = INDY,    .mnemonic = "SBC"};
    nes_2A02_cpu_opcode_map[SBC_ZPX] =  (_2A02_cpu_opcode_map){.AM = ZPX,     .mnemonic = "SBC"};
    nes_2A02_cpu_opcode_map[INC_ZPX] =  (_2A02_cpu_opcode_map){.AM = ZPX,     .mnemonic = "INC"};
    nes_2A02_cpu_opcode_map[SED_IMP] =  (_2A02_cpu_opcode_map){.AM = IMP,     .mnemonic = "SED"};
    nes_2A02_cpu_opcode_map[SBC_ABSY] = (_2A02_cpu_opcode_map){.AM = ABSY,    .mnemonic = "SBC"};
    nes_2A02_cpu_opcode_map[SBC_ABSX] = (_2A02_cpu_opcode_map){.AM = ABSX,    .mnemonic = "SBC"};
    nes_2A02_cpu_opcode_map[INC_ABSX] = (_2A02_cpu_opcode_map){.AM = ABSX,    .mnemonic = "INC"};
}

/* Debug function to print zero page memory */
static inline void print_zp()
{
    uint8_t temp_addr = 0x0000;

    printf("\n---ZERO PAGE DUMP---\n");
    for(size_t i = 0; i <= 0xF; i++)
    {
        printf("0x%02X:\t", (i << 4));
        for(size_t j = 0; j <= 0xF; j++)
        {
            temp_addr = (uint8_t) (i << 4) | j;
            printf("%02X ", nes_cpu_mem.zp[temp_addr]);
        }
        printf("\n");
    }
}

/* Debug function to print ROM space */


/* Peek (read) byte from memory at address 'addr' */
static inline uint8_t PEEK(uint16_t addr)
{
    PEEK_MAPPER(addr);
}

/* Peek (read) byte from memory at address 'addr' */
static inline uint8_t PEEK_ZP(uint16_t addr)
{
    return nes_cpu_mem.zp[(uint8_t)(addr & 0x00FF)];
}

/* Poke (write) byte in memory at address 'addr' */
static inline void POKE(uint16_t addr, uint8_t data)
{
    POKE_MAPPER(addr, data);
}

/* Poke (write) byte in zero page at address ('addr' & 0x00FF) */
static inline void POKE_ZP(uint16_t addr, uint8_t data)
{
    nes_cpu_mem.zp[addr & 0xFF] = data;
}


/* Set 6502 flags */
static inline void test_flag(nes_cpu_flags flag, uint16_t condition)
{
    nes_cpu_registers.S = (condition > 0) ? (nes_cpu_registers.S | flag) : (nes_cpu_registers.S & (~flag));
}

/* Clear 6502 flags */
static inline void clear_flag(nes_cpu_flags flag)
{
    nes_cpu_registers.S &= ~flag;
}

/* Push value on top of stack */
static inline void PUSH(uint8_t data)
{
    nes_cpu_registers.SP--;
    POKE((nes_cpu_registers.SP + 0x100), data);
}

/* Pop top-most value off stack and return it */
static inline uint8_t POP()
{
    uint8_t val = PEEK(nes_cpu_registers.SP + 0x100);

    POKE((nes_cpu_registers.SP + 0x100), 0x00);
    nes_cpu_registers.SP++;

    return val;
}

/* Get 6502 flags */
static inline bool get_flag(nes_cpu_flags flag)
{
    switch (flag)
    {
        case N: return(nes_cpu_registers.S & flag) >> 7;
        case V: return(nes_cpu_registers.S & flag) >> 6;
        case B: return(nes_cpu_registers.S & flag) >> 4;
        case I: return(nes_cpu_registers.S & flag) >> 2;
        case Z: return(nes_cpu_registers.S & flag) >> 1;
        case C: return(nes_cpu_registers.S & flag);
        case U: return 1;
        case D: return 0;
        default: fprintf(stderr, "error: unknown flag %02X, ignoring", flag);
    }
}

/* Checks the sign bit */
#define IS_NEGATIVE(Val)        (Val & 0x80)

/* Performs twos complement on a value V */
#define TWOS_COMP(Val)          (~Val + 1)

/* Checks for overflow (can be improved) */
#define DOES_OVERFLOW(Val)      (Val > 127 | Val < -128)

/* Takes the branch */
#define TAKE_BRANCH             (PC_offset += (int8_t)nes_cpu_bus.DB)

#define CLEAR_OPERAND_STRING    memset((void*)operand, ' ', 9);

/* Current addressing mode */
uint8_t current_addr_mode = NONE;

/* String used in disassembly of rom to display operand */
char operand[9];

/* Get operand using different address modes */
static inline void get_operand_AM(nes_cpu_addr_modes mode)
{
    current_addr_mode = mode;
    CLEAR_OPERAND_STRING;
    switch (mode)
    {
        case ABS:
        {
            uint8_t hi = PEEK(nes_cpu_registers.PC + 2);
            uint8_t lo = PEEK(nes_cpu_registers.PC + 1);

            nes_cpu_bus.AB = (uint16_t) hi << 8 | lo;
            nes_cpu_bus.DB = PEEK(nes_cpu_bus.AB);
            PC_offset = 3;

            /* Set Operand string (TO-DO: fix this) */
            sprintf(operand, "$%04X   \0", nes_cpu_bus.AB);
        }
        break;
        case REL: 
        {
            int8_t offset  = PEEK(nes_cpu_registers.PC + 1);
            nes_cpu_bus.DB = offset;
            PC_offset = 2;

            /* Set Operand string (TO-DO: fix this) */
            sprintf(operand, "$%02X     \0", nes_cpu_bus.DB);
        }
        break;
        case ZP:
        {
            nes_cpu_bus.AB = PEEK(nes_cpu_registers.PC + 1);
            
            nes_cpu_bus.DB = PEEK_ZP(nes_cpu_bus.AB);
            PC_offset = 2;

            /* Set Operand string (TO-DO: fix this) */
            sprintf(operand, "$%02X     \0", (nes_cpu_bus.AB & 0xFF));
        }
        break;
        case ABSX:
        {
            uint8_t hi = PEEK(nes_cpu_registers.PC + 2);
            uint8_t lo = PEEK(nes_cpu_registers.PC + 1);

            nes_cpu_bus.AB = (uint16_t) hi << 8 | lo;
            nes_cpu_bus.DB = PEEK(nes_cpu_bus.AB + nes_cpu_registers.X);
            PC_offset = 3;

            /* Set Operand string (TO-DO: fix this) */
            sprintf(operand, "$%04X, X\0", nes_cpu_bus.AB + nes_cpu_registers.X);
        }
        break;
        case ABSY:
        {
            uint8_t hi = PEEK(nes_cpu_registers.PC + 2);
            uint8_t lo = PEEK(nes_cpu_registers.PC + 1);

            nes_cpu_bus.AB = (uint16_t) hi << 8 | lo;
            nes_cpu_bus.DB = PEEK(nes_cpu_bus.AB + nes_cpu_registers.Y);
            PC_offset = 3;

            /* Set Operand string (TO-DO: fix this) */
            sprintf(operand, "$%04X, Y\0", nes_cpu_bus.AB + nes_cpu_registers.Y);
        }
        break;
        case ZPX:
        {
            uint16_t addr = PEEK(nes_cpu_registers.PC + 1) + nes_cpu_registers.X;
            
            nes_cpu_bus.DB = PEEK_ZP(addr);
            PC_offset = 2;

            /* Set Operand string (TO-DO: fix this) */
            sprintf(operand, "$%02X, X  \0", (addr & 0xFF));
        }
        break;
        case ZPY:
        {
            uint16_t addr = PEEK(nes_cpu_registers.PC + 1) + nes_cpu_registers.Y;
            
            nes_cpu_bus.DB = PEEK_ZP(addr);
            PC_offset = 2;

            /* Set Operand string (TO-DO: fix this) */
            sprintf(operand, "$%02X, Y  \0", (addr & 0xFF));
        }
        break;
        case ACC:
            nes_cpu_bus.DB = nes_cpu_registers.A; 
            PC_offset = 1;

            operand[0] = 'A';
        break;
        case IMM: 
            nes_cpu_bus.DB = PEEK(nes_cpu_registers.PC + 1);
            PC_offset = 2;

            /* Set Operand string (TO-DO: fix this) */
            sprintf(operand, "#$%02X    \0", nes_cpu_bus.DB);
        break;
        case IND: 
        {
            uint8_t hi = PEEK(nes_cpu_registers.PC + 2),
                    lo = PEEK(nes_cpu_registers.PC + 1);

            uint16_t ind_addr = (uint16_t)hi << 8 | lo;
            hi = PEEK(ind_addr + 1);
            lo = PEEK(ind_addr);
            
            nes_cpu_bus.AB = (uint16_t)hi << 8 | lo;
            nes_cpu_bus.DB = PEEK(nes_cpu_bus.AB);
            PC_offset = 3;

            /* Set Operand string (TO-DO: fix this) */
            sprintf(operand, "($%04X) \0", ind_addr);
        }
        break;
        case INDX:
        {
            /* Add operand to X to get ZP address */
            uint8_t op = PEEK(nes_cpu_registers.PC + 1),
                    hi = PEEK_ZP(op + nes_cpu_registers.X + 1),
                    lo = PEEK_ZP(op + nes_cpu_registers.X);

            uint16_t index_addr = (uint16_t) (hi << 8) | lo;
            
            nes_cpu_bus.DB = PEEK(index_addr);
            PC_offset = 2;

            /* Set Operand string (TO-DO: fix this) */
            sprintf(operand, "($%02X), X \0", op);
        }
        break;
        case INDY:
        {
            /* Get high and low byte from zero page (using zp address from operand) and add contents of Y register to the final address. */
            uint8_t op = PEEK(nes_cpu_registers.PC + 1),
                    hi = PEEK_ZP(op + 1),
                    lo = PEEK_ZP(op);
            
            uint16_t indir_addr = ((uint16_t)hi << 8 | lo) + nes_cpu_registers.Y;
            if((indir_addr & 0xFF00) != hi)
            {
                nes_cpu_registers.Cycles += 1;
            }
            
            nes_cpu_bus.DB = PEEK(indir_addr);
            PC_offset = 2;

            /* Set Operand string (TO-DO: fix this) */
            sprintf(operand, "($%02X), Y \0", op);
        }
        break;
        case IMP: case NONE:
            PC_offset = 1;
        break;
    }
}

/* Debug function to print opcode and operand (incomplete) */
static inline void print_opcode(nes_cpu_opcodes opcode)
{
    char ins_bytes_str[10];

    memset((void *)ins_bytes_str, ' ', 9);
    ins_bytes_str[9] = '\0';

    /* Print each byte of instruction */
    switch(nes_2A02_cpu_opcode_map[opcode].AM)
    {
        case IMP: case ACC:                                                     /* Print one byte of instruction */
        sprintf(ins_bytes_str, "%02X       \0", opcode);
        break;
        case IMM: case ZP: case ZPX: case ZPY: case REL: case INDX: case INDY:  /* Print two bytes of instruction */
        sprintf(ins_bytes_str, "%02X %02X    \0", opcode, PEEK(nes_cpu_registers.PC + 1));
        break;
        case ABS: case ABSX: case ABSY: case IND:                               /* Print three bytes of instruction */
        sprintf(ins_bytes_str, "%02X %02X %02X \0", opcode, PEEK(nes_cpu_registers.PC + 1), PEEK(nes_cpu_registers.PC + 2));
        break;
    }

    printf("%s: %s  %s\t", ins_bytes_str, nes_2A02_cpu_opcode_map[opcode].mnemonic, operand);
}


/* 6502 interrupts */

/* IRQ */
static inline void IRQ()
{
    if (!get_flag(I))
    {
        uint8_t PC_hi = ((nes_cpu_registers.PC + 2) >> 8 & 0x00FF);
        uint8_t PC_lo = ((nes_cpu_registers.PC + 2) & 0x00FF); 
        PUSH(PC_hi);
        PUSH(PC_lo);
        PUSH(nes_cpu_registers.S);

        nes_cpu_registers.PC = (uint16_t)PEEK(0xFFFF) << 8 | PEEK(0xFFFE);
        test_flag(B, 1);
        test_flag(I, 1);

        nes_cpu_registers.Cycles = 7;
    }
}

/* Decrement # of cycles and wait for a period of time */
void CPU_wait()
{
    while (nes_cpu_registers.Cycles != 0)
    {
        nes_cpu_registers.Cycles -= 1;
        tick();
    }
}

/* Non-maskable interrupt */
static inline void NMI()
{
    uint8_t PC_hi = ((nes_cpu_registers.PC + 2) >> 8 & 0x00FF);
    uint8_t PC_lo = ((nes_cpu_registers.PC + 2) & 0x00FF); 
    PUSH(PC_hi);
    PUSH(PC_lo);
    PUSH(nes_cpu_registers.S);
    
    nes_cpu_registers.PC = (uint16_t)PEEK(0xFFFB) << 8 | PEEK(0xFFFA);
    test_flag(B, 1);

    nes_cpu_registers.Cycles = 8;
}

/* Reset registers */
static inline void RESET()
{
    nes_cpu_registers.PC = (uint16_t)PEEK(0xFFFD) << 8 | PEEK(0xFFFC);
    
    nes_cpu_registers.SP = 0xFF;
    nes_cpu_registers.S  = U;
    nes_cpu_registers.A  = 0x00;
    nes_cpu_registers.X  = 0x00;
    nes_cpu_registers.Y  = 0x00;
    
    nes_cpu_bus.AB = 0x00;
    nes_cpu_bus.DB = 0x00;
    
    test_flag(B, 1);
    test_flag(I, 1);

    nes_cpu_registers.Cycles = 8;
}

/* 6502 instructions (Interpreter mode only)) */

/* Add with Carry, Adds memory to Accumulator  */
static inline void ADC()
{
    uint8_t tmp_a = nes_cpu_registers.A;
    uint16_t sum = (uint16_t) (nes_cpu_bus.DB + tmp_a);
    nes_cpu_registers.A = (uint8_t) sum;

    /* 
    ADC is just Tmp_A + DB => A, check the sign bit of Tmp_A and DB with the result and if it's non-zero, set V flag.
    
    http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
    */
    bool overflow = (((nes_cpu_bus.DB ^ nes_cpu_registers.A) & (tmp_a ^ nes_cpu_registers.A) & 0x80) != 0);

    test_flag(N, IS_NEGATIVE(sum)); 
    test_flag(Z, (nes_cpu_registers.A == 0x00)); 
    test_flag(V, overflow);     
    test_flag(C, (sum > 0xFF)); 
}

/* Bitwise AND with Accumulator */
static inline void AND()
{
    nes_cpu_registers.A &= nes_cpu_bus.DB;

    test_flag(N, IS_NEGATIVE(nes_cpu_registers.A));
    test_flag(Z, (nes_cpu_registers.A == 0x00)); 
}

/* Arithmetic shift left */
static inline void ASL()
{
    test_flag(C, IS_NEGATIVE(nes_cpu_bus.DB));
    nes_cpu_bus.DB <<= 1;

    test_flag(N, IS_NEGATIVE(nes_cpu_bus.DB)); 
    test_flag(Z, (nes_cpu_bus.DB == 0x00));  
}

/* Test Bits */
static inline void BIT()
{
    /*test_flag(N, IS_NEGATIVE(nes_cpu_bus.DB));
    test_flag(V, (nes_cpu_bus.DB & 0x40 == 0x40));
    test_flag(Z, (nes_cpu_bus.DB == 0x00));*/

    test_flag(N, IS_NEGATIVE(nes_cpu_bus.DB));
    test_flag(V, (nes_cpu_bus.DB & 0x40) >> 6);
    test_flag(Z, !(nes_cpu_bus.DB & nes_cpu_registers.A));
}

/* Branch on Carry Clear */
static inline void BCC()
{
    if ( !get_flag(C) ) { TAKE_BRANCH; }
}

/* Branch on Carry Set */
static inline void BCS()
{
    if ( get_flag(C) ) { TAKE_BRANCH; }
}

/* Branch on Result Zero */
static inline void BEQ()
{
    if ( get_flag(Z) ) { TAKE_BRANCH; }
}

/* Branch on Result Negative */
static inline void BMI()
{
    if ( get_flag(N) ) { TAKE_BRANCH; }
}

/* Branch on Result not Zero*/
static inline void BNE()
{
    if ( !get_flag(Z) ) { TAKE_BRANCH; }
}

/* Branch on Result Positive */
static inline void BPL()
{
    if ( !get_flag(N) ) { TAKE_BRANCH; }
}

/* Branch on Overflow Clear */
static inline void BVC()
{
    if ( !get_flag(V) ) { TAKE_BRANCH; }
}

/* Branch on Overflow Set */
static inline void BVS()
{
    if ( get_flag(V) ) { TAKE_BRANCH; }
}

/* Force break */
static inline void BRK()
{
    /* Dirty hack pls fix ty :) */
    Break_and_die = true;

    uint8_t PC_hi = (uint8_t)(nes_cpu_registers.PC + 2) >> 8;
    uint8_t PC_lo = (uint8_t)(nes_cpu_registers.PC + 2);
    PUSH(PC_hi);
    PUSH(PC_lo);
    PUSH(nes_cpu_registers.S);

    nes_cpu_registers.PC = (uint16_t)PEEK(0xFFFF) << 8 | PEEK(0xFFFE);
    test_flag(B, 1);
}

/* Clear Carry Flag */
static inline void CLC()
{
    clear_flag(C);
}

/* Clear Decimal Mode, Unused in the NES */
static inline void CLD()
{
    clear_flag(D);
}

/* Clear Interrupt Disable Bit */
static inline void CLI()
{
    clear_flag(I);
}

/* CLear Overflow Flag */
static inline void CLV()
{
    clear_flag(V);
}

/* Compare Memory with Accumulator */
static inline void CMP()
{
    uint16_t sub = nes_cpu_bus.DB - nes_cpu_registers.A;

    test_flag(N, IS_NEGATIVE((uint8_t)sub));
    test_flag(Z, (uint8_t)sub == 0x00);
    test_flag(C, ( sub > 0xFF ) | ((uint8_t)sub == 0x00));
}

/* Compare Memory and Index X */
static inline void CPX()
{
    uint16_t sub = nes_cpu_bus.DB - nes_cpu_registers.X;

    test_flag(N, IS_NEGATIVE((uint8_t)sub));
    test_flag(Z, (uint8_t)sub == 0x00);
    test_flag(C, ( sub > 0xFF ) | ((uint8_t)sub == 0x00));
}

/* Compare Memory and Index Y */
static inline void CPY()
{
    uint16_t sub = nes_cpu_bus.DB - nes_cpu_registers.Y;

    test_flag(N, IS_NEGATIVE((uint8_t)sub));
    test_flag(Z, (uint8_t)sub == 0x00);
    test_flag(C, ( sub > 0xFF ) | ((uint8_t)sub == 0x00));
}

/* DECrement memory */
static inline void DEC()
{
    uint8_t mem = PEEK(nes_cpu_registers.PC + 1) - 1;
    POKE(nes_cpu_bus.AB, mem);

    test_flag(N, IS_NEGATIVE(nes_cpu_bus.DB));
    test_flag(Z, ( nes_cpu_bus.DB == 0x00 ));
}

/* Decrement Index X by One */
static inline void DEX()
{
    nes_cpu_registers.X--;

    test_flag(N, IS_NEGATIVE(nes_cpu_registers.X));
    test_flag(Z, (nes_cpu_registers.X == 0x00)); 
}

/* Decrement Index Y by One */
static inline void DEY()
{
    nes_cpu_registers.Y--;

    test_flag(N, IS_NEGATIVE(nes_cpu_registers.Y));
    test_flag(Z, (nes_cpu_registers.Y == 0x00));
}

/* Exclusive OR (XOR) memory with accumulator */
static inline void EOR()
{
    nes_cpu_registers.A ^= nes_cpu_bus.DB;

    test_flag(N, IS_NEGATIVE(nes_cpu_registers.A));
    test_flag(Z, (nes_cpu_registers.A == 0x00));
}

/* Increment memory by one */
static inline void INC()
{
    nes_cpu_bus.DB++;

    test_flag(N, IS_NEGATIVE(nes_cpu_bus.DB));
    test_flag(Z, (nes_cpu_bus.DB == 0x00));
}

/* Increment X by one */
static inline void INX()
{
    nes_cpu_registers.X++;

    test_flag(N, IS_NEGATIVE(nes_cpu_registers.X));
    test_flag(Z, (nes_cpu_registers.X == 0x00));
}

/* Increment Y by one */
static inline void INY()
{
    nes_cpu_registers.Y++;

    test_flag(N, IS_NEGATIVE(nes_cpu_registers.Y));
    test_flag(Z, (nes_cpu_registers.Y == 0x00));
}

/* Jump to new location */
static inline void JMP()
{
    nes_cpu_registers.PC = nes_cpu_bus.AB;
    PC_offset = 0; /* Dirty hack pls fix ty :) */
}

/* Jump saving return address */
static inline void JSR()
{
    uint8_t hi = ((nes_cpu_registers.PC + 2) & 0xFF00) >> 8;
    uint8_t lo = ((nes_cpu_registers.PC + 2) & 0x00FF);

    PUSH(hi);
    PUSH(lo);

    nes_cpu_registers.PC = nes_cpu_bus.AB; 
    PC_offset = 0; /* Dirty hack pls fix ty :) */
}

/* Load accumulator with memory */
static inline void LDA()
{
    nes_cpu_registers.A = nes_cpu_bus.DB;

    test_flag(N, IS_NEGATIVE(nes_cpu_registers.A));
    test_flag(Z, (nes_cpu_registers.A == 0x00));
}

/* Load index X with memory */
static inline void LDX()
{
    nes_cpu_registers.X = nes_cpu_bus.DB;

    test_flag(N, IS_NEGATIVE(nes_cpu_registers.X));
    test_flag(Z, (nes_cpu_registers.X == 0x00));
}

/* Load index Y with memory */
static inline void LDY()
{
    nes_cpu_registers.Y = nes_cpu_bus.DB;

    test_flag(N, IS_NEGATIVE(nes_cpu_registers.Y));
    test_flag(Z, (nes_cpu_registers.Y == 0x00));
}

/* Logical shift right (memory or accumulator)*/
static inline void LSR()
{
    clear_flag(N);

    test_flag(C, (nes_cpu_bus.DB & 0x01));
    nes_cpu_bus.DB >>= 1;

    test_flag(Z, (nes_cpu_bus.DB == 0x00));
    POKE(nes_cpu_bus.AB, nes_cpu_bus.DB);
}

/* No operation */
static inline void NOP()
{
    
}

/* OR memory with Accumulator */
static inline void ORA()
{
    nes_cpu_registers.A |= nes_cpu_bus.DB;

    test_flag(N, IS_NEGATIVE(nes_cpu_registers.A));
    test_flag(Z, (nes_cpu_registers.A == 0x00)); 
}

/* Push Accumulator on Stack */
static inline void PHA()
{
    PUSH(nes_cpu_registers.A);
}

/* Push Processor Status on Stack */
static inline void PHP()
{
    PUSH(nes_cpu_registers.S);
}

/* Pull Accumulator from Stack */
static inline void PLA()
{
    nes_cpu_registers.A = POP();

    test_flag(N, IS_NEGATIVE(nes_cpu_registers.A));
    test_flag(Z, (nes_cpu_registers.A == 0x00));
}

/* Pull Processor Status from Stack */
static inline void PLP()
{
    nes_cpu_registers.S = POP();
    clear_flag(B);
    test_flag(U, 1);
}

/* Rotate one bit left */
static inline void ROL()
{
    test_flag(C, (nes_cpu_bus.DB & 0x01));
    nes_cpu_bus.DB = (nes_cpu_bus.DB << 1) | (IS_NEGATIVE(nes_cpu_bus.DB));
    
    test_flag(N, IS_NEGATIVE(nes_cpu_bus.DB));
    test_flag(Z, (nes_cpu_bus.DB == 0x00));    
}

/* Rotate one bit right */
static inline void ROR()
{
    test_flag(C, (nes_cpu_bus.DB & 0x01));
    /* TO-DO: Test if this code would work */
    nes_cpu_bus.DB = (nes_cpu_bus.DB >> 1) | ((nes_cpu_bus.DB & 0x01) ? 0x80 : 0x00);
    
    test_flag(N, IS_NEGATIVE(nes_cpu_bus.DB));
    test_flag(Z, (nes_cpu_bus.DB == 0x00));
}

/* Return from interrupt */
static inline void RTI()
{
    nes_cpu_registers.S = POP();
    uint8_t lo = POP();
    uint8_t hi = POP();
    
    nes_cpu_registers.PC = (uint16_t)(hi << 8) | lo;
}

/* Return from subroutine */
static inline void RTS()
{
    uint8_t lo = POP();
    uint8_t hi = POP();

    nes_cpu_registers.PC = ((uint16_t)(hi << 8) | lo);
}

/* Subtract with carry */
static inline void SBC()
{
    nes_cpu_bus.DB = TWOS_COMP(nes_cpu_bus.DB);
    ADC();
}

/* Set carry flag */
static inline void SEC()
{
    test_flag(C, 1);
}

/* Set decimal mode flag (unused on NES) */
static inline void SED()
{
    test_flag(D, 1);
}

/* Set interrupt disable status flag */
static inline void SEI()
{
    test_flag(I, 1);
}

/* Store accumulator in memory */
static inline void STA()
{
    POKE(nes_cpu_bus.AB, nes_cpu_registers.A);
}

/* Store Index X in memory */
static inline void STX()
{
    POKE(nes_cpu_bus.AB, nes_cpu_registers.X);
}

/* Store Index Y in memory */
static inline void STY()
{
    POKE(nes_cpu_bus.AB, nes_cpu_registers.Y);
}

/* Transfer accumulator to X */
static inline void TAX()
{
    test_flag(N, IS_NEGATIVE(nes_cpu_registers.A));
    test_flag(Z, (nes_cpu_registers.A == 0x00));

    nes_cpu_registers.X = nes_cpu_registers.A;
}

/* Transfer accumulator to Y */
static inline void TAY()
{
    test_flag(N, IS_NEGATIVE(nes_cpu_registers.A));
    test_flag(Z, (nes_cpu_registers.A == 0x00));

    nes_cpu_registers.Y = nes_cpu_registers.A;
}

/* Transfer stack pointer to X */
static inline void TSX()
{
    test_flag(N, IS_NEGATIVE(nes_cpu_bus.DB));
    test_flag(Z, (nes_cpu_bus.DB == 0x00));

    nes_cpu_registers.X = nes_cpu_registers.SP;
}

/* Transfer X to Accumulator */
static inline void TXA()
{
    test_flag(N, IS_NEGATIVE(nes_cpu_registers.X));
    test_flag(Z, (nes_cpu_registers.X == 0x00));

    nes_cpu_registers.A = nes_cpu_registers.X;
}

/* Transfer X to Stack Pointer */
static inline void TXS()
{
    test_flag(N, IS_NEGATIVE(nes_cpu_registers.X));
    test_flag(Z, (nes_cpu_registers.X == 0x00));

    nes_cpu_registers.SP = nes_cpu_registers.X;
}

/* Transfer Y to Accumulator */
static inline void TYA()
{
    test_flag(N, IS_NEGATIVE(nes_cpu_registers.Y));
    test_flag(Z, (nes_cpu_registers.Y == 0x00));

    nes_cpu_registers.A = nes_cpu_registers.Y;
}