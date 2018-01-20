VAR
	a b c d e
BEGIN
	READ e;
	a := 5;
	b := 20;
	c := 100000;
	d := a + b;
	d := d + c;
	d := d + e;
	WRITE d;
END
