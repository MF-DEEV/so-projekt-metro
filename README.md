# Metro - symulacja stacji metra

Projekt z przedmiotu **Systemy Operacyjne** (2026/2027, studia stacjonarne), temat 11 „Metro”.
Autor: Dawid Buszta, nr albumu 158131.

Program symuluje stację metra, z której pociągi odjeżdżają w kierunku A lub B. W symulacji
biorą udział zawiadowca, kasa, maszyniści pociągów i pasażerowie. Każda z tych ról działa
jako osobny proces uruchamiany przez `fork()` + `exec()`, a procesy komunikują się wyłącznie
przez mechanizmy IPC Systemu V i sygnały.

Pełny opis zadania, przyjęte doprecyzowania i opis testów:
[`docs/BUSZTA_DAWID_158131_opis_METRO.md`](docs/BUSZTA_DAWID_158131_opis_METRO.md).

## Wymagania

- Linux (docelowo serwer `torus.uck.pk.edu.pl`, Debian 13, gcc 14)
- `gcc` z obsługą C11, `make`, biblioteka `pthread`

## Budowanie i uruchomienie

```sh
make            # kompilacja wszystkich programów do katalogu bin/
make run        # uruchomienie symulacji z domyślnymi parametrami
make test       # uruchomienie testów z katalogu tests/
make ipc-clean  # usunięcie struktur IPC pozostałych po przerwanej symulacji
make clean      # usunięcie plików wynikowych
```

Uruchomienie z własnymi parametrami:

```sh
./bin/metro -P 20 -R 4 -N 3 -T 5 -A 4-8 -B 6-12 -K 2 -M 300 -s 42
```

| Opcja | Znaczenie | Domyślnie |
|---|---|---|
| `-P` | pojemność pociągu (liczba pasażerów) | 20 |
| `-R` | maksymalna liczba rowerów w pociągu (`R ≤ P`) | 4 |
| `-N` | łączna liczba pociągów | 3 |
| `-T` | czas postoju pociągu na stacji, w sekundach | 5 |
| `-A` | zakres czasu kursu w kierunku A (`min-max`, sekundy) | 4-8 |
| `-B` | zakres czasu kursu w kierunku B (`min-max`, sekundy) | 6-12 |
| `-K` | liczba stanowisk kasowych | 2 |
| `-M` | łączna liczba pasażerów przychodzących na stację | 300 |
| `-s` | ziarno generatora liczb losowych (powtarzalne przebiegi) | czas systemowy |

Gdy `-R` nie zostanie podane, a `P < 4`, liczba miejsc na rowery jest równa `P`.
`./bin/metro -h` wypisuje pomoc. Wszystkie wartości są sprawdzane przed startem. Program sprawdza też, czy łączna liczba
procesów zmieści się w limicie użytkownika (`ulimit -u`; na torusie wynosi on 100).

### Sterowanie w trakcie symulacji

PID zawiadowcy jest wypisywany przy starcie.

```sh
kill -USR1 <pid_zawiadowcy>   # sygnał 1: pociąg stojący na stacji odjeżdża od razu
kill -USR2 <pid_zawiadowcy>   # sygnał 2: stacja zamknięta, nikt więcej nie wchodzi
```

`Ctrl+C` kończy symulację łagodnie: procesy się zamykają, a struktury IPC są usuwane.

## Architektura

```
metro (proces startowy)
 ├── zawiadowca      ogłasza kierunek pociągu, obsługuje sygnały 1 i 2
 ├── kasa ×K         sprzedaje bilety, rejestruje PID każdej wchodzącej osoby
 ├── maszynista ×N   jeden na pociąg: wpuszcza pasażerów, pilnuje P i R, odjeżdża co T
 └── generator       tworzy pasażerów w losowych momentach
      └── pasazer ×M   proces pasażera; dziecko poniżej 8 lat to wątek w procesie opiekuna
```

| Mechanizm | Zastosowanie |
|---|---|
| Pamięć dzielona | stan peronu: pociąg na stacji, kierunek, liczba pasażerów i rowerów, długości kolejek do wejść, flaga zamknięcia stacji |
| Semafory | wyłączny dostęp do stanu peronu, trzy wejścia do pociągu (po jednej osobie), jeden pociąg na peronie |
| Kolejki komunikatów | zakup biletu w kasie; kontrola biletu przez maszynistę (VIP-y obsługiwane w pierwszej kolejności dzięki priorytetowi typu komunikatu) |
| Sygnały | `SIGUSR1`, `SIGUSR2` (polecenia zawiadowcy), `SIGINT`/`SIGTERM` (zakończenie), `SIGCHLD` (sprzątanie procesów potomnych) |
| Wątki | dziecko pod opieką dorosłego |
| Pliki | logi przebiegu symulacji w katalogu `logs/` |

Program nie czeka aktywnie: procesy blokują się na semaforach, kolejkach komunikatów
i w `wait()`. Wszystkie wywołania systemowe są sprawdzane (`perror()` + `errno`),
a struktury IPC są tworzone z prawami `0600` i usuwane po zakończeniu.

## Struktura repozytorium

```
src/        kod źródłowy (każda rola w osobnym pliku + moduły wspólne)
docs/       opis projektu i testów
tests/      skrypty testów
Makefile
```

## Stan prac

- [x] Opis zadania i testów
- [x] Szkielet projektu, obsługa błędów, logowanie, walidacja parametrów
- [ ] Struktury IPC i uruchamianie procesów
- [ ] Kasa i bilety
- [ ] Wsiadanie przez trzy wejścia, limity P i R
- [ ] Cykl pociągów i kierunki A/B
- [ ] VIP-y, dzieci pod opieką
- [ ] Sygnały 1 i 2, łagodne zamknięcie
- [ ] Testy
- [ ] Raport
