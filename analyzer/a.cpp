#include <iostream>
#include <cctype>

int main() {
    char c1 = 'A';
    char c2 = '9';
    char c3 = '=';

    // Safely checking characters by casting to unsigned char
    if (std::isalnum(static_cast<unsigned char>(c1))) {
        std::cout << c1 << " is alphanumeric\n";
    }
    
    if (std::isalnum(static_cast<unsigned char>(c2))) {
        std::cout << c2 << " is alphanumeric\n";
    }

    if (!std::isalnum(static_cast<unsigned char>(c3))) {
        std::cout << c3 << " is not alphanumeric\n";
    }

    return 0;
}
