#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include <cstdlib> // For system function to open the default browser
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h> // ComPtr
#include <msclr/marshal_cppstd.h> // For .NET Framework integration
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#include <d3d12.h>
#include <d3dcompiler.h>
#include <wrl/client.h> // ComPtr

#pragma comment(lib, "d3dcompiler.lib")


// Deklaracje funkcji assemblera
extern "C" void enable_protected_mode();

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
    Process(int id, const std::string& name, int executionTime, int startAddress, int memorySize)
        : id(id), name(name), executionTime(executionTime), startAddress(startAddress), memorySize(memorySize) {}

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

int getStartAddress() const {
        return startAddress;
    }

    int getMemorySize() const {
        return memorySize;
    }

private:
    int id;
    std::string name;
    int executionTime;
    int startAddress;
    int memorySize;
};

// Funkcja do uruchamiania domyślnej przeglądarki
void openDefaultBrowser() {
    std::cout << "Otwieranie domyślnej przeglądarki..." << std::endl;

#ifdef _WIN32
    system("start http://www.example.com");
#elif defined __linux__
    system("xdg-open http://www.example.com");
#elif defined __APPLE__
    system("open http://www.example.com");
#else
    std::cout << "Nieobsługiwany system operacyjny - nie można otworzyć przeglądarki." << std::endl;
#endif
}

// Funkcja, którą będzie wykonywać wątek przeglądarki
void browserThreadFunction() {
    callDotNetFunction();
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
    int systemCallNumber = 0; // Domyślnie wywołanie 0 to zmiana aktywnego procesu
    std::cout << "Wywołanie systemowe - ";

    // Tutaj możemy dodać logikę do obsługi różnych numerów wywołania systemowego
    // Dla uproszczenia przyjmujemy, że jeśli jest parzysty numer wywołania, to jest to nasza własna obsługa
    if (systemCallNumber % 2 == 0) {
        std::cout << "zmiana aktywnego procesu!" << std::endl;
        // Wywołanie przerwania systemowego (int 0x20) do zmiany aktywnego procesu
        asm volatile("int $0x20");
    } else {
        std::cout << "własne wywołanie systemowe!" << std::endl;
        // Wywołanie naszej własnej funkcji obsługującej własne wywołanie systemowe
        customSystemCallHandler();
    }
}

// Funkcja obsługująca harmonogramowanie procesów na podstawie przerwań systemowych
void schedulerBySystemCalls(std::vector<Process>& processes) {
    while (true) {
        // Oczekiwanie na przerwanie systemowe
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::lock_guard<std::mutex> lock(schedulerMutex);
        systemCallHandler();
    }
}

// Funkcja do wyświetlenia menu i pobrania wyboru użytkownika
int pokazMenu(const std::vector<Process>& processes, int activeProcessIndex) {
    int wybor;
    std::cout << "\nMenu:\n";
    std::cout << "1. Wykonaj procesy\n";
    std::cout << "2. Wykonaj przykład .NET Framework\n";
    std::cout << "3. Otwórz domyślną przeglądarkę\n";
    std::cout << "4. Wyjście\n";
    std::cout << "5. Utwórz nowy proces\n";
    std::cout << "6. Wyświetl listę procesów\n";
    std::cout << "7. Wyświetl informacje o aktualnym aktywnym procesie\n";
    std::cout << "Wybierz opcję: ";
    std::cin >> wybor;

    if (wybor == 6) {
        std::cout << "Aktualnie aktywny proces: " << processes[activeProcessIndex].getExecutionTime()
                  << " (" << processes[activeProcessIndex].getExecutionTime() << " jednostek czasu)" << std::endl;
        wybor = pokazMenu(processes, activeProcessIndex); // Show the menu again
    }

    return wybor;
}

// Funkcja do tworzenia i uruchamiania nowego procesu
void createProcess(std::vector<Process>& processes) {
    std::string name;
    int executionTime;
    std::cout << "Podaj nazwę nowego procesu: ";
    std::cin >> name;
    std::cout << "Podaj czas wykonania nowego procesu: ";
    std::cin >> executionTime;

    int requiredMemory;
    std::cout << "Podaj ilość pamięci potrzebnej dla procesu: ";
    std::cin >> requiredMemory;

    int allocatedMemory = memoryManager.allocateMemory(requiredMemory);
    if (allocatedMemory != -1) {
        processes.emplace_back(processes.size() + 1, name, executionTime, allocatedMemory, requiredMemory);
        std::cout << "Utworzono nowy proces o nazwie '" << name << "' i czasie wykonania " << executionTime << " jednostek czasu."
                  << " Przydzielono " << requiredMemory << " bajtów pamięci." << std::endl;
    } else {
        std::cout << "Nie udało się utworzyć procesu. Brak wystarczającej ilości wolnej pamięci." << std::endl;
    }
}


// Funkcja do wyświetlenia listy procesów
void showProcesses(const std::vector<Process>& processes) {
    std::cout << "Lista procesów:" << std::endl;
    for (const auto& process : processes) {
        std::cout << "Proces " << process.getExecutionTime() << " (" << process.getExecutionTime() << " jednostek czasu)" << std::endl;
    }
}

// Główna funkcja jądra systemu
extern "C" void kernel_main() {
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
        threads.emplace_back(&Process::execute, &process);
    }

    // Wątek harmonogramujący procesy
    std::thread schedulerThread(scheduler, std::ref(processes));

    // Wątek obsługujący przerwania systemowe
    std::thread systemCallThread(schedulerBySystemCalls, std::ref(processes));

    // Wątek obsługujący przerwania sprzętowe
    std::thread interruptThread(hardwareInterruptHandler);

    // Wątek przeglądarki
    std::thread browserThread(browserThreadFunction);

    // Zmienna do przechowywania wyboru użytkownika
    int wyborUzytkownika;

    while (true) {
        wyborUzytkownika = pokazMenu(processes, activeProcessIndex);

        switch (wyborUzytkownika) {
            case 1:
                // Wykonanie procesów
                for (auto& thread : threads) {
                    thread.join();
                }
                break;
             case 2:
                // Wykonaj przykład .NET Framework
                browserThread.join();
                break;
            case 3:
                // Otworzenie domyślnej przeglądarki
                browserThread.join();
                break;
            case 4:
                // Wyjście z programu
                std::cout << "Zamykanie programu..." << std::endl;
                // Dodaj ewentualne procedury czyszczenia lub zamykania tutaj, jeśli są potrzebne
                return;
            case 5:
                // Utworzenie nowego procesu
                createProcess(processes);
                break;
            case 6:
                // Wyświetlenie listy procesów
                showProcesses(processes);
                break; 
            case 7:
                // Terminate a running process
                std::lock_guard<std::mutex> lock(schedulerMutex);
                std::cout << "Zatrzymywanie aktualnie aktywnego procesu..." << std::endl;
                threads[activeProcessIndex].join();
                processes.erase(processes.begin() + activeProcessIndex);
                activeProcessIndex = 0;
                std::cout << "Proces został zatrzymany." << std::endl;
                break;
            case 8:
                // Wywołanie własnego wywołania systemowego
                customSystemCallHandler();
                break;
            default:
                std::cout << "Nieprawidłowy wybór. Spróbuj ponownie." << std::endl;
                break;
    }
}

    // Czekamy na zakończenie wątku harmonogramującego
    schedulerThread.join();

    // Czekamy na zakończenie wątku obsługującego przerwania systemowe
    systemCallThread.join();

    // Czekamy na zakończenie wątku przeglądarki
    browserThread.join();

    std::cout << "Wszystkie wątki zakończyły pracę." << std::endl;

    // Pętla w celu uniknięcia przypadkowego zatrzymania programu
    while (true) {
        // Symulacja harmonogramowania procesów (zmiana aktywnego procesu co 3 sekundy)
        std::this_thread::sleep_for(std::chrono::seconds(3));
        std::lock_guard<std::mutex> lock(schedulerMutex);
        std::cout << "Zmiana aktywnego procesu - proces " << processes[activeProcessIndex].getExecutionTime() << " zakończył pracę." << std::endl;
    }
}

