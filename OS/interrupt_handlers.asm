section .data
    ; IDT zawierająca jeden wpis dla przerwania systemowego
    idt_descriptor:
        dw idt_end - idt_start - 1
        dd idt_start

idt_start:
    ; Pierwszy (jedyny) wpis IDT dla przerwania systemowego
    dw system_call_handler_offset_low    ; Dolne 16 bitów adresu obsługi przerwania systemowego
    dw 0x08                             ; Selektor segmentu kodu (GDT indeks 1)
    db 0x0                              ; Domyślny istniejący segment
    db 0x8E                             ; Typ bramy: 0b10001110 (P=1, DPL=00, S=0, Type=32-bit Interrupt Gate)
    dw system_call_handler_offset_high   ; Górne 16 bitów adresu obsługi przerwania systemowego

idt_end: