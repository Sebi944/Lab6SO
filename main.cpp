#include <iostream>
#include <vector>
#include <string>
#include <unistd.h> 
#include <sys/wait.h> 
#include <fcntl.h>
#include <cstdlib>
using namespace std;

bool estePrim(int numar) {
    if (numar < 2) return false;
    for (int i = 2; i * i <= numar; ++i) {
        if (numar % i == 0) return false;
    }
    return true;
}

void gasestePrime(int start, int end, int pipeScriere) {
    for (int i = start; i <= end; ++i) {
        if (estePrim(i)) {
            write(pipeScriere, &i, sizeof(i));
        }
    }
    int semnalFinal = -1; 
    write(pipeScriere, &semnalFinal, sizeof(semnalFinal));
}

int main(int argc, char* argv[]) {
    if (argc == 3) {
        int start = atoi(argv[1]);
        int end = atoi(argv[2]);
        int pipeScriere = STDOUT_FILENO; 
        gasestePrime(start, end, pipeScriere);
        return 0;
    }


    const int TOTAL_NUMERE = 10000;
    const int NUMAR_PROCESE = 10;
    const int DIMENSIUNE_INTERVAL = TOTAL_NUMERE / NUMAR_PROCESE;

    int pipes[NUMAR_PROCESE][2];
    pid_t procese[NUMAR_PROCESE];

    for (int i = 0; i < NUMAR_PROCESE; ++i) {
        if (pipe(pipes[i]) == -1) {
            cerr << "Eroare la crearea pipe-ului!" << endl;
            return 1;
        }

        pid_t pid = fork();
        if (pid == -1) {
            cerr << "Eroare la fork!" << endl;
            return 1;
        }

        if (pid == 0) {

            close(pipes[i][0]); 
	    int start = i * DIMENSIUNE_INTERVAL + 1;
            int end = start + DIMENSIUNE_INTERVAL - 1;
            gasestePrime(start, end, pipes[i][1]);
            close(pipes[i][1]); 
        return 0;
        } else {
            procese[i] = pid;
            close(pipes[i][1]); 
	}
    }

    
    cout << "Numere prime găsite de procesele secundare:\n";
    for (int i = 0; i < NUMAR_PROCESE; ++i) {
        int numarPrim;

        while (read(pipes[i][0], &numarPrim, sizeof(numarPrim)) > 0) {
            if (numarPrim == -1) break; 
            cout << numarPrim << " "; 
        }

        close(pipes[i][0]);

        waitpid(procese[i], NULL, 0);
    }

    cout << "\nAfișare finalizată.\n";
    return 0;
}