// Funkcja odpowiedzialna za aktywację trybu chronionego (x86)
extern "C" void enable_protected_mode() {
    // W tym miejscu należy umieścić odpowiednie instrukcje assemblerowe,
    // które wykonają aktywację trybu chronionego x86.
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

// Funkcja do obsługi własnego wywołania systemowego
void customSystemCallHandler() {
    std::cout << "Wywołano własne wywołanie systemowe!" << std::endl;
    // Dodaj tutaj odpowiednią obsługę dla własnego wywołania systemowego
}

class MemoryManager {
public:
    MemoryManager(int initialMemorySize) : totalMemory(initialMemorySize), remainingMemory(initialMemorySize) {
        memoryBlocks.push_back({0, initialMemorySize});
    }

    int allocateMemory(int size) {
        if (size <= remainingMemory) {
            for (auto& block : memoryBlocks) {
                if (block.free && block.size >= size) {
                    block.free = false;
                    remainingMemory -= size;
                    return block.startAddress;
                }
            }
        }
        return -1; // Memory allocation failed
    }

    void deallocateMemory(int startAddress) {
        for (auto& block : memoryBlocks) {
            if (!block.free && block.startAddress == startAddress) {
                block.free = true;
                remainingMemory += block.size;
                return;
            }
        }
    }

private:
    struct MemoryBlock {
        int startAddress;
        int size;
        bool free = true;
    };

    int totalMemory;
    int remainingMemory;
    std::vector<MemoryBlock> memoryBlocks;
};

// Create a memory manager with 64 KB of initial memory
MemoryManager memoryManager(64 * 1024); // 64 KB

void deallocateProcessMemory(const Process& process) {
    memoryManager.deallocateMemory(process.getStartAddress());
}

// Funkcja do zatrzymywania i dealokacji pamięci dla aktualnie aktywnego procesu
void processTermination(std::vector<Process>& processes) {
    std::lock_guard<std::mutex> lock(schedulerMutex);
    std::cout << "Zatrzymywanie aktualnie aktywnego procesu..." << std::endl;
    threads[activeProcessIndex].join();
    deallocateProcessMemory(processes[activeProcessIndex]);
    processes.erase(processes.begin() + activeProcessIndex);
    activeProcessIndex = 0;
    std::cout << "Proces został zatrzymany." << std::endl;
}

// Funkcja do wyświetlania informacji o aktualnym aktywnym procesie
void displayActiveProcessInfo(const std::vector<Process>& processes, int activeProcessIndex) {
    std::cout << "Aktualnie aktywny proces: " << processes[activeProcessIndex].getExecutionTime()
              << " (" << processes[activeProcessIndex].getMemorySize() << " bajtów pamięci)" << std::endl;
}


// Wprowadzenie przestrzeni nazw .NET Framework
#include <msclr\marshal_cppstd.h>

using namespace System;

// Klasa reprezentująca przykład interakcji z .NET Framework
class DotNetExample {
public:
    // Prosta metoda w języku C#, która wyświetla napis w konsoli
    void displayMessage(const std::string& message) {
        String^ managedMessage = gcnew String(message.c_str());
        Console::WriteLine(managedMessage);
    }

    // Metoda wykonująca akcję w języku C#
    void executeDotNetFunction() {
        displayMessage("Witaj ze strony .NET Framework!");
    }
};

// Funkcja wywołująca funkcję z .NET Framework
void callDotNetFunction() {
    DotNetExample dotNetExample;
    dotNetExample.executeDotNetFunction();
}

// Zdefiniuj urządzenie (device) i łańcuch wymiany (swap chain) DirectX 12
Microsoft::WRL::ComPtr<ID3D12Device> device;
Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
UINT descriptorSize;

void InitializeDirectX12(HWND hwnd) {
    // Tworzenie urządzenia (device) i łańcucha wymiany (swap chain)
    Microsoft::WRL::ComPtr<ID3D12Device> device;
    Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
    UINT descriptorSize;
    void InitializeDirectX12(HWND hwnd) {
    // Tworzenie obiektu urządzenia (device) DirectX 12
    D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device));

    // Konfiguracja łańcucha wymiany (swap chain)
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = 800; // Szerokość ekranu
    swapChainDesc.Height = 600; // Wysokość ekranu
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Format koloru
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc.Count = 1; // Liczba próbek
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2; // Liczba buforów w łańcuchu wymiany
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    // Tworzenie obiektu łańcucha wymiany (swap chain) DirectX 12
    dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, &swapChain);

    // Tworzenie sterty deskryptorów (descriptor heap) dla render targetów (RTV)
    D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
    rtvDescriptorHeapDesc.NumDescriptors = 2; // Liczba render targetów (buforów klatki)
    rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; // Typ sterty deskryptorów
    rtvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // Brak dodatkowych flag

    device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));

    // Pobranie rozmiaru deskryptora (wykorzystywane przy aktualizacji sterty deskryptorów)
    descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // Ustawienie deskryptorów render targetów (buforów klatki)
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < 2; i++) {
        swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
        device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, descriptorSize);
    }

    // Tworzenie alokatora komend dla komendów renderowania
    device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));

    // Tworzenie kolejki komend dla komend renderowania
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
    commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // Typ kolejki komend
    commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL; // Priorytet kolejki komend
    commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; // Brak dodatkowych flag
    commandQueueDesc.NodeMask = 0;

    device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));

    // Tworzenie obiektu stanu potoku (PSO) i wczytanie shaderów
