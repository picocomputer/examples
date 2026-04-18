# MATHVM Examples

Minimalne przykłady wywołania `MATHVM` na Picocomputer 6502 przez `OS $80`.

Pliki:

- `m3v3l_cc65.c`: przykład w `cc65 C`
- `m3v3l_ca65.s`: przykład w czystym `6502 asm` dla `ca65`
- `mathvm_negative_cc65.c`: mały zestaw negatywnych testów `v1` w `cc65 C`
- `mathvm_negative_ca65.s`: ten sam zestaw negatywnych testów w czystym `6502 asm`
- `mathvm_mathcop_cc65.c`: test zgodności `MATHVM` wzorowany na `MTHexamples/mathcop.c`
- `mathvm_batch_benchmark_cc65.c`: benchmark jednego wywołania `MATHVM` vs wielu scalar RPC `mth_*`
- `mathvm_dodecahedron_batch_cc65.c`: dwunastościan z jednym wywołaniem `MATHVM` na klatkę dla wszystkich 20 wierzchołków
- `mathvm_dodecahedron_debug_cc65.c`: debug batcha dwunastościanu, wypisuje zakres i pierwsze współrzędne z `xram_out`
- `spr2l_bbox_cc65.c`: przykład `SPR2L` w `cc65 C`
- `spr2l_bbox_ca65.s`: przykład `SPR2L` w czystym `6502 asm`
- `spr2l_corners_cc65.c`: przykład `SPR2L` zwracający 4 rogi w `cc65 C`
- `spr2l_corners_ca65.s`: przykład `SPR2L` zwracający 4 rogi w czystym `6502 asm`

Warianty `cc65 C` korzystają teraz ze wspólnego layera `mathvm_client.h/.c`, który:

- buduje jedną ramkę `MATHVM`
- wykonuje `OS $80`
- od razu zrzuca wynik z `XSTACK` do RAM, zanim jakikolwiek kod zrobi `printf()`

Jeśli kopiujesz przykład do osobnego repo `cc65`, skopiuj razem:

- wybrany `*_cc65.c`
- `mathvm_client.h`
- `mathvm_client.c`
- `mathvm.h`

Oba przykłady robią to samo:

1. Budują ramkę dla programu `M3V3L 0,9 ; RET 3`
2. Wypychają ją na `$FFEC` od końca do początku
3. Zapisują `$80` do `$FFEF`
4. Wykonują `JSR $FFF1`
5. Odczytują `A = status`, `X = liczba słów wyniku`
6. Zdejmują wynik z `$FFEC`

Ważne:

- W wersjach `cc65 C` wynik z `$FFEC` trzeba zrzucić do zwykłego RAM natychmiast po `JSR $FFF1`.
- Nie wolno robić `printf()` ani żadnego innego wywołania OS przed odczytem wyniku, bo `stdio` też używa `XSTACK` i nadpisze rezultat `MATHVM`.

W przykładzie `M3V3L`:

- `locals[0..8]` to macierz `3x3`
- `locals[9..11]` to wektor `vec3`
- oczekiwany wynik to `140.0, 320.0, 500.0`

Wynik jest czytany jako trzy `float32` w little-endian.

Najważniejsze adresy:

- `$FFEC`: `XSTACK`
- `$FFEF`: `OS op`
- `$FFF1`: wejście do `wait/return trampoline`

To są przykłady minimalne, bez integracji z istniejącym API `cc65`.

Przykład `mathvm_dodecahedron_batch_cc65.c`:

- zapisuje 20 wierzchołków `vec3` do `XRAM`
- robi jedno wywołanie `MATHVM` na klatkę
- nowy opcode batch czyta wszystkie rekordy z `xram_in` i zapisuje `int16 x/y` do `xram_out`
- przez pierwsze 180 stopni rysuje krawędzie, a przez drugie 180 stopni tylko punkty wierzchołków

Przykład `mathvm_negative_cc65.c` wypisuje tylko `status` i `words` dla:

- `bad magic`
- `bad header`
- `unsupported flag`
- `stack underflow`
- `bad local`

Przykład `mathvm_negative_ca65.s` wykonuje te same ramki, ale zapisuje wyniki do:

- `statuses[0..4]`
- `out_words[0..4]`

Przykłady `SPR2L` używają:

- affine `2x3`: identyczność + translacja `(100, 50)`
- sprite: `w=16`, `h=8`, `ax=0.5`, `ay=0.5`
- program: `SPR2L 0,6,0x02 ; RET 4`

Oczekiwany `bbox`:

- `92.0, 46.0, 108.0, 54.0`

Oczekiwane 4 rogi:

- `92.0, 46.0`
- `108.0, 46.0`
- `108.0, 54.0`
- `92.0, 54.0`
