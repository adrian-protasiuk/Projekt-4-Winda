// logikaWindy.cpp
#include "logikaWindy.hpp"

int Pasazer::globalId = 0;

Pasazer::Pasazer(int pStart, int pKoniec)
    : pietroStart(pStart), pietroKoniec(pKoniec), stan(spawn) {
    kierunek = (pStart < pKoniec) ? gora : dol;
    id = globalId++;
}

// Globalny wektor pasażerów
std::vector<std::unique_ptr<Pasazer>> pasazerowie;

// implementacja metod klasy Winda — to już masz, przepisz z obecnego logikaWindy.cpp



enum Kierunek
{
    dol, stop, gora
};

enum Stan
{
    spawn, czeka, jedzie, dojechal
};

class Winda {
public:
    int pietro;
    Kierunek kierunek;
    vector<int> vectorPasazerow; // indeksy do globalnego pasazerowie[]
    int liczbaPasazerow;
    int waga;
    //mozliwa poprawka wagi
    int pierwszaKolej;
    set<int> kolejkaGora;
    set<int, greater<int>> kolejkaDol;
    bool cele[5];


    Winda() :pierwszaKolej(0), pietro(0), kierunek(stop), liczbaPasazerow(0) {
        waga = liczbaPasazerow * 70;
        for (int i = 0; i < 5; i++) cele[i] = 0;
    }

    void aktualizacjaWagi() {
        liczbaPasazerow = vectorPasazerow.size();
        waga = liczbaPasazerow * 70;
    }

    void wezwij(Pasazer& pasazer) {
        if (pasazer.stan == spawn) {
            if (pasazer.kierunek == gora) {
                kolejkaGora.insert(pasazer.pietroStart);
                pasazer.stan = czeka;
            }
            else if (pasazer.kierunek == dol) {
                kolejkaDol.insert(pasazer.pietroStart);
                pasazer.stan = czeka;
            }
        }

        // Jeśli ktoś nadal czeka, upewnij się, że jego piętro jest w kolejce
        if (pasazer.stan == czeka) {
            if (pasazer.kierunek == gora)
                kolejkaGora.insert(pasazer.pietroStart);
            else
                kolejkaDol.insert(pasazer.pietroStart);
        }
    }



    //odnbieramy tylko jak vector.size()<8
    void odbierz(Pasazer& pasazer, int index) {
        if (vectorPasazerow.size() >= 8) return;
        if (pietro == pasazer.pietroStart && pasazer.stan == czeka &&
            (kierunek == pasazer.kierunek || kierunek == stop))
        {
            //winda open animacja
            pasazer.stan = jedzie;

            std::cout << "[INFO] Pasazer wsiada: start=" << pasazer.pietroStart
                << " -> cel=" << pasazer.pietroKoniec
                << ", cel aktywowany = " << cele[pasazer.pietroKoniec]
                << ", liczba pasazerow: " << vectorPasazerow.size() << endl;

            vectorPasazerow.push_back(index);

            if (pasazer.pietroKoniec < 0 || pasazer.pietroKoniec >= 5) {
                std::cout << "[BŁĄD] Nieprawidłowy cel pasażera ID=" << pasazer.id
                    << ": " << pasazer.pietroKoniec << std::endl;
                return;
            }


            if (pasazer.kierunek == gora) {
                kolejkaGora.erase(pasazer.pietroStart);
            }
            if (pasazer.kierunek == dol) {
                kolejkaDol.erase(pasazer.pietroStart);
            }
            if (pasazer.pietroKoniec >= 0 && pasazer.pietroKoniec < 5)
                if (!cele[pasazer.pietroKoniec]) {
                    cele[pasazer.pietroKoniec] = true;
                    std::cout << "[CELE] Dodano cel: " << pasazer.pietroKoniec << std::endl;
                }


            aktualizacjaWagi();
            //animacja pasazer idzie do pola okreslonego indeksem wektora
            //zamykamy winde
            return;
        }
    }

    void odstaw() {
        bool ktosWysiadl = false;

        for (auto it = vectorPasazerow.begin(); it != vectorPasazerow.end(); ) {
            int idx = *it;
            if (pasazerowie[idx]->pietroKoniec == pietro && pasazerowie[idx]->stan == jedzie) {
                pasazerowie[idx]->stan = dojechal;
                it = vectorPasazerow.erase(it);
                ktosWysiadl = true;
            }
            else {
                ++it;
            }

        }


        // Dezaktywuj cel tylko jesli ktos rzeczywiscie wysiadl
        if (ktosWysiadl) {
            cele[pietro] = false;
            std::cout << "[CELE] Dezaktywowano cel: " << pietro << std::endl;
        }

        aktualizacjaWagi();
    }


    void ruchPierwszaKolej() {
        //animacja ruchu od do
        //pietro = pierwszaKolej;
        if (pierwszaKolej == pietro) return;
        pietro < pierwszaKolej ? pietro++ : pietro--;
    }

    //test
    void pierwszyRuchJesliPotrzeba() {
        if (kierunek == stop) {
            if (!kolejkaGora.empty()) {
                pierwszaKolej = *kolejkaGora.begin();
                kierunek = gora;
                ruchPierwszaKolej();
            }
            else if (!kolejkaDol.empty()) {
                pierwszaKolej = *kolejkaDol.begin();
                kierunek = dol;
                ruchPierwszaKolej();
            }
            else if (vectorPasazerow.empty() && pietro != 0) {
                kierunek = dol;
                pietro--; // powrót na parter
            }
        }
    }

    void ruch() {
        if (kierunek == gora) {
            for (int i = pietro + 1; i < 5; i++) {
                if (cele[i] || kolejkaGora.count(i)) {
                    std::cout << "[RUCH] Winda w gore z " << pietro << " na " << pietro + 1 << std::endl;
                    pietro++;
                    return;
                }
            }
        }


        if (kierunek == dol) {
            for (int i = pietro - 1; i >= 0; i--) {
                if (cele[i] || kolejkaDol.count(i)) {
                    pietro--;
                    return;
                }
            }
        }

        // SPRAWDZENIE: czy ktoś czeka na obecnym piętrze?
        for (int idx : vectorPasazerow) {
            const Pasazer& p = *pasazerowie[idx];
            if (p.pietroStart == pietro && p.stan == czeka) {
                return; // ktoś czeka — nie przechodzimy w STOP
            }
        }


        // 🔧 KROK 2 — awaryjne przywracanie celów
        for (int idx : vectorPasazerow) {
            const Pasazer* p = pasazerowie[idx].get();
            if (p->stan == jedzie && !cele[p->pietroKoniec]) {
                cele[p->pietroKoniec] = true;
                std::cout << "[AWARIA] Przywrocono cel pasazera ID=" << p->id
                    << " -> pietro " << p->pietroKoniec << std::endl;
            }
        }

        kierunek = stop;
    }

};

int Pasazer::globalId = 0;
