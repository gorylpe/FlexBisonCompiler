VAR
	a b c d e
BEGIN
	READ e;
	a := e;
	e := a;
	a := e;
	b := 20;
	c := 100000;
	d := a + b;
	d := d + c;
	d := d + e;
	a := d;
	d := a + 10;
	a := d;
	WRITE d;
END
