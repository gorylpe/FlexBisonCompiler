VAR
    a b
BEGIN
    READ a;
    WHILE a > 0 DO
        b := a / 2;
        b := 2 * b;
        IF a > b THEN WRITE 1;
        ELSE WRITE 0;
        ENDIF
        a := a / 2;
    ENDWHILE
END
