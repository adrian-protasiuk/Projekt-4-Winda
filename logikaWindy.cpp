#include "logikaWindy.hpp"

int Pasazer::globalId = 0;
std::vector<std::unique_ptr<Pasazer>> pasazerowie;

Pasazer::Pasazer(int pStart, int pKoniec)
    : pietroStart(pStart), pietroKoniec(pKoniec), stan(spawn) {
    kierunek = (pStart < pKoniec) ? gora : dol;
    id = globalId++;
}

Winda::Winda()
    : pierwszaKolej(0), pietro(0), kierunek(stop), liczbaPasazerow(0), waga(0) {
    for (int i = 0; i < 5; i++) cele[i] = false;
}

void Winda::aktualizacjaWagi() {
    liczbaPasazerow = vectorPasazerow.size();
    waga = liczbaPasazerow * 70;
}

void Winda::wezwij(Pasazer& pasazer) {
    if (pasazer.stan == spawn) {
        if (pasazer.kierunek == gora) {
            kolejkaGora.insert(pasazer.pietroStart);
            pasazer.stan = czeka;
        } else if (pasazer.kierunek == dol) {
            kolejkaDol.insert(pasazer.pietroStart);
            pasazer.stan = czeka;
        }
    }

    if (pasazer.stan == czeka) {
        if (pasazer.kierunek == gora)
            kolejkaGora.insert(pasazer.pietroStart);
        else
            kolejkaDol.insert(pasazer.pietroStart);
    }
}

void Winda::odbierz(Pasazer& pasazer, int index) {
    if (vectorPasazerow.size() >= 8) return;
    if (pietro == pasazer.pietroStart && pasazer.stan == czeka &&
        (kierunek == pasazer.kierunek || kierunek == stop)) {

        pasazer.stan = jedzie;

        std::cout << "[INFO] Pasazer wsiada: start=" << pasazer.pietroStart
                  << " -> cel=" << pasazer.pietroKoniec
                  << ", cel aktywowany = " << cele[pasazer.pietroKoniec]
                  << ", liczba pasazerow: " << vectorPasazerow.size() << std::endl;

        vectorPasazerow.push_back(index);

        if (pasazer.kierunek == gora)
            kolejkaGora.erase(pasazer.pietroStart);
        if (pasazer.kierunek == dol)
            kolejkaDol.erase(pasazer.pietroStart);

        if (pasazer.pietroKoniec >= 0 && pasazer.pietroKoniec < 5 && !cele[pasazer.pietroKoniec]) {
            cele[pasazer.pietroKoniec] = true;
            std::cout << "[CELE] Dodano cel: " << pasazer.pietroKoniec << std::endl;
        }

        aktualizacjaWagi();
    }
}

void Winda::odstaw() {
    bool ktosWysiadl = false;

    for (auto it = vectorPasazerow.begin(); it != vectorPasazerow.end(); ) {
        int idx = *it;
        if (pasazerowie[idx]->pietroKoniec == pietro && pasazerowie[idx]->stan == jedzie) {
            pasazerowie[idx]->stan = dojechal;
            it = vectorPasazerow.erase(it);
            ktosWysiadl = true;
            std::cout << "[INFO] Pasazer wysiada na pietrze " << pietro << std::endl;
        } else {
            ++it;
        }
    }

    if (ktosWysiadl) {
        cele[pietro] = false;
        std::cout << "[CELE] Dezaktywowano cel: " << pietro << std::endl;
    }

    aktualizacjaWagi();
}

void Winda::ruchPierwszaKolej() {
    if (pierwszaKolej == pietro) return;
    pietro < pierwszaKolej ? pietro++ : pietro--;
}

void Winda::pierwszyRuchJesliPotrzeba() {
    if (kierunek == stop) {
        if (!kolejkaGora.empty()) {
            pierwszaKolej = *kolejkaGora.begin();
            kierunek = gora;
            ruchPierwszaKolej();
        } else if (!kolejkaDol.empty()) {
            pierwszaKolej = *kolejkaDol.begin();
            kierunek = dol;
            ruchPierwszaKolej();
        } else if (vectorPasazerow.empty() && pietro != 0) {
            kierunek = dol;
            pietro--;
        }
    }
}

void Winda::ruch() {
    if (kierunek == gora) {
        for (int i = pietro + 1; i < 5; i++) {
            if (cele[i] || kolejkaGora.count(i)) {
                std::cout << "[RUCH] Winda w gore z " << pietro << " na " << i << std::endl;
                pietro++;
                return;
            }
        }
    } else if (kierunek == dol) {
        for (int i = pietro - 1; i >= 0; i--) {
            if (cele[i] || kolejkaDol.count(i)) {
                std::cout << "[RUCH] Winda w dol z " << pietro << " na " << i << std::endl;
                pietro--;
                return;
            }
        }
    }

    for (int idx : vectorPasazerow) {
        const Pasazer& p = *pasazerowie[idx];
        if (p.pietroStart == pietro && p.stan == czeka)
            return;
    }

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