void CreatePipelineStateObject() {
    
    // Funkcja do wczytywania i kompilowania shadera w formacie HLSL
Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(const std::wstring& filename, const std::string& entryPoint, const std::string& target) {
    Microsoft::WRL::ComPtr<ID3DBlob> shaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    // Kompilacja shadera za pomocą DirectX Shader Compiler (dxc)
    HRESULT hr = D3DCompileFromFile(filename.c_str(), nullptr, nullptr, entryPoint.c_str(), target.c_str(), 0, 0, &shaderBlob, &errorBlob);

    // Sprawdzenie, czy kompilacja się powiodła
    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA(static_cast<const char*>(errorBlob->GetBufferPointer()));
        }
        return nullptr;
    }

    return shaderBlob;
}

int main() {
    // Wczytanie i kompilacja shaderów
    Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob = CompileShader(L"VertexShader.hlsl", "VSMain", "vs_5_0");
    Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob = CompileShader(L"PixelShader.hlsl", "PSMain", "ps_5_0");

    if (!vertexShaderBlob || !pixelShaderBlob) {
        // Obsługa błędu wczytywania i kompilacji shaderów
        return 1;
    }

    // Definicja formatu danych wejściowych (input layout)
    D3D12_INPUT_ELEMENT_DESC inputElementDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // Konfiguracja stanu renderowania
    D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
    renderTargetBlendDesc.BlendEnable = FALSE;
    renderTargetBlendDesc.LogicOpEnable = FALSE;
    renderTargetBlendDesc.SrcBlend = D3D12_BLEND_ONE;
    renderTargetBlendDesc.DestBlend = D3D12_BLEND_ZERO;
    renderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
    renderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
    renderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
    renderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    renderTargetBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
    renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    D3D12_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    blendDesc.RenderTarget[0] = renderTargetBlendDesc;

    D3D12_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.MultisampleEnable = FALSE;
    rasterizerDesc.AntialiasedLineEnable = FALSE;
    rasterizerDesc.ForcedSampleCount = 0;
    rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    depthStencilDesc.StencilEnable = FALSE;
    depthStencilDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    depthStencilDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    depthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    depthStencilDesc.BackFace = depthStencilDesc.FrontFace;

    // Konfiguracja obiektu stanu potoku (PSO)
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDesc, _countof(inputElementDesc) };
    psoDesc.pRootSignature = rootSignature.Get(); // Zakładając, że istnieje obiekt klasy ComPtr<ID3D12RootSignature> o nazwie rootSignature
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
    psoDesc.RasterizerState = rasterizerDesc;
    psoDesc.BlendState = blendDesc;
    psoDesc.DepthStencilState = depthStencilDesc;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1; // Brak anti-aliasingu
    psoDesc.SampleDesc.Quality = 0;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

    // Tworzenie obiektu stanu potoku
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineStateObject)));
}

    // Tworzenie obiektu listy komend renderowania
    device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), pipelineState.Get(), IID_PPV_ARGS(&commandList));

    // Zakończenie listy komend (jeśli jest wymagane)
    // ...
}

void Render() {
    // Resetowanie alokatora komend i listy komend
    commandAllocator->Reset();
    commandList->Reset(commandAllocator.Get(), pipelineState.Get());

    // Ustawienie celów renderowania (render targets)
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart());
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Zakładamy, że mamy zmienną "device" typu Microsoft::WRL::ComPtr<ID3D12Device>, która reprezentuje nasze urządzenie DirectX 12

int main() {
    // Inicjalizacja urządzenia (device) i innych elementów (pomijamy to w tym przykładzie)
    // ...

    // Utworzenie obiektu viewport
    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX = 0; // Początkowy punkt x widoku
    viewport.TopLeftY = 0; // Początkowy punkt y widoku
    viewport.Width = 800; // Szerokość widoku
    viewport.Height = 600; // Wysokość widoku
    viewport.MinDepth = 0.0f; // Minimalna głębokość (0.0f dla całego zakresu)
    viewport.MaxDepth = 1.0f; // Maksymalna głębokość (1.0f dla całego zakresu)

    // Utworzenie prostokąta przycinania (scissor rect)
    D3D12_RECT scissorRect = {};
    scissorRect.left = 0; // Początkowy punkt x prostokąta przycinania
    scissorRect.top = 0; // Początkowy punkt y prostokąta przycinania
    scissorRect.right = 800; // Prawy punkt x prostokąta przycinania
    scissorRect.bottom = 600; // Dolny punkt y prostokąta przycinania

    // Utworzenie alokatora komend (command allocator)
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
    HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
    if (FAILED(hr)) {
        // Obsługa błędu, jeśli tworzenie alokatora komend nie powiodło się
        return -1;
    }

    // Utworzenie listy komend (command list)
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
    hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
    if (FAILED(hr)) {
        // Obsługa błędu, jeśli tworzenie listy komend nie powiodło się
        return -1;
    }

    // Dodatkowe kroki związane z inicjalizacją listy komend, np. przygotowanie buforów, ustawienie widoków, itp.
    // ...

    // Zakończenie listy komend
    commandList->Close();

    // Teraz lista komend jest gotowa do użycia i może być użyta do przekazywania poleceń renderowania do karty graficznej

    return 0;
}

    // Zdefiniowanie widoku i prostokąta przycinania na liście komend
    ID3D12GraphicsCommandList* commandList; // Załóżmy, że mamy już zmienną commandList reprezentującą listę komend

    commandList->RSSetViewports(1, &viewport); // Ustawienie viewportu
    commandList->RSSetScissorRects(1, &scissorRect); // Ustawienie prostokąta przycinania

    // Wywołanie polecenia rysowania (draw call) - renderowanie trójkąta
    // Załóżmy, że mamy wcześniej zdefiniowany bufor wierzchołków "vertexBuffer" i informacje o jego rozmiarze "vertexBufferSize"
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.SizeInBytes = vertexBufferSize;
    vertexBufferView.StrideInBytes = sizeof(Vertex); // Załóżmy, że struktura Vertex definiuje nasze dane wierzchołków

    // Ustawienie bufora wierzchołków
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

    // Ustawienie typu topologii (np. trójkąty)
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Wywołanie polecenia rysowania - renderowanie trójkąta z 3 wierzchołkami (od indeksu 0 do 2)
    commandList->DrawInstanced(3, 1, 0, 0);

    // Wywołanie innych poleceń renderowania, jeśli wymagane (np. rysowanie indeksowane)
    // ...

    // Zakończenie listy komend
    commandList->Close();

    // Wykonanie listy komend - przekazanie instrukcji renderowania do karty graficznej
    ID3D12CommandList* commandLists[] = { commandList.Get() };
    commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

    // Oczekiwanie na zakończenie wykonania listy komend (opcjonalne)
    // ...

    return 0;

     // Wywołanie funkcji Render, która będzie odpowiedzialna za rysowanie na ekranie
    Render();

    // Wykonanie listy komend - przekazanie instrukcji renderowania do karty graficznej
    ID3D12CommandList* commandLists[] = { commandList.Get() };
    commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

    // Oczekiwanie na zakończenie wykonania listy komend (opcjonalne)
    // ...

    return 0;
}

    // Wyczyszczenie celu renderowania (render target)
    const float clearColor[] = {0.0f, 0.0f, 0.0f, 1.0f};
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // Ustawienie stanu potoku (pipeline state)
    commandList->SetPipelineState(pipelineState.Get());

    // Ustawienie bufora wierzchołków i innych zasobów
    struct Vertex {
    float position[3];
    float color[4];
};

