#include <pch.h>
#include "Core/Runtime/Runtime.h"

int main()
{
    srand(static_cast<unsigned>(time(nullptr)));

    RuntimeSystem()->Run();

    // Leave the console open until the user presses a key
    std::cout << "Press any key to exit..." << std::endl;
    std::cin.get();
    return 0;
}
