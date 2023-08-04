section .text
    ; Wprowadzenie do trybu chronionego
    cli                   ; Wyłączenie przerwań
    lgdt [gdt_descriptor] ; Wczytanie deskryptora GDT

    ; Ustawienie bitu PE (Protected Mode Enable) w rejestrze kontrolnym CR0
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    ; Skok do kodu segmentu 0x08 w trybie chronionym (w tym przypadku do etykiety protected_mode_code)
    jmp 0x08:start_protected_mode

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

gdt_start:
    ; Pierwszy deskryptor GDT (indeks 0)
    dd 0x00000000 ; Deskryptor pusty

    ; Drugi deskryptor GDT (indeks 1) - deskryptor segmentu kodu
    dw 0xFFFF ; Limit 0-15
    dw 0x0000 ; Bazowy adres 0-15
    db 0x00   ; Bazowy adres 16-23
    db 0x9A   ; Atrybuty: P=1, DPL=00, S=1, Type=Executable, Readable
    db 0xA0   ; Atrybuty: G=1, D/B=1 (32-bitowy segment), AVL=0
    db 0x00   ; Bazowy adres 24-31

    ; Trzeci deskryptor GDT (indeks 2) - deskryptor segmentu danych
    dw 0xFFFF ; Limit 0-15
    dw 0x0000 ; Bazowy adres 0-15
    db 0x00   ; Bazowy adres 16-23
    db 0x92   ; Atrybuty: P=1, DPL=00, S=1, Type=Data, Writable
    db 0xA0   ; Atrybuty: G=1, D/B=1 (32-bitowy segment), AVL=0
    db 0x00   ; Bazowy adres 24-31

gdt_end:

section .bss
    ; Deskryptor IDT (Interrupt Descriptor Table)
    idt_descriptor:
        dw idt_end - idt_start - 1
        dd idt_start

    idt_start:
        ; Tutaj definiujemy wpisy dla przerwań systemowych, np.:
        ; dd interrupt_handler_0 ; Wpis dla przerwania 0
        ; dd interrupt_handler_1 ; Wpis dla przerwania 1
        ; ...
        ; dd interrupt_handler_n ; Wpis dla przerwania n

    idt_end:

section .text
    ; Obsługa przerwania sprzętowego (np. od timera lub innego urządzenia)
    hardware_interrupt_handler:
        ; Kod obsługi przerwania sprzętowego
        ; W tym miejscu dodaj odpowiedni kod obsługi przerwania

        iret ; Powrót z obsługi przerwania

    ; Tutaj można dodać obsługę innych przerwań, jak np. przerwanie od klawiatury

section .text
    start_protected_mode:
        ; W tym miejscu można umieścić kod ładowania i uruchomienia jądra systemu

        ; Przejście do trybu chronionego
        mov ax, 0x10 ; Segment kodu w trybie chronionym (2. deskryptor GDT)
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        mov ss, ax

        ; W tym miejscu możemy wywołać kod jądra systemu (tutaj zakładamy, że kod jądra to adres 0x1000)
        jmp 0x1000:kernel.cpp

        ; Ewentualnie można dodać procedurę z czyszczeniem ekranu, żeby wyglądało bardziej efektownie
        ; np. dla trybu tekstowego z dostępem do VGA:
        ; xor ah, ah ; Czyszczenie ekranu: kod ASCII spacji
        ; mov al, 0x20
        ; xor di, di
        ; mov cx, 80*25 ; Wielkość ekranu (80 kolumn * 25 wierszy)
        ; rep stosw

        ; Wywołanie jądra systemu (przykładowo na adresie 0x1000)
        ; jmp 0x1000