// Funkcja do ustawienia bufora wierzchołków
void SetVertexBuffers() {
    // Przykładowe dane wierzchołków
    Vertex vertices[] = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{0.0f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}
    };

    // Tworzenie bufora wierzchołków
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
    const UINT vertexBufferSize = sizeof(vertices);
    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&vertexBuffer)
    );

    // Kopiowanie danych wierzchołków do bufora
    UINT8* pVertexDataBegin;
    CD3DX12_RANGE readRange(0, 0); // Brak danych do odczytu - używane tylko do zapisu
    vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
    memcpy(pVertexDataBegin, vertices, vertexBufferSize);
    vertexBuffer->Unmap(0, nullptr);

    // Ustawienie widoku bufora wierzchołków
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = sizeof(Vertex);
    vertexBufferView.SizeInBytes = vertexBufferSize;

    // Ustawienie bufora wierzchołków na liście komend
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
}

// Wykonanie listy komend - przekazanie instrukcji renderowania do karty graficznej
    ID3D12CommandList* commandLists[] = { commandList.Get() };
    commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

    // Oczekiwanie na zakończenie wykonania listy komend (opcjonalne)
    // ...

    return 0;
}

    // Wywołanie polecenia rysowania (draw calls) i innych poleceń renderowania
    commandList->DrawInstanced(3, 1, 0, 0); // Wywołujemy polecenie rysowania trójkąta, składającego się z 3 wierzchołków

    // Wywołanie funkcji Render, która będzie odpowiedzialna za rysowanie na ekranie
    Render();

    // Wykonanie listy komend - przekazanie instrukcji renderowania do karty graficznej
    ID3D12CommandList* commandLists[] = { commandList.Get() };
    commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);


    // Wyświetlenie ramki
    commandList->Close();
    ID3D12CommandList* commandLists[] = {commandList.Get()};
    commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
    swapChain->Present(1, 0);
}

    // Tworzenie sterty deskryptorów (descriptor heap)
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
    
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = 100; // Określamy liczbę deskryptorów w stercie
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // Określamy typ deskryptorów (stałych buforów, widoków i zasobów)
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // Opcjonalne flagi, np. D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE

    device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeap));

    // Teraz możemy używać "descriptorHeap" w naszym programie do przechowywania i zarządzania deskryptorami.

    return 0;

    // Tworzenie kolejki komend (command queue)
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // Określamy typ kolejki komend (direct, bundle, compute, copy)
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL; // Opcjonalnie: priorytet kolejki
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; // Opcjonalne flagi

    device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));

    // Tworzenie bufora alokatorów komend (command allocator)
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;

    D3D12_COMMAND_ALLOCATOR_DESC allocatorDesc = {};
    allocatorDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // Określamy typ bufora alokatorów komend (direct, bundle, compute, copy)

    device->CreateCommandAllocator(&allocatorDesc, IID_PPV_ARGS(&commandAllocator));

    // Tworzenie listy komend (command list)
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;

    device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));

    // Teraz możemy używać "commandQueue", "commandAllocator" i "commandList" w naszym programie do przygotowywania i wysyłania poleceń renderowania do GPU.

    return 0;

    // Wczytanie i kompilacja shaderów
    Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob;

    // Shader źródłowy w formacie HLSL
    LPCWSTR vertexShaderSource = L"VertexShader.hlsl";
    LPCWSTR pixelShaderSource = L"PixelShader.hlsl";

    // Kompilacja shadera w formacie HLSL do odpowiedniego formatu binarnego
    D3DCompileFromFile(vertexShaderSource, nullptr, nullptr, "main", "vs_5_0", 0, 0, &vertexShaderBlob, nullptr);
    D3DCompileFromFile(pixelShaderSource, nullptr, nullptr, "main", "ps_5_0", 0, 0, &pixelShaderBlob, nullptr);

    // Konfiguracja obiektu stanu potoku (PSO)
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = {}; // Opcjonalnie: konfiguracja formatów danych wejściowych
    psoDesc.pRootSignature = nullptr; // Opcjonalnie: korzeń podpisu dla shaderów
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get()); // Vertex shader
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get()); // Pixel shader
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // Opcjonalnie: konfiguracja stanu rasterizacji
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // Opcjonalnie: konfiguracja stanu mieszania kolorów
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // Opcjonalnie: konfiguracja stanu bufora głębokości i testu szablonowego
    psoDesc.SampleMask = UINT_MAX; // Opcjonalnie: maska próbkowania
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // Opcjonalnie: typ topologii prymitywu
    psoDesc.NumRenderTargets = 1; // Opcjonalnie: liczba celów renderowania
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // Opcjonalnie: format render target
    psoDesc.SampleDesc.Count = 1; // Opcjonalnie: liczba próbek (anti-aliasing)
    psoDesc.SampleDesc.Quality = 0; // Opcjonalnie: jakość próbkowania

    // Tworzenie obiektu stanu potoku (PSO)
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
    device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));

    // Teraz możemy używać "pipelineState" w naszym programie do renderowania.

    return 0;

    // Załóżmy, że mamy zdefiniowany wektor z danymi wierzchołków o strukturze Vertex, a także zmienną "device" typu Microsoft::WRL::ComPtr<ID3D12Device> reprezentującą nasze urządzenie DirectX 12.

struct Vertex {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 normal;
    DirectX::XMFLOAT2 texCoord;
};

std::vector<Vertex> vertices = {
    // Dodaj dane wierzchołków tutaj
};

const UINT vertexBufferSize = sizeof(Vertex) * vertices.size();

// Utworzenie bufora wierzchołków
Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUploadHeap;

// Tworzenie zasobu na GPU (bufora wierzchołków)
device->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // Właściwości sterty (domyślna sterta)
    D3D12_HEAP_FLAG_NONE, // Dodatkowe flagi sterty (brak)
    &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), // Opis zasobu (bufora wierzchołków)
    D3D12_RESOURCE_STATE_COPY_DEST, // Początkowy stan zasobu (kopia docelowa)
    nullptr,
    IID_PPV_ARGS(&vertexBuffer)
);

// Tworzenie zasobu na CPU (bufora wierzchołków) do zapisu danych
device->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // Właściwości sterty (sterta w pamięci RAM, z której możliwe są kopiowanie)
    D3D12_HEAP_FLAG_NONE, // Dodatkowe flagi sterty (brak)
    &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), // Opis zasobu (bufora wierzchołków)
    D3D12_RESOURCE_STATE_GENERIC_READ, // Początkowy stan zasobu (możliwość odczytu przez GPU)
    nullptr,
    IID_PPV_ARGS(&vertexBufferUploadHeap)
);

