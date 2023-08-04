#ifdef _WIN32
#include <Windows.h>
#endif

int main() {
#ifdef _WIN32
    // Ustawienie wartości rejestru systemowego (Windows)
    HKEY hKey;
    const char* keyPath = "SOFTWARE\\MyApp";
    const char* valueName = "MySetting";
    const char* valueData = "Hello, World!";
    if (RegCreateKeyExA(HKEY_CURRENT_USER, keyPath, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, valueName, 0, REG_SZ, reinterpret_cast<const BYTE*>(valueData), strlen(valueData) + 1);
        RegCloseKey(hKey);
    }

    // Odczytanie wartości z rejestru
    char buffer[256];
    DWORD bufferSize = sizeof(buffer);
    if (RegGetValueA(HKEY_CURRENT_USER, keyPath, valueName, RRF_RT_REG_SZ, nullptr, buffer, &bufferSize) == ERROR_SUCCESS) {
        std::cout << "Wartość z rejestru: " << buffer << std::endl;
    } else {
        std::cout << "Nie udało się odczytać wartości z rejestru." << std::endl;
    }
#endif

    return 0;
}
