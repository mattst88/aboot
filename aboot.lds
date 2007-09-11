OUTPUT_FORMAT("elf64-alpha")
ENTRY(__start)
PHDRS { kernel PT_LOAD; }
SECTIONS
{
  . = 0x20000000;
  .text : { *(.text) } :kernel
  _etext = .;
  PROVIDE (etext = .);
  .rodata : { *(.rodata) } :kernel
  .data : { *(.data) } :kernel
  .got : { *(.got) } :kernel
  .sdata : { *(.sdata) } :kernel
  _edata = .;
  PROVIDE (edata = .);
  .sbss : { *(.sbss) *(.scommon) } :kernel
  .bss : { *(.bss) *(COMMON) } :kernel
  _end = . ;
  PROVIDE (end = .);

  /DISCARD/ : { *(.eh_frame) }

  .mdebug 0 : { *(.mdebug) }
  .note 0 : { *(.note) }
  .comment 0 : { *(.comment) }
}
