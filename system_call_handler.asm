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

section .text
extern system_call_handler  ; Deklaracja funkcji obsługującej przerwanie systemowe

; Funkcja obsługująca przerwanie systemowe
system_call_handler:
    ; Umieść tutaj odpowiedni kod obsługi przerwania systemowego.
    ; W tym przypadku, możemy dodać wywołanie naszej wcześniej zdefiniowanej funkcji `systemCallHandler`.
    call systemCallHandler

    ; Obsługa przerwania systemowego zakończona, wróć do normalnego wykonania programu.
    iret

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

    ; Wczytanie deskryptora IDT do rejestru IDTR
    lidt [idt_descriptor]

    ; Skok do kodu segmentu 0x08 w trybie chronionym (w tym przypadku do etykiety protected_mode_code)
    jmp 0x08:protected_mode_code

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

section .data
    ; Kod segmentu w trybie chronionym
    protected_mode_code:
        ; Tutaj umieść odpowiedni kod w trybie chronionym
        ; np. kod do dalszej inicjalizacji i uruchomienia systemu operacyjnego

// Obsługa przerwania sprzętowego (np. od timera lub innego urządzenia)
void hardwareInterruptHandler() {
    // Symulacja obsługi przerwania
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Przerwanie sprzętowe - zmiana aktywnego procesu!" << std::endl;

        // Wywołanie przerwania systemowego (int 0x20) do zmiany aktywnego procesu
        asm volatile("int $0x20");
    }
}
