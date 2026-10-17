# Temat 11 – Metro

**Autor:** Dawid Buszta, nr albumu 158131
**Przedmiot:** Systemy Operacyjne, 2026/2027, studia stacjonarne
**Repozytorium:** https://github.com/DEV-industry/so-projekt-metro

## Opis zadania

Ze stacji metra można odjechać w kierunku A lub w kierunku B. O tym w którym kierunku odjedzie
dany pociąg informuje pasażerów Zawiadowca bezpośrednio przed przyjazdem pociągu na stację.

Na stacji stoi pociąg o pojemności P pasażerów, w którym jednocześnie można przewieźć maksymalnie
R rowerów. Do pociągu są trzy wejścia (każde może pomieścić jedną osobę): dwa dla pasażerów
z bagażem podręcznym i trzecie (na końcu składu) dla pasażerów z rowerami. Do każdego z wejść
pasażerowie ustawiają się wg zasady „najkrótszej kolejki”.

Pociąg odjeżdża co określoną ilość czasu T (np.: co 5 minut). W momencie odjazdu maszynista musi
dopilnować aby na wejściach nie było żadnego wchodzącego pasażera. Jednocześnie musi dopilnować
by liczba wszystkich pasażerów pociągu nie przekroczyła P i liczba pasażerów z rowerami nie była
większa niż R. Po odjeździe pociągu na jego miejsce pojawia się natychmiast (jeżeli jest dostępny)
nowy skład o takiej samej pojemności jak poprzedni. Łączna liczba pociągów wynosi N, każdy
o pojemności P z R miejscami na rowery.

Pasażerowie w różnym wieku przychodzą na stację w losowych momentach czasu. Przed wejściem na
przystanek kupują bilet w kasie (K) na pociąg odjeżdżający w kierunku A lub B. Istnieje pewna
liczba osób VIP (ok. 0.5% na każdy z kierunków) – posiadająca wcześniej wykupiony bilet, które nie
płacą za bilet i mogą wejść do metra i do pociągu z pominięciem kolejki oczekujących. Dzieci
w wieku poniżej 8 roku życia mogą wejść do pociągu tylko pod opieką osoby dorosłej (dziecko
zajmuje osobne miejsce w pociągu). Kasa rejestruje wszystkie wchodzące osoby (ID procesu/wątku).
Każdy pasażer przy wejściu do pociągu okazuje maszyniście ważny bilet – do pociągu nie może wejść
osoba bez wykupionego wcześniej biletu.

Pociągi przewożą pasażerów do miejsca docelowego (A lub B) i po czasie Ti (wartość losowa dla
kierunku A inna niż dla kierunku B) wracają na stację. Pociąg, który pierwotnie jechał
w kierunku A zmienia kierunek i w momencie powrotu na stacje jedzie w kierunku B.

Na polecenie zawiadowcy (sygnał 1) pociąg, który w danym momencie stoi na stacji może odjechać
z niepełną liczbą pasażerów. Po otrzymaniu od zawiadowcy polecenia (sygnał 2) pasażerowie nie
mogą wsiąść do żadnego pociągu – nie mogą wejść na dworzec. Pociągi kończą pracę po rozwiezieniu
wszystkich pasażerów.

Napisz programy symulujące działanie zawiadowcy, kasy, maszynisty i pasażerów. Raport z przebiegu
symulacji (logi) zapisać w pliku (plikach) tekstowym.

## Doprecyzowania i założenia autorskie

1. **Procesy.** Każda rola jest osobnym programem uruchamianym przez `fork()` + `exec()`:
   `zawiadowca`, `kasa` (K procesów), `maszynista` (N procesów, jeden na pociąg),
   `generator` (tworzy pasażerów) i `pasazer` (jeden proces na pasażera). Proces `metro`
   tworzy struktury IPC, uruchamia pozostałe procesy, czeka na ich zakończenie i sprząta.
2. **K** oznacza liczbę stanowisk kasowych (procesów kasy). Pasażer ustawia się do wspólnej
   kolejki i obsługuje go pierwsze wolne stanowisko.
3. **Sygnał 1** to `SIGUSR1`, **sygnał 2** to `SIGUSR2`, oba wysyłane do procesu zawiadowcy
   (np. `kill -USR1 <pid>`). Zawiadowca przekazuje polecenie maszyniście pociągu stojącego na
   stacji albo zamyka dworzec.
4. **Kierunek.** Pociągi wyjeżdżają naprzemiennie w kierunku A i B. Każdy pociąg po powrocie
   zmienia kierunek. Zawiadowca ogłasza kierunek tuż przed wjazdem pociągu na peron, a na
   peron wchodzą tylko pasażerowie z biletem na ten kierunek; pozostali czekają na dworcu.
5. **Wejścia.** Pasażer z rowerem korzysta wyłącznie z wejścia 3. Pasażer z bagażem
   podręcznym wybiera krótszą z kolejek do wejść 1 i 2. W wejściu może przebywać jedna osoba.
6. **VIP** (ok. 0,5% pasażerów na każdy kierunek) ma bilet wcześniej, więc pomija kasę,
   a maszynista obsługuje go przed oczekującymi pasażerami.
7. **Dziecko poniżej 8 lat** jest wątkiem w procesie opiekuna. Dorosły kupuje bilet także dla
   dziecka. Dziecko i opiekun wchodzą tym samym wejściem i wsiadają razem albo wcale (gdy
   w pociągu zostało tylko jedno wolne miejsce, oboje czekają na kolejny pociąg).
