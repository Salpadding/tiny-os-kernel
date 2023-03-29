ENTRY(_start)
SECTIONS
{

 . = 0x500,
 .text : {
  *(entry);
  *(.text);
 }
 .data : {
   *(.data);
   *(.rodata*);
 }
}
