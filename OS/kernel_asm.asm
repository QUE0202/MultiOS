section .data
    ; Pierwszy deskryptor GDT (indeks 0)
    gdt_entry_0:
        dd 0x00000000 ; Deskryptor pusty

    ; Drugi deskryptor GDT (indeks 1) - deskryptor segmentu kodu
    gdt_entry_1:
        dw 0xFFFF ; Limit 0-15
        dw 0x0000 ; Bazowy adres 0-15
        db 0x00   ; Bazowy adres 16-23
        db 0x9A   ; Atrybuty: P=1, DPL=00, S=1, Type=Executable, Readable
        db 0xA0   ; Atrybuty: G=1, D/B=1 (32-bitowy segment), AVL=0
        db 0x00   ; Bazowy adres 24-31

    ; Trzeci deskryptor GDT (indeks 2) - deskryptor segmentu danych
    gdt_entry_2:
        dw 0xFFFF ; Limit 0-15
        dw 0x0000 ; Bazowy adres 0-15
        db 0x00   ; Bazowy adres 16-23
        db 0x92   ; Atrybuty: P=1, DPL=00, S=1, Type=Data, Writable
        db 0xA0   ; Atrybuty: G=1, D/B=1 (32-bitowy segment), AVL=0
        db 0x00   ; Bazowy adres 24-31

    ; Deskryptor GDT (Global Descriptor Table)
    gdt_descriptor:
        dw gdt_end - gdt_start - 1
        dd gdt_start

gdt_start:
    ; Adres początku tablicy deskryptorów GDT
    dd gdt_entry_0
    dd gdt_entry_1
    dd gdt_entry_2

gdt_end:

section .text
global enable_protected_mode

enable_protected_mode:
    ; Wyłączenie przerwań
    cli

    ; Wczytanie deskryptora Global Descriptor Table (GDT) do rejestru GDTR
    lgdt [gdt_descriptor]

    ; Ustawienie bitu PE (Protected Mode Enable) w rejestrze kontrolnym CR0
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    ; Skok do kodu segmentu 0x08 w trybie chronionym (w tym przypadku do etykiety protected_mode_code)
    jmp 0x08:protected_mode_code

section .data
    ; Kod segmentu w trybie chronionym
    protected_mode_code:
        ; Tutaj umieść odpowiedni kod w trybie chronionym
        ; np. kod do dalszej inicjalizacji i uruchomienia systemu operacyjnego