8. **Bilet** zawiera kierunek i identyfikator nadany przez kasę. Maszynista sprawdza bilet przy
   wejściu; pasażer bez ważnego biletu na dany kierunek nie wsiada.
9. **Limit procesów.** Generator pilnuje, by jednocześnie istniało tylko tylu pasażerów, ilu
   mieści się w limicie procesów użytkownika (na torusie 100). Gdy limit jest osiągnięty,
   generator czeka w `wait()` na zakończenie któregoś pasażera.
10. **Koniec symulacji.** Pociągi kończą pracę, gdy wszyscy pasażerowie, którzy weszli na
    dworzec, zostali przewiezieni, a generator nie tworzy już nowych. `SIGINT`/`SIGTERM`
    (np. `Ctrl+C`) powoduje łagodne zakończenie wszystkich procesów i usunięcie IPC.
11. **Logi.** Każdy proces dopisuje zdarzenia do pliku `logs/symulacja.log` (czas, rola, PID,
    zdarzenie). Kasa dodatkowo zapisuje rejestr wchodzących osób w `logs/kasa.log`.

## Parametry

| Parametr | Znaczenie | Zakres |
|---|---|---|
| P | pojemność pociągu | 1–1000 |
| R | miejsca na rowery | 0–P |
| N | liczba pociągów | 1–10 |
| T | czas postoju na stacji (s) | 1–3600 |
| Ti A, Ti B | zakres czasu kursu dla kierunku A i B (s) | 1–3600, min ≤ max |
| K | liczba stanowisk kasowych | 1–10 |
| M | łączna liczba pasażerów | 1–100000 |

Niepoprawne wartości powodują wyświetlenie komunikatu i zakończenie programu przed utworzeniem
jakichkolwiek procesów i struktur IPC.

## Testy

Każdy test sprawdzany jest na podstawie logów symulacji (skrypty w katalogu `tests/`) oraz
stanu systemu po zakończeniu (`ps`, `ipcs`).

### Test 1 – Limity pojemności P i R

- **Cel:** sprawdzenie, czy synchronizacja wejść nie dopuszcza do przekroczenia pojemności.
- **Parametry:** P = 10, R = 2, N = 2, T = 2, M = 2000, duży udział pasażerów z rowerami.
- **Przebieg:** uruchomienie symulacji do końca, analiza logu odjazdów.
- **Oczekiwany wynik:** w żadnym odjeżdżającym pociągu nie ma więcej niż 10 pasażerów
  ani więcej niż 2 rowerów; w chwili odjazdu żadne wejście nie jest zajęte; wszyscy
  pasażerowie zostali przewiezieni.

### Test 2 – Sygnał 1: odjazd niepełnego pociągu

- **Cel:** sprawdzenie obsługi sygnału i braku blokady po wymuszonym odjeździe.
- **Parametry:** P = 50, T = 60, M = 30.
- **Przebieg:** po wjeździe pociągu i wejściu kilku pasażerów wysłanie `SIGUSR1` do zawiadowcy.
- **Oczekiwany wynik:** pociąg odjeżdża od razu z niepełną liczbą pasażerów, na peron wjeżdża
  kolejny dostępny pociąg, symulacja kończy się poprawnie.

### Test 3 – Sygnał 2: zamknięcie dworca

- **Cel:** sprawdzenie, czy zamknięcie dworca nie powoduje zakleszczenia ani osieroconych zasobów.
- **Parametry:** M = 2000, sygnał wysłany w trakcie symulacji.
- **Przebieg:** wysłanie `SIGUSR2` do zawiadowcy, odczekanie do końca symulacji.
- **Oczekiwany wynik:** po sygnale nikt nie wchodzi na dworzec ani do pociągu, pasażerowie
  obecni wcześniej w pociągach są dowiezieni, wszystkie procesy kończą się, `ipcs` nie pokazuje
  struktur użytkownika, `ps` nie pokazuje procesów zombie.

### Test 4 – Dziecko pod opieką dorosłego

- **Cel:** sprawdzenie, czy dziecko i opiekun wsiadają razem (synchronizacja wątku z procesem).
- **Parametry:** P = 3, duży udział dorosłych z dziećmi.
- **Przebieg:** analiza logu wsiadania.
- **Oczekiwany wynik:** dziecko nigdy nie wsiada bez opiekuna; gdy w pociągu jest tylko jedno
  wolne miejsce, para czeka na kolejny pociąg; liczba miejsc zajętych przez parę wynosi 2.

### Test 5 – Pierwszeństwo VIP

- **Cel:** sprawdzenie, czy VIP pomija kolejkę oczekujących.
- **Parametry:** P = 5, M = 1000, zwiększony udział VIP-ów (opcja testowa).
- **Przebieg:** analiza logu wsiadania przy długich kolejkach.
- **Oczekiwany wynik:** VIP nie kupuje biletu w kasie i wsiada przed pasażerami, którzy
  czekali wcześniej na ten sam pociąg.

### Test 6 – Obciążenie i limit procesów

- **Cel:** sprawdzenie, czy symulacja nie przekracza limitu procesów użytkownika.
- **Parametry:** M = 20000; osobno próba uruchomienia z N i K przekraczającymi limit.
- **Przebieg:** uruchomienie symulacji, okresowe sprawdzanie liczby procesów użytkownika.
- **Oczekiwany wynik:** `fork()` nigdy nie kończy się błędem `EAGAIN`, liczba procesów nie
  przekracza limitu, symulacja kończy się poprawnie, a konfiguracja przekraczająca limit jest
  odrzucana z komunikatem przed startem.
