/* NXP UM10360 cap. 2 y 32. SRAM AHB separada de la SRAM local. */
ENTRY(Reset_Handler)
EXTERN(main)
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 512K
    RAM (rwx)   : ORIGIN = 0x10000000, LENGTH = 32K
    AHB0 (rwx)  : ORIGIN = 0x2007C000, LENGTH = 16K
    AHB1 (rwx)  : ORIGIN = 0x20080000, LENGTH = 16K
}
__stack_top__ = ORIGIN(RAM) + LENGTH(RAM);
__stack_bottom__ = __stack_top__ - 2K;
__vector_checksum__ = (0 - (__stack_top__ + (Reset_Handler | 1)
    + (NMI_Handler | 1) + (HardFault_Handler | 1) + (MemManage_Handler | 1)
    + (BusFault_Handler | 1) + (UsageFault_Handler | 1))) & 0xFFFFFFFF;

SECTIONS
{
    .isr_vector ORIGIN(FLASH) : { KEEP(*(.isr_vector)) } > FLASH
    .crp 0x2FC : { LONG(0xFFFFFFFF) } > FLASH
    .text :
    {
        *(.text .text.*)
        *(.rodata .rodata.*)
        . = ALIGN(4);
    } > FLASH
    .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } > FLASH
    .ARM.exidx : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) } > FLASH
    .data : ALIGN(4)
    {
        __data_start__ = .;
        *(.data .data.*)
        . = ALIGN(4);
        __data_end__ = .;
    } > RAM AT > FLASH
    __data_load__ = LOADADDR(.data);
    .bss (NOLOAD) : ALIGN(4)
    {
        __bss_start__ = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } > RAM
    .stack __stack_bottom__ (NOLOAD) :
    {
        . += 2K;
    } > RAM
    /DISCARD/ : { *(.comment) *(.note.GNU-stack) }
}
ASSERT(SIZEOF(.isr_vector) == 51 * 4, "Tabla de vectores LPC1769 incompleta")
ASSERT(__bss_end__ <= __stack_bottom__, "SRAM local invade reserva de pila")
ASSERT((__stack_top__ & 7) == 0, "Pila no alineada a 8 bytes")
ASSERT((__data_load__ & 3) == 0, "Origen de .data no alineado")