// Skopiowanie danych wierzchołków do bufora na CPU
UINT8* pVertexDataBegin;
CD3DX12_RANGE readRange(0, 0);
vertexBufferUploadHeap->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
memcpy(pVertexDataBegin, vertices.data(), vertexBufferSize);
vertexBufferUploadHeap->Unmap(0, nullptr);

// Przygotowanie komendy kopiowania danych z bufora na CPU do bufora na GPU
Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
commandList->CopyBufferRegion(vertexBuffer.Get(), 0, vertexBufferUploadHeap.Get(), 0, vertexBufferSize);

// Zakończenie i wykonanie komendy
commandList->Close();
ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

// Oczekiwanie na zakończenie komendy (może być wymagane w celu synchronizacji)
// ...

// Ustawienie bufora wierzchołków do rysowania
D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
vertexBufferView.StrideInBytes = sizeof(Vertex);
vertexBufferView.SizeInBytes = vertexBufferSize;

// Załóżmy, że mamy zdefiniowane dane do stałego bufora (np. macierze) o strukturze ConstantBuffer, a także zmienną "device" typu Microsoft::WRL::ComPtr<ID3D12Device> reprezentującą nasze urządzenie DirectX 12.

struct ConstantBuffer {
    DirectX::XMFLOAT4X4 worldMatrix;
    DirectX::XMFLOAT4X4 viewMatrix;
    DirectX::XMFLOAT4X4 projectionMatrix;
};

ConstantBuffer constantBufferData; // Dane stałego bufora
// Zainicjuj dane w stałym buforze constantBufferData tutaj

const UINT constantBufferSize = sizeof(ConstantBuffer);

// Utworzenie stałego bufora
Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer;
Microsoft::WRL::ComPtr<ID3D12Resource> constantBufferUploadHeap;

// Tworzenie zasobu na GPU (stałego bufora)
device->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // Właściwości sterty (domyślna sterta)
    D3D12_HEAP_FLAG_NONE, // Dodatkowe flagi sterty (brak)
    &CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize), // Opis zasobu (stały bufor)
    D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, // Początkowy stan zasobu (bufor wierzchołków i stałych)
    nullptr,
    IID_PPV_ARGS(&constantBuffer)
);

// Tworzenie zasobu na CPU (stałego bufora) do zapisu danych
device->CreateCommittedResource(
    &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // Właściwości sterty (sterta w pamięci RAM, z której możliwe są kopiowanie)
    D3D12_HEAP_FLAG_NONE, // Dodatkowe flagi sterty (brak)
    &CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize), // Opis zasobu (stały bufor)
    D3D12_RESOURCE_STATE_GENERIC_READ, // Początkowy stan zasobu (możliwość odczytu przez GPU)
    nullptr,
    IID_PPV_ARGS(&constantBufferUploadHeap)
);

// Skopiowanie danych stałego bufora do bufora na CPU
UINT8* pConstantBufferDataBegin;
constantBufferUploadHeap->Map(0, nullptr, reinterpret_cast<void**>(&pConstantBufferDataBegin));
memcpy(pConstantBufferDataBegin, &constantBufferData, constantBufferSize);
constantBufferUploadHeap->Unmap(0, nullptr);

// Przygotowanie komendy kopiowania danych z bufora na CPU do bufora na GPU
Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
commandList->CopyBufferRegion(constantBuffer.Get(), 0, constantBufferUploadHeap.Get(), 0, constantBufferSize);

// Zakończenie i wykonanie komendy
commandList->Close();
ID3D12CommandList* ppCommandLists[] = { commandList.Get() };
commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

// Oczekiwanie na zakończenie komendy (może być wymagane w celu synchronizacji)
// ...

// Ustawienie stałego bufora do rysowania
D3D12_CONSTANT_BUFFER_VIEW_DESC constantBufferViewDesc = {};
constantBufferViewDesc.BufferLocation = constantBuffer->GetGPUVirtualAddress();
constantBufferViewDesc.SizeInBytes = constantBufferSize;
device->CreateConstantBufferView(&constantBufferViewDesc, constantBufferViewHeap->GetCPUDescriptorHandleForHeapStart());


    // Pobranie rozmiaru deskryptora (wykorzystywane przy aktualizacji sterty deskryptorów)
    descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}
