VAR
	a b c
BEGIN
	a := 5;
	WHILE a > 3 DO
		a := a - 1;
	ENDWHILE
	WRITE a;
END
