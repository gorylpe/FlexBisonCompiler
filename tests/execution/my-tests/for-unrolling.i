VAR
	a b c d
BEGIN
	a := 12345
	b := 12347
	FOR i FROM a TO b DO
		WRITE i;
		WRITE a;
		WRITE b;
	ENDFOR
	b := a + b;
	WRITE b;
END
