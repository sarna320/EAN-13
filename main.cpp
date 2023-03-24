#include <QCoreApplication>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <cmath>
#include <vector>

using namespace std;

vector<uint64_t> WChosen; // Wektor z wybranymi ilosciami walkow jako najlepszy

vector<uint64_t> WLoop; // Wektor do petli resetowny w kazdej iteracji

uint64_t kwota_pozostala_koszt; // Najlepszy koszt

uint64_t koszt_lokalny = 0;

uint64_t licznik_lokalny;

const uint64_t ilosc_iteracji = 100000000;

const uint64_t Q = 542; // Ilosc walkow lub kwota do wydania wdg prezentacji

uint64_t W[] = {101, 102, 9, 109, 108, 107, 8, 105, 77, 100, 40}; // Ilosc potencjalnie wyprodukowanych walkow lub nominaly wdg prezezentacji

uint64_t K[] = {10100, 9000, 800, 9500, 11000, 10900, 1007, 8800, 7700, 10000, 4000}; // Koszty wyprodukowane odpowiadajace ilosc walkow w tablicy W

const uint64_t C = 50000; // Kwota nie do przekroczenia przy produkcji

int64_t kwota_pozostala;  // pozostala kwota do wydania. Tu musi być ze znakiem !!!!!
uint64_t ilosc_nominalow; // ilosc nominalow monet jakimi wydajemy - wielkosc tablicy monety
uint64_t licznik_wydanych_monet;

// Czesc do losowania

static uint64_t init_state;        // The state can be seeded with any value.  Wartosc potrzebna do inicjalizacji SplitMix64
static uint64_t internal_state[4]; // tablica stanow funkcji losujacej

static inline uint64_t rotation(const uint64_t x, int k)
{
    return ((x << k) | x >> (64 - k));
}

// Funkcja do inicjalizacji stanu algorytmu XoShiRo256pp
uint64_t SplitMix64(void)
{
    uint64_t z = (init_state += 0x9e3779b97f4a7c15);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
    z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
    return z ^ (z >> 31);
}

// Funkcja losujaca
uint64_t XoShiRo256pp(void)
{
    const uint64_t result = rotation(internal_state[0] + internal_state[3], 23) + internal_state[0];
    const uint64_t t = internal_state[1] << 17;
    internal_state[2] ^= internal_state[0];
    internal_state[3] ^= internal_state[1];
    internal_state[1] ^= internal_state[2];
    internal_state[0] ^= internal_state[3];
    internal_state[2] ^= t;
    internal_state[3] = rotation(internal_state[3], 45);
    return result;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Czesc od losowania
    init_state = (uint64_t)time(nullptr);
    for (uint64_t i = 0; i < 4; i++)
    {
        internal_state[i] = SplitMix64();
    }
    uint64_t los;
    srand((unsigned)time(0)); // inicjalizacja funkcji losujacej rand(). Aby uniknac powtorzen podczas kazdorazowego uruchamiania.

    // Inicjalizacja wartosci
    kwota_pozostala_koszt = ~0; // Kwota pozostała to najlepsza uzyskana kwota
    ilosc_nominalow = sizeof(W) / sizeof(W[0]);
    licznik_wydanych_monet = ~0;

    // Glowna petla algorytmu
    for (uint64_t i = 0; i < ilosc_iteracji; i++)
    {
        licznik_lokalny = 0;
        kwota_pozostala = Q;
        koszt_lokalny = 0;
        WLoop.clear();
        while ((kwota_pozostala > 0))
        {
            los = XoShiRo256pp() % ilosc_nominalow;
            kwota_pozostala -= W[los];
            WLoop.push_back(W[los]); // Dodajemy odpowiednia maszyne do wektora
            koszt_lokalny += K[los]; // Koszt lokalny maszyny
            licznik_lokalny++;
        }

        if (kwota_pozostala == 0 && koszt_lokalny < kwota_pozostala_koszt) // jesli koszt lokalny jest lepszy i zostaly wyprodukowane wszystkie walki
        {                                                                  // usuniety warunek sprawdzajacy czy ilosc maszyn jest mniejsza, bardziej zalezy chyba na lepszym koszcie
                                                                           // dodatkowo algorytm nie mogl znalezc istniejacego rozwiazania gdy ilosc maszyn za szybko zmalala
            kwota_pozostala_koszt = koszt_lokalny;                         // przypisanie nowego kosztu
            licznik_wydanych_monet = licznik_lokalny;                      // przypisanie ilosci wykorzystanych maszyn
            cout << endl
                 << "Znalazlem cos lepszego!!! Ilosc uzytych maszyn: " << licznik_wydanych_monet << endl
                 << "Koszt calkowity wynosi " << kwota_pozostala_koszt << endl;
            WChosen = WLoop; // przypisanie znalezionych walkow
        }
    }

    if (licznik_wydanych_monet == ~0)
    {
        cout << endl
             << "!!!! Niestety nie udalo sie wydac poprawnie reszty !!!!!" << endl;
    }

    else
    { // wypisanie znalezionego rozwiazania
        cout << endl
             << "Ilosc uzytych maszyn: " << licznik_wydanych_monet << endl
             << "Znalezione ilosci walkow wyprodukowanych przez maszyny:";

        for (size_t i = 0; i < WChosen.size(); i++)
        {
            cout << WChosen[i] << " ";
        }

        if (kwota_pozostala_koszt <= C)
        {
            cout << endl
                 << "Zlecenie jest mozliwe do zrealizowania i jego koszt wynosi " << kwota_pozostala_koszt << " i uzyte maszyny to:";
            for (size_t i = 0; i < WChosen.size(); i++)
            {
                for (size_t j = 0; j < ilosc_nominalow; j++)
                {
                    if (WChosen[i] == W[j])
                    {
                        cout << "m" << j << " ";
                    }
                }
            }
        }

        else
        {
            cout << endl
                 << "Niestety koszt uzycia maszyn jest zbyt duzy i wynosi:" << kwota_pozostala_koszt << endl;
        }
    }

    return a.exec();
}
