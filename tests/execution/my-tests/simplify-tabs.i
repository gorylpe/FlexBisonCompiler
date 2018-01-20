VAR
	a b c t[100] 
BEGIN
	a := 5;
	FOR i FROM 0 TO 99 DO
		t[i] := a + i;
	ENDFOR
	FOR i FROM 0 TO 99 DO
		WRITE t[i];
	ENDFOR
END
