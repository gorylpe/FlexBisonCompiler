( loopiii.imp - zagniezdzone petle 
	0 0 0
	31000 40900 2222010
	
	1 0 2
	31001 40900 2222012
)
VAR 
	a b c
BEGIN
	READ a;
	READ b;
	READ c;
	FOR i FROM 111091 TO 111110 DO
		FOR j FROM 209 DOWNTO 200 DO
			FOR k FROM 11 TO 20 DO
				a := a + k;
			ENDFOR
			b := b + j;
		ENDFOR
		c := c + i;
	ENDFOR
	WRITE a;
	WRITE b;
	WRITE c;
END
