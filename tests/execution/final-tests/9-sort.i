( sort.imp 
	DODAC PROPAGACJE STALYCH
	NASTEPNIE q I n SĄ NIEUZYWANE - usuwanie ich
	n JEST W PETLI - WIECEJ NIZ RAZ ZASTOSOWANE, SPRAWDZIC CZY LOAD SZYBSZE NIZ TWORZENIE
	q PRZY MNOZENIU - UZYWANIE STALEJ ZAWSZE SZYBSZE
	ROZWINIECIE PETLI FOR BO SAME STALE xd
	tab[liczba] SZYBSZE niz tab[zmienna]
)
VAR
	tab[22] x q w j k n m
BEGIN
	n := 23;
	m := n - 2;
	q := 5;
	w := 1;
	(generowanie nieposortowanej tablicy)
	FOR i FROM 0 TO m DO
		w := w * q;
		w := w % n;
		tab[i] := w;
	ENDFOR
	(wypisywanie nieposortowanej tablicy)
	FOR i FROM 0 TO m DO
		WRITE tab[i];
	ENDFOR
	WRITE 1234567890;
	(sortowanie)
	FOR i FROM 1 TO m DO
		x := tab[i];
		j := i;
		WHILE j > 0 DO
			k := j - 1;
			IF tab[k] > x THEN
				tab[j] := tab[k];
				j := j - 1;
			ELSE
				k := j;
				j := 0;
			ENDIF
		ENDWHILE
		tab[k] := x;
	ENDFOR
	(wypisywanie posortowanej tablicy)
	FOR i FROM 0 TO m DO
		WRITE tab[i];
	ENDFOR
END
