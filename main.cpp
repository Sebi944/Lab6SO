#include <windows.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>

void createChildProcesses(std::vector<std::pair<HANDLE, HANDLE>>& pipes, int start, int end, int step) {
    for (int i = 0; i < 10; ++i) {
        int rangeStart = start + i * step;
        int rangeEnd = rangeStart + step;

        // Creare pipe pentru comunicare
        HANDLE readPipe, writePipe;
        SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
        if (!CreatePipe(&readPipe, &writePipe, &sa, 0)) {
            std::cerr << "Eroare la crearea pipe-ului.\n";
            exit(1);
        }

        // Configurare proces copil
        STARTUPINFOW si = { sizeof(STARTUPINFOW) };
        PROCESS_INFORMATION pi = { 0 };
        si.hStdOutput = writePipe;
        si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        si.dwFlags |= STARTF_USESTDHANDLES;

        // Creare linie de comandă pentru procesul copil
        std::wstringstream argsStream;
        argsStream << L"ChildProcess.exe " << rangeStart << L" " << rangeEnd;
        std::wstring commandLine = argsStream.str();

        if (!CreateProcessW(
            NULL, &commandLine[0], NULL, NULL, TRUE,
            0, NULL, NULL, &si, &pi)) {
            std::cerr << "Eroare la crearea procesului copil. Cod eroare: " << GetLastError() << "\n";
            exit(1);
        }

        // Închidem capătul de scriere al pipe-ului în procesul părinte
        CloseHandle(writePipe);
        pipes.push_back({ readPipe, pi.hProcess });
    }
}

void readFromChildren(std::vector<std::pair<HANDLE, HANDLE>>& pipes) {
    for (auto& pipe : pipes) {
        HANDLE readPipe = pipe.first;
        HANDLE processHandle = pipe.second;

        char buffer[4096];
        DWORD bytesRead;

        // Citim datele de la procesul copil
        while (ReadFile(readPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
            buffer[bytesRead] = '\0'; // Încheiem șirul
            std::cout << buffer;      // Afișăm datele
        }

        // Închidem pipe-ul și așteptăm terminarea procesului copil
        CloseHandle(readPipe);
        WaitForSingleObject(processHandle, INFINITE);
        CloseHandle(processHandle);
    }
}

int main() {
    std::vector<std::pair<HANDLE, HANDLE>> pipes;
    int start = 1, step = 1000;

    // Creăm procesele copil
    createChildProcesses(pipes, start, 10000, step);

    // Citim rezultatele
    readFromChildren(pipes);

    return 0;
}
