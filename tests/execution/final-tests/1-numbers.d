( numbers.imp - liczby )
VAR
	a b c t[7] d e f g h i j tab[6]
BEGIN
	WRITE 0;
	WRITE 1;
	WRITE 2;
	WRITE 10;
	WRITE 100;
	WRITE 10000;
	WRITE 1234567890;

	a := 1234566543;
	b := 677777177;
	c := 15;
	t[2] := 555555555;
	d := 8888;
	tab[4] := 11;
	t[0] := 999;
	e := 1111111111;
	tab[0] := 7777;
	f := 2048;
	g := 123;
	t[3] := t[0];
	tab[5] := a;
	tab[5] := tab[0] / tab[4];
	t[5] := tab[0];

	READ h;
	i := 1;
	j := h + c;
	
	WRITE j; ( j = h + 15 )
	WRITE c; ( c = 15 )
	WRITE t[3]; ( 999 )
	WRITE t[2]; ( 555555555 )
	WRITE t[5]; ( 7777 )
	WRITE t[0]; ( 999 )
	WRITE tab[4]; ( 11 )
	WRITE tab[5]; ( 707 )
	WRITE tab[0]; ( 7777 )
END
