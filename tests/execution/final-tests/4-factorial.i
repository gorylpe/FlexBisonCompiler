( Silnia
? 20
> 2432902008176640000
)
VAR
  s[101] n m a j
BEGIN
    READ n;
    s[0]:=1;
    m:=n;
    FOR i FROM 1 TO m DO
		a:=i%2;
		j:=i-1;
		IF a=1 THEN
			s[i]:=s[j]*m;
		ELSE
			s[i]:=m*s[j];
		ENDIF
		m:=m-1;
    ENDFOR
    WRITE s[n];
END
(
must looks like this after optimization
VAR
  s[101] n m a j
BEGIN
    READ n;
    s[0]:=1;
    m:=n;
    FOR i FROM 1 TO m DO
		a:=i%2; //need to remove that
		j:=i-1;
			s[i]:=s[j]*m;
		m:=m-1;
    ENDFOR
    WRITE s[n];
END
)

