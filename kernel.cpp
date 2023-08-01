#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>

// Funkcja odpowiedzialna za aktywację trybu chronionego (x86)
extern "C" void enable_protected_mode();

// Struktura opisująca deskryptor GDT (Global Descriptor Table)
struct GDTDescriptor {
    uint16_t size;
    uint32_t offset;
};

// GDT zawierająca tylko jeden segment danych dla trybu chronionego
struct GDTEntry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

// Klasa reprezentująca pojedynczy proces
class Process {
public:
    Process(int id, const std::string& name, int executionTime) : id(id), name(name), executionTime(executionTime) {}

    // Metoda do wykonania akcji przez proces
    void execute() {
        for (int i = 0; i < executionTime; ++i) {
            std::cout << "Proces " << id << " (" << name << ") wykonywany krok " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Symulacja pracy procesu
        }
    }

    // Getter zwracający czas wykonania procesu
    int getExecutionTime() const {
        return executionTime;
    }

private:
    int id;
    std::string name;
    int executionTime;
};

// Funkcja, którą będzie wykonywać wątek
void threadFunction(Process& process) {
    process.execute();
}

// Funkcja symulująca obsługę przerwania
void interruptHandler() {
    std::cout << "Obsługa przerwania - program został zatrzymany na 5 sekund." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
}

// Deklaracja funkcji obsługującej przerwanie sprzętowe
void hardwareInterruptHandler();

// Zmienna do synchronizacji dostępu do harmonogramu procesów
std::mutex schedulerMutex;

// Aktualnie aktywny proces
int activeProcessIndex = 0;

// Funkcja obsługująca harmonogramowanie procesów
void scheduler(std::vector<Process>& processes) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(2));

        std::lock_guard<std::mutex> lock(schedulerMutex);

        // Zmiana aktywnego procesu
        activeProcessIndex = (activeProcessIndex + 1) % processes.size();
        std::cout << "Zmiana aktywnego procesu - proces " << processes[activeProcessIndex].getExecutionTime() << " zakończył pracę." << std::endl;
    }
}

// Funkcja obsługująca przerwania systemowe
void systemCallHandler() {
    std::lock_guard<std::mutex> lock(schedulerMutex);
    std::cout << "Przerwanie systemowe - zmiana aktywnego procesu!" << std::endl;
    // Wywołanie przerwania systemowego (int 0x20) do zmiany aktywnego procesu
    asm volatile("int $0x20");
}

// Funkcja obsługująca harmonogramowanie procesów na podstawie przerwań systemowych
void schedulerBySystemCalls(std::vector<Process>& processes) {
    while (true) {
        // Oczekiwanie na przerwanie systemowe
        std::this_thread::sleep_for(std::chrono::seconds(2));
        systemCallHandler();
    }
}

// Główna funkcja jądra systemu
extern "C" void kernel_main()
{
    // Przykładowa funkcjonalność jądra
    std::cout << "Witaj w jądrze systemu!" << std::endl;

    // Aktywacja trybu chronionego
    enable_protected_mode();

    // Tworzenie procesów
    std::vector<Process> processes;
    processes.emplace_back(1, "Proces 1", 3); // Proces 1 ma czas wykonania 3 jednostki czasu
    processes.emplace_back(2, "Proces 2", 5); // Proces 2 ma czas wykonania 5 jednostek czasu

    // Tworzenie wątków dla każdego procesu
    std::vector<std::thread> threads;
    for (auto& process : processes) {
        threads.emplace_back(threadFunction, std::ref(process));
    }

    // Wątek harmonogramujący procesy
    std::thread schedulerThread(scheduler, std::ref(processes));

    // Wątek obsługujący przerwania systemowe
    std::thread systemCallThread(schedulerBySystemCalls, std::ref(processes));

    // Oczekiwanie na zakończenie wątków
    for (auto& thread : threads) {
        thread.join();
    }

    // Czekamy na zakończenie wątku harmonogramującego
    schedulerThread.join();

    // Czekamy na zakończenie wątku obsługującego przerwania systemowe
    systemCallThread.join();

    std::cout << "Wszystkie wątki zakończyły pracę." << std::endl;

    // Wątek obsługujący przerwanie
    std::thread interruptThread(hardwareInterruptHandler);

    // Pętla w celu uniknięcia przypadkowego zatrzymania programu
    while (true)
    {
        // Symulacja harmonogramowania procesów (zmiana aktywnego procesu co 3 sekundy)
        std::this_thread::sleep_for(std::chrono::seconds(3));
        std::lock_guard<std::mutex> lock(schedulerMutex);
        std::cout << "Zmiana aktywnego procesu - proces " << processes[activeProcessIndex].getExecutionTime() << " zakończył pracę." << std::endl;
    }

    // Czekamy na zakończenie wątku obsługującego przerwanie
    interruptThread.join();
}

// Funkcja odpowiedzialna za aktywację trybu chronionego (x86)
extern "C" void enable_protected_mode() {
    // W tym miejscu należy umieścić odpowiednie instrukcje assemblerowe,
    // które wykonają aktywację trybu chronionego x86.
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


    // Inicjalizacja GDT (Global Descriptor Table) zawierającej tylko jeden segment danych dla trybu chronionego
    GDTEntry gdt_entry;
    gdt_entry.limit_low = 0xFFFF;
    gdt_entry.base_low = 0x0000;
    gdt_entry.base_middle = 0x00;
    gdt_entry.access = 0x9A;  // Segment jest dostępny w trybie supervisor, jest segmentem kodu i ma pełne uprawnienia
    gdt_entry.granularity = 0xCF;  // Granularność 4 KB, 32-bitowy segment
    gdt_entry.base_high = 0x00;

    GDTDescriptor gdt_descriptor;
    gdt_descriptor.size = sizeof(GDTEntry) - 1;
    gdt_descriptor.offset = reinterpret_cast<uint32_t>(&gdt_entry);

    asm volatile (
        "cli\n\t"                         // Wyłącz przerwania
        "lgdtl %0\n\t"                    // Załaduj deskryptor GDT do GDTR
        "movl %%cr0, %%eax\n\t"           // Odczytaj rejestr CR0 do EAX
        "orl $0x01, %%eax\n\t"            // Ustaw bit PE na 1 w rejestrze EAX
        "movl %%eax, %%cr0\n\t"           // Zapisz EAX do rejestr CR0
        "ljmpl $0x08, $protected_mode\n"  // Skok do nowego punktu wejścia trybu chronionego
        "protected_mode:\n\t"
        "movw $0x10, %%ax\n\t"            // Segment danych dla trybu chronionego
        "movw %%ax, %%ds\n\t"             // Ustaw rejestr DS na nowy segment
        "movw %%ax, %%es\n\t"             // Ustaw rejestr ES na nowy segment
        "movw %%ax, %%fs\n\t"             // Ustaw rejestr FS na nowy segment
        "movw %%ax, %%gs\n\t"             // Ustaw rejestr GS na nowy segment
        "movw %%ax, %%ss\n\t"             // Ustaw rejestr SS na nowy segment
        :
        : "m" (gdt_descriptor)
        : "eax"
    );

    std::cout << "Tryb chroniony aktywowany!" << std::endl;
}

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