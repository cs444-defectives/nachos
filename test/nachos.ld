SEARCH_DIR(/home/f85/kearns/nachos_compiler/decstation-ultrix/lib);
ENTRY(__start)
SECTIONS
{
  .text : {
     _ftext = . ;
    *(.init)
    *(.text)
     etext  =  .;
     _etext  =  .;
  }
  .data : {
    *(.rdata)
    *(.data)
    *(.sdata)
    *(.scommon)
  }
  .bss : {
    *(.bss)
    *(.sbss)
    *(COMMON)
  }
   end = .;
   _end = .;
}