void Render() {
    // Resetowanie alokatora komend i listy komend
    // Załóżmy, że mamy zmienną "commandAllocator" typu Microsoft::WRL::ComPtr<ID3D12CommandAllocator> reprezentującą nasz alokator komend.

    // Resetowanie alokatora komend
    commandAllocator->Reset();

    // Załóżmy, że mamy zmienną "commandList" typu Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> reprezentującą naszą listę komend.

    // Zakończenie aktywnej listy komend przed resetowaniem
    commandList->Close();

    // Resetowanie listy komend
    commandList->Reset(commandAllocator.Get(), nullptr);

    // Załóżmy, że mamy zmienną "swapChain" typu Microsoft::WRL::ComPtr<IDXGISwapChain3> reprezentującą nasz łańcuch wymiany (swap chain).
    // Załóżmy również, że mamy zmienną "rtvDescriptorSize" typu UINT, która przechowuje rozmiar deskryptora render target view (RTV) w bajtach.

    // Ilość render targetów (można ustawić dowolną liczbę, ale na ogół używa się 1 dla pojedynczego celu renderowania).
    const int numRenderTargets = 1;

    // Tablica zawierająca deskryptory render target view (RTV) dla każdego render targetu.
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> renderTargets(numRenderTargets);

    // Tworzenie sterty deskryptorów (descriptor heap) dla render targetów.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = numRenderTargets;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));

    // Uzyskanie rozmiaru deskryptora RTV.
    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // Pobranie wskaźniku na obiekt wymiany (buffer) w celu pobrania render targetów.
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

    // Pobranie render targetów z łańcucha wymiany (swap chain) i zapisanie ich w tablicy renderTargets.
    for (int i = 0; i < numRenderTargets; i++) {
        swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));

    // Przypisanie render targeta do odpowiedniego deskryptora RTV.
    device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);
    rtvHandle.Offset(1, rtvDescriptorSize);
}

    // Tutaj możemy również ustawić dodatkowe informacje o render targetach, takie jak ich format, rozmiar, itp., ale na ogół są one konfigurowane podczas tworzenia swap chaina.


    // Ustawienie widoku (viewport) i prostokąta przycinania (scissor rect)
    // Definicje struktur D3D12_VIEWPORT i D3D12_RECT
    D3D12_VIEWPORT viewport;
    D3D12_RECT scissorRect;

    // Konfiguracja wartości widoku (viewport)
    viewport.TopLeftX = 0.0f;       // Początkowa pozycja X widoku na ekranie
    viewport.TopLeftY = 0.0f;       // Początkowa pozycja Y widoku na ekranie
    viewport.Width = screenWidth;  // Szerokość widoku
    viewport.Height = screenHeight;// Wysokość widoku
    viewport.MinDepth = 0.0f;       // Minimalna głębokość widoku (0 - najbliższy punkt)
    viewport.MaxDepth = 1.0f;       // Maksymalna głębokość widoku (1 - najdalszy punkt)

    // Konfiguracja wartości prostokąta przycinania (scissor rect)
    scissorRect.left = 0;           // Początkowa pozycja X prostokąta przycinania
    scissorRect.top = 0;            // Początkowa pozycja Y prostokąta przycinania
    scissorRect.right = screenWidth;// Prawa krawędź prostokąta przycinania
    scissorRect.bottom = screenHeight;// Dolna krawędź prostokąta przycinania

    // Przypisanie widoku i prostokąta przycinania do listy komend
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);


    // Wyczyszczenie celu renderowania (render target)
    // Definicja koloru do wyczyszczenia render targetu (RGBA)
    const float clearColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f }; // Czerwony kolor (RGB) + alpha = 1.0 (pełna nieprzezroczystość)

    // Ustawienie render targetu jako bieżącego celu renderowania
    commandList->OMSetRenderTargets(1, &renderTargetViewDescriptorHandle, FALSE, nullptr);

    // Wyczyszczenie render targetu komendą ClearRenderTargetView
    commandList->ClearRenderTargetView(renderTargetViewDescriptorHandle, clearColor, 0, nullptr);


    // Ustawienie stanu potoku (pipeline state)
    // Tworzenie obiektu stanu potoku (Pipeline State Object - PSO)
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    ZeroMemory(&psoDesc, sizeof(psoDesc));

    // Ustawienia dla głównego potoku (main pipeline)
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // Typ topologii - tutaj trójkąty
    psoDesc.InputLayout = { inputElementDescs.data(), static_cast<UINT>(inputElementDescs.size()) }; // Opis wejścia wierzchołków
    psoDesc.pRootSignature = rootSignature.Get(); // Sygnatura korzenia (Root Signature) - definiuje dostępne stałe bufory i inne dane globalne
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get()); // Shader wierzchołków
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get()); // Shader pikseli
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // Stan rasterizatora - użyj domyślnych ustawień
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // Stan mieszania (blend state) - użyj domyślnych ustawień
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT); // Stan testu głębokości - użyj domyślnych ustawień
    psoDesc.SampleMask = UINT_MAX; // Ustawienie maski próbkowania - użyj wszystkich dostępnych próbek
    psoDesc.PrimitiveRestartEnable = FALSE; // Wyłączenie restartowania topologii

    // Ustawienie konfiguracji wyjścia render targetu (render target format i liczba próbek)
    psoDesc.NumRenderTargets = 1; // Jedno celu renderowania
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // Format render targetu (RGBA 8-bit UNORM)
    psoDesc.SampleDesc.Count = 1; // Liczba próbek (bez antyaliasingu)

    // Utworzenie obiektu stanu potoku
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
    device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));


    // Ustawienie bufora wierzchołków i innych zasobów
    // Tworzenie bufora wierzchołków
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
    UINT vertexBufferSize = sizeof(Vertex) * vertexCount; // Załóżmy, że mamy zdefiniowaną strukturę Vertex i liczbę wierzchołków vertexCount

    // Opis bufora wierzchołków
    D3D12_RESOURCE_DESC vertexBufferDesc = {};
    vertexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    vertexBufferDesc.Alignment = 0;
    vertexBufferDesc.Width = vertexBufferSize;
    vertexBufferDesc.Height = 1;
    vertexBufferDesc.DepthOrArraySize = 1;
    vertexBufferDesc.MipLevels = 1;
    vertexBufferDesc.Format = DXGI_FORMAT_UNKNOWN; // Format bufora wierzchołków (nieznany, bo to jest bufor, nie tekstura)
    vertexBufferDesc.SampleDesc.Count = 1; // Liczba próbek (bez antyaliasingu)
    vertexBufferDesc.SampleDesc.Quality = 0;
    vertexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    vertexBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    // Utworzenie bufora wierzchołków
    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // Właściwości sterty bufora (domyślna sterta)
        D3D12_HEAP_FLAG_NONE, // Brak flag dla sterty
        &vertexBufferDesc, // Opis zasobu
        D3D12_RESOURCE_STATE_COPY_DEST, // Stan początkowy - jako cel kopiowania
        nullptr, // Opcjonalnie, zaawansowane opcje stworzenia (nullptr dla braku)
        IID_PPV_ARGS(&vertexBuffer) // Uchwyt do utworzonego bufora wierzchołków
    );

    // Tworzenie bufora indeksów (jeśli korzystamy z indeksowanej geometrii)
    Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;
    UINT indexBufferSize = sizeof(UINT) * indexCount; // Załóżmy, że mamy liczbę indeksów indexCount

    // Opis bufora indeksów
    D3D12_RESOURCE_DESC indexBufferDesc = {};
    indexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    indexBufferDesc.Alignment = 0;
    indexBufferDesc.Width = indexBufferSize;
    indexBufferDesc.Height = 1;
    indexBufferDesc.DepthOrArraySize = 1;
    indexBufferDesc.MipLevels = 1;
    indexBufferDesc.Format = DXGI_FORMAT_UNKNOWN; // Format bufora indeksów (nieznany, bo to jest bufor, nie tekstura)
    indexBufferDesc.SampleDesc.Count = 1; // Liczba próbek (bez antyaliasingu)
    indexBufferDesc.SampleDesc.Quality = 0;
    indexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    indexBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    // Utworzenie bufora indeksów
    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // Właściwości sterty bufora (domyślna sterta)
        D3D12_HEAP_FLAG_NONE, // Brak flag dla sterty
        &indexBufferDesc, // Opis zasobu
        D3D12_RESOURCE_STATE_COPY_DEST, // Stan początkowy - jako cel kopiowania
        nullptr, // Opcjonalnie, zaawansowane opcje stworzenia (nullptr dla braku)
        IID_PPV_ARGS(&indexBuffer) // Uchwyt do utworzonego bufora indeksów
    );

    // Przygotowanie danych wierzchołków i indeksów (np. kopiowanie z CPU do GPU)
    // ...

    // Utworzenie bufora zasobów (stałej bufora)
    Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer;
    UINT constantBufferSize = sizeof(ConstantBufferData); // Zakładamy, że mamy zdefiniowaną stałą bufora ConstantBufferData

    // Opis stałej bufora
    D3D12_RESOURCE_DESC constantBufferDesc = {};
    constantBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    constantBufferDesc.Alignment = 0;
    constantBufferDesc.Width = constantBufferSize;
    constantBufferDesc.Height = 1;
    constantBufferDesc.DepthOrArraySize = 1;
    constantBufferDesc.MipLevels = 1;
    constantBufferDesc.Format = DXGI_FORMAT_UNKNOWN; // Format stałej bufora (nieznany, bo to jest bufor, nie tekstura)
    constantBufferDesc.SampleDesc.Count = 1; // Liczba próbek (bez antyaliasingu)
    constantBufferDesc.SampleDesc.Quality = 0;
    constantBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    constantBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    // Utworzenie stałej bufora
    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // Właściwości sterty bufora (sterta przeznaczona do ładowania z CPU)
        D3D12_HEAP_FLAG_NONE, // Brak flag dla sterty
        &constantBufferDesc, // Opis zasobu
        D3D12_RESOURCE_STATE_GENERIC_READ, // Stan początkowy - dostęp do CPU
        nullptr, // Opcjonalnie, zaawansowane opcje stworzenia (nullptr dla braku)
        IID_PPV_ARGS(&constantBuffer) // Uchwyt do utworzonego bufora stałej
    );

    // Odwzorowanie danych z CPU do GPU stałej bufora
    ConstantBufferData constantBufferData; // Zakładamy, że mamy strukturę ConstantBufferData
    // Przygotowanie danych w stałej buforze (np. przekopiowanie z CPU do GPU)
    void* pConstantBufferData;
    constantBuffer->Map(0, nullptr, &pConstantBufferData);
    memcpy(pConstantBufferData, &constantBufferData, sizeof(ConstantBufferData));
    constantBuffer->Unmap(0, nullptr);

    // Inne zasoby (np. tekstury, bufory obiektów, bufory stałe itp.)
    // Tworzenie tekstury 2D
    Microsoft::WRL::ComPtr<ID3D12Resource> texture2D;
    UINT textureWidth = 512;
    UINT textureHeight = 512;
    UINT texturePixelSize = 4; // 4 bajty na piksel (RGBA)
    UINT textureBufferSize = textureWidth * textureHeight * texturePixelSize;

    // Opis tekstury 2D
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    textureDesc.Alignment = 0;
    textureDesc.Width = textureWidth;
    textureDesc.Height = textureHeight;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Załóżmy, że używamy formatu RGBA8 UNORM
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    // Utworzenie tekstury 2D
    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // Właściwości sterty bufora (domyślna sterta)
        D3D12_HEAP_FLAG_NONE, // Brak flag dla sterty
        &textureDesc, // Opis zasobu
        D3D12_RESOURCE_STATE_COPY_DEST, // Stan początkowy - jako cel kopiowania
        nullptr, // Opcjonalnie, zaawansowane opcje stworzenia (nullptr dla braku)
        IID_PPV_ARGS(&texture2D) // Uchwyt do utworzonej tekstury 2D
    );

    // Przygotowanie danych dla tekstury 2D (np. kopiowanie z CPU do GPU)
    // ...

    // Inicjalizacja DirectX 12

    // Tworzenie obiektu urządzenia (device)
    Microsoft::WRL::ComPtr<ID3D12Device> device;
    D3D12CreateDevice(/*...*/, IID_PPV_ARGS(&device));

    // Tworzenie bufora tekstury 3D
    const UINT width = 128;
    const UINT height = 128;
    const UINT depth = 128;
    const DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM; // Przykładowy format pikseli

    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
    texDesc.Alignment = 0;
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.DepthOrArraySize = depth;
    texDesc.MipLevels = 1;
    texDesc.Format = format;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    Microsoft::WRL::ComPtr<ID3D12Resource> texture3D;
    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&texture3D));

    // Opcjonalnie, możesz załadować dane do tekstury

    // Przygotowanie do renderowania

    // Stworzenie widoku tekstury 3D
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
    srvDesc.Texture3D.MipLevels = 1;
    srvDesc.Texture3D.MostDetailedMip = 0;
    srvDesc.Texture3D.ResourceMinLODClamp = 0.0f;

    // Pamiętaj, aby alokować pamięć dla deskryptorów
    UINT descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    D3D12_CPU_DESCRIPTOR_HANDLE srvHandle; // Ustawianie tej zmiennej za pomocą odpowiedniego deskryptora

    // Ustawienie deskryptora widoku tekstury 3D
    device->CreateShaderResourceView(texture3D.Get(), &srvDesc, srvHandle);

    // Renderowanie

    // Tworzenie dodatkowego bufora stałego
    Microsoft::WRL::ComPtr<ID3D12Resource> additionalConstantBuffer;
    UINT additionalConstantBufferSize = sizeof(AdditionalConstantBufferData); // Zakładamy, że mamy zdefiniowaną stałą bufora AdditionalConstantBufferData

    // Opis dodatkowego bufora stałego
    D3D12_RESOURCE_DESC additionalConstantBufferDesc = {};
    additionalConstantBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    additionalConstantBufferDesc.Alignment = 0;
    additionalConstantBufferDesc.Width = additionalConstantBufferSize;
    additionalConstantBufferDesc.Height = 1;
    additionalConstantBufferDesc.DepthOrArraySize = 1;
    additionalConstantBufferDesc.MipLevels = 1;
    additionalConstantBufferDesc.Format = DXGI_FORMAT_UNKNOWN; // Format bufora (nieznany, bo to jest bufor, nie tekstura)
    additionalConstantBufferDesc.SampleDesc.Count = 1; // Liczba próbek (bez antyaliasingu)
    additionalConstantBufferDesc.SampleDesc.Quality = 0;
    additionalConstantBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    additionalConstantBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    // Utworzenie dodatkowego bufora stałego
    device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // Właściwości sterty bufora (sterta przeznaczona do ładowania z CPU)
        D3D12_HEAP_FLAG_NONE, // Brak flag dla sterty
        &additionalConstantBufferDesc, // Opis zasobu
        D3D12_RESOURCE_STATE_GENERIC_READ, // Stan początkowy - dostęp do CPU
        nullptr, // Opcjonalnie, zaawansowane opcje stworzenia (nullptr dla braku)
        IID_PPV_ARGS(&additionalConstantBuffer) // Uchwyt do utworzonego bufora stałego
    );

    // Odwzorowanie danych z CPU do GPU dodatkowego bufora stałego
    AdditionalConstantBufferData additionalConstantBufferData; // Zakładamy, że mamy strukturę AdditionalConstantBufferData
    // Przygotowanie danych w dodatkowym buforze stałym (np. przekopiowanie z CPU do GPU)
    void* pAdditionalConstantBufferData;
    additionalConstantBuffer->Map(0, nullptr, &pAdditionalConstantBufferData);
    memcpy(pAdditionalConstantBufferData, &additionalConstantBufferData, sizeof(AdditionalConstantBufferData));
    additionalConstantBuffer->Unmap(0, nullptr);

    // Dodatkowe zasoby (np. tekstury, bufory obiektów, bufory stałe itp.)
    // ...




    // Wywołanie polecenia rysowania (draw calls) i innych poleceń renderowania
    // Załóżmy, że wcześniej utworzyliśmy i skonfigurowaliśmy obiekt stanu potoku (PSO)
    // oraz załadowaliśmy dane do bufora wierzchołków i stałej bufora.

    // Przygotowanie polecenia rysowania
    D3D12_VIEWPORT viewport; // Przygotowanie widoku (viewport) - wcześniej zdefiniowany
    D3D12_RECT scissorRect; // Przygotowanie prostokąta przycinania (scissor rect) - wcześniej zdefiniowany

    // Pobranie alokatora komend dla kolejnego frame'a (assumption: mamy go zaimplementowanego)
    ID3D12CommandAllocator* commandAllocator = GetNextCommandAllocator();
    commandAllocator->Reset(); // Resetowanie alokatora komend

    // Ustawienie stanu alokatora komend jako aktywny
    // (commandListType to rodzaj komend - DIRECT, BUNDLE, COMPUTE, COPY)
    ID3D12GraphicsCommandList* commandList;
    device->CreateCommandList(0, commandListType, commandAllocator, pipelineState, IID_PPV_ARGS(&commandList));
    commandList->SetGraphicsRootSignature(rootSignature); // Ustawienie korzeniowego podpisu dla potoku graficznego
    commandList->RSSetViewports(1, &viewport); // Ustawienie widoku
    commandList->RSSetScissorRects(1, &scissorRect); // Ustawienie prostokąta przycinania

    // Przygotowanie polecenia rysowania
    commandList->SetPipelineState(pipelineState); // Ustawienie stanu potoku
    commandList->SetGraphicsRootConstantBufferView(0, constantBuffer->GetGPUVirtualAddress()); // Ustawienie stałej bufora

    // Ustawienie bufora wierzchołków (jeśli istnieje)
    if (vertexBuffer != nullptr)
    {
        D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
        vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
        vertexBufferView.SizeInBytes = vertexBufferSize; // Rozmiar bufora wierzchołków
        vertexBufferView.StrideInBytes = vertexStride; // Rozmiar pojedynczego wierzchołka (stride)
        commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // Ustawienie bufora wierzchołków w potoku
    }

    // Inne ustawienia, np. ustawienie tekstury, bufora indeksów, innych stałych buforów itp.

    // Wywołanie polecenia rysowania (Draw Call)
    commandList->DrawInstanced(vertexCount, instanceCount, startVertexLocation, startInstanceLocation);

    // Inne wywołania, np. rysowanie indeksowane (Indexed Draw Call), rysowanie instancyjne (Instanced Draw Call), rysowanie indeksowane i instancyjne itp.

    // Zakończenie listy komend
    commandList->Close();

    // Wykonanie listy komend
    ID3D12CommandList* ppCommandLists[] = { commandList };
    commandQueue->ExecuteCommandLists(1, ppCommandLists);

    // Przypisanie alokatora komend do kolejki, aby możliwe było ponowne użycie w przyszłości
    // (assumption: mamy tę funkcję zaimplementowaną)
    QueueCommandAllocatorForNextFrame(commandAllocator);

    // Wykonanie renderingu
    Render();


    // Wyświetlenie ramki
    // Rozmiar okna
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;
    // Inicjalizacja biblioteki GLFW
    if (!glfwInit()) {
        std::cerr << "Błąd: Nie można zainicjować GLFW." << std::endl;
        return -1;
    }

    // Ustawienie wersji OpenGL (3.3 w tym przypadku)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // Tworzenie okna
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Wyświetlenie ramki w C++", nullptr, nullptr);
    if (!window) {
        std::cerr << "Błąd: Nie można utworzyć okna GLFW." << std::endl;
        glfwTerminate();
        return -1;
    }

    // Ustawienie kontekstu OpenGL dla okna
    glfwMakeContextCurrent(window);

    // Główna pętla programu
    while (!glfwWindowShouldClose(window)) {
        // Wyczyszczenie bufora koloru (czyszczenie ekranu)
        glClear(GL_COLOR_BUFFER_BIT);

        // Renderowanie tutaj - np. wyświetlenie ramki
        glBegin(GL_LINE_LOOP);
        glVertex2f(-0.5f, -0.5f);
        glVertex2f(-0.5f, 0.5f);
        glVertex2f(0.5f, 0.5f);
        glVertex2f(0.5f, -0.5f);
        glEnd();

        // Wyświetlenie rysunku na ekranie
        glfwSwapBuffers(window);

        // Obsługa zdarzeń okna
        glfwPollEvents();
    }

    // Zakończenie działania GLFW
    glfwTerminate();
    return 0;
}

