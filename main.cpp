#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include <sstream>
#include <cstdlib>
using namespace std;

bool estePrim(int numar) {
    if (numar < 2) return false;
    for (int i = 2; i * i <= numar; ++i) {
        if (numar % i == 0) return false;
    }
    return true;
}

void gasestePrime(int start, int end, HANDLE pipeScriere) {
    DWORD bytesScrise;
    for (int i = start; i <= end; ++i) {
        if (estePrim(i)) {
            WriteFile(pipeScriere, &i, sizeof(i), &bytesScrise, NULL);
        }
    }
    int semnalFinal = -1;
    WriteFile(pipeScriere, &semnalFinal, sizeof(semnalFinal), &bytesScrise, NULL);
}

int main(int argc, char* argv[]) {
    if (argc == 3) {
        int start = atoi(argv[1]);
        int end = atoi(argv[2]);
        HANDLE pipeScriere = GetStdHandle(STD_OUTPUT_HANDLE);
        gasestePrime(start, end, pipeScriere);
        return 0;
    }


    const int TOTAL_NUMERE = 10000;
    const int NUMAR_PROCESE = 10;
    const int DIMENSIUNE_INTERVAL = TOTAL_NUMERE / NUMAR_PROCESE;

    HANDLE pipes[NUMAR_PROCESE][2];       
    PROCESS_INFORMATION procese[NUMAR_PROCESE];
    STARTUPINFOA startupInfo[NUMAR_PROCESE];


    for (int i = 0; i < NUMAR_PROCESE; ++i) {
        SECURITY_ATTRIBUTES secAttr;
        secAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        secAttr.lpSecurityDescriptor = NULL;
        secAttr.bInheritHandle = TRUE;

        if (!CreatePipe(&pipes[i][0], &pipes[i][1], &secAttr, 0)) {
            cerr << "Eroare la crearea pipe-ului!" << endl;
            return 1;
        }

        ZeroMemory(&startupInfo[i], sizeof(startupInfo[i]));
        startupInfo[i].cb = sizeof(startupInfo[i]);
        startupInfo[i].hStdOutput = pipes[i][1]; 
        startupInfo[i].dwFlags |= STARTF_USESTDHANDLES;

        int start = i * DIMENSIUNE_INTERVAL + 1;
        int end = start + DIMENSIUNE_INTERVAL - 1;

        stringstream comanda;
        comanda << "\"" << argv[0] << "\" " << start << " " << end;

        ZeroMemory(&procese[i], sizeof(procese[i]));
        if (!CreateProcessA(NULL, const_cast<LPSTR>(comanda.str().c_str()),
            NULL, NULL, TRUE, 0, NULL, NULL, &startupInfo[i], &procese[i])) {
            cerr << "Eroare la crearea procesului!" << endl;
            return 1;
        }

        CloseHandle(pipes[i][1]);
    }


    cout << "Numere prime gasite de procesele secundare:\n";
    for (int i = 0; i < NUMAR_PROCESE; ++i) {
        DWORD bytesCitite;
        int numarPrim;

        while (true) {
            if (ReadFile(pipes[i][0], &numarPrim, sizeof(numarPrim), &bytesCitite, NULL) && bytesCitite > 0) {
                if (numarPrim == -1) break;
                cout << numarPrim << " ";
            }
            else {
                break;
            }
        }

        CloseHandle(pipes[i][0]);

        WaitForSingleObject(procese[i].hProcess, INFINITE);
        CloseHandle(procese[i].hProcess);
        CloseHandle(procese[i].hThread);
    }

    return 0;
}
