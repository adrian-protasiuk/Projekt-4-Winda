// logikaWindy.h
#pragma once
#include <iostream>
#include <set>
#include <vector>
#include <memory>

enum Kierunek {
    dol, stop, gora
};

enum Stan {
    spawn, czeka, jedzie, dojechal
};

class Pasazer {
public:
    int id;
    static int globalId;
    int pietroStart;
    int pietroKoniec;
    Kierunek kierunek;
    Stan stan;
    Pasazer(int pStart, int pKoniec);
};

extern std::vector<std::unique_ptr<Pasazer>> pasazerowie;

class Winda {
public:
    int pietro;
    Kierunek kierunek;
    std::vector<int> vectorPasazerow;
    int liczbaPasazerow;
    int waga;
    int pierwszaKolej;
    std::set<int> kolejkaGora;
    std::set<int, std::greater<int>> kolejkaDol;
    bool cele[5];

    Winda();
    void aktualizacjaWagi();
    void wezwij(Pasazer& pasazer);
    void odbierz(Pasazer& pasazer, int index);
    void odstaw();
    void ruchPierwszaKolej();
    void pierwszyRuchJeœliPotrzeba();
    void ruch();
};