// Pętla renderowania
void RunRenderingLoop() {
    while (true) {
        // Obsługa komunikatów okna (jeśli jest to wymagane)
        // ...

        // Wywołanie funkcji renderowania
        Render();

        // Opcjonalnie, ograniczenie ilości klatek na sekundę (aby zapobiec dużej użyciu CPU)
        // Rozmiar okna
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Liczba docelowa klatek na sekundę (FPS)
    const int TARGET_FPS = 60;
    // Inicjalizacja biblioteki GLFW
    if (!glfwInit()) {
        std::cerr << "Błąd: Nie można zainicjować GLFW." << std::endl;
        return -1;
    }

    // Ustawienie wersji OpenGL (3.3 w tym przypadku)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // Tworzenie okna
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Wyświetlenie ramki w C++", nullptr, nullptr);
    if (!window) {
        std::cerr << "Błąd: Nie można utworzyć okna GLFW." << std::endl;
        glfwTerminate();
        return -1;
    }

    // Ustawienie kontekstu OpenGL dla okna
    glfwMakeContextCurrent(window);

    // Ustawienie docelowej liczby klatek na sekundę (FPS)
    glfwSwapInterval(1); // Ustawienie na 1, aby ograniczyć FPS do docelowej wartości

    // Główna pętla programu
    while (!glfwWindowShouldClose(window)) {
        // Wyczyszczenie bufora koloru (czyszczenie ekranu)
        glClear(GL_COLOR_BUFFER_BIT);

        // Renderowanie tutaj - np. wyświetlenie ramki
        glBegin(GL_LINE_LOOP);
        glVertex2f(-0.5f, -0.5f);
        glVertex2f(-0.5f, 0.5f);
        glVertex2f(0.5f, 0.5f);
        glVertex2f(0.5f, -0.5f);
        glEnd();

        // Wyświetlenie rysunku na ekranie
        glfwSwapBuffers(window);

        // Oczekiwanie, aby ograniczyć ilość klatek na sekundę
        glfwWaitEventsTimeout(1.0 / TARGET_FPS);
    }

    // Zakończenie działania GLFW
    glfwTerminate();
    return 0;

        // std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}
