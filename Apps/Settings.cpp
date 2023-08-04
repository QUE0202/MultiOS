#include <iostream>
#include <locale>
#include <cstdlib>

int main() {
    // Ustawienie lokalizacji na angielską (USA)
    std::locale::global(std::locale("en_US.utf8"));

    // Wyświetlenie daty w formacie zależnym od lokalizacji
    std::time_t now = std::time(nullptr);
    std::cout << "Aktualna data: " << std::put_time(std::localtime(&now), "%c") << std::endl;

    // Ustawienie zmiennej środowiskowej
    std::string variableName = "MY_VARIABLE";
    std::string variableValue = "Hello, World!";
    setenv(variableName.c_str(), variableValue.c_str(), 1);

    // Pobranie wartości zmiennej środowiskowej
    char* value = getenv(variableName.c_str());
    if (value) {
        std::cout << "Wartość zmiennej środowiskowej " << variableName << ": " << value << std::endl;
    } else {
        std::cout << "Zmienna środowiskowa " << variableName << " nie została ustawiona." << std::endl;
    }



    return 0;
}
