#ifndef INTERRUPT_HANDLER_H
#define INTERRUPT_HANDLER_H

#include <iostream>
#include <chrono>
#include <thread>

// Funkcja obsługująca przerwania systemowe
inline void systemCallHandler() {
    std::cout << "Przerwanie systemowe - zmiana aktywnego procesu!" << std::endl;
}

// Funkcja obsługująca przerwania sprzętowe (np. od timera lub innego urządzenia)
inline void hardwareInterruptHandler() {
    // Symulacja obsługi przerwania
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Przerwanie sprzętowe - zmiana aktywnego procesu!" << std::endl;
        // Wywołanie przerwania systemowego (int 0x20) do zmiany aktywnego procesu
        asm volatile("int $0x20");
    }
}

#endif // INTERRUPT_HANDLER_H
