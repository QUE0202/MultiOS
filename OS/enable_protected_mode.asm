section .text
global enable_protected_mode

enable_protected_mode:
    ; Wyłączenie przerwań
    cli

    ; Wczytanie deskryptora Global Descriptor Table (GDT) do rejestru GDTR
    lgdt [gdt_descriptor]

    ; Wczytanie deskryptora Interrupt Descriptor Table (IDT) do rejestru IDTR
    lidt [idt_descriptor]

    ; Ustawienie bitu PE (Protected Mode Enable) w rejestrze kontrolnym CR0
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    ; Skok do kodu segmentu 0x08 w trybie chronionym (w tym przypadku do etykiety protected_mode_code)
    jmp 0x08:protected_mode_code

; ... reszta kodu ...

section .data
    ; Kod segmentu w trybie chronionym
    protected_mode_code:
        ; Tutaj umieść odpowiedni kod w trybie chronionym
        ; np. kod do dalszej inicjalizacji i uruchomienia systemu operacyjnego

    ; Obsługa przerwania systemowego - Offset dla funkcji system_call_handler
    system_call_handler_offset_low:
        dw system_call_handler & 0xFFFF
    system_call_handler_offset_high:
        dw (system_call_handler >> 16) & 0xFFFF
