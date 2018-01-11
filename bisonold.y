%{
    #include <cstdio>
    #include <sstream>
    #include <iostream>
    #include <cmath>
    void yyerror(const char *);
    int yylex(void);

    using namespace std;

    bool err = false;
    std::stringstream ss;

    long long order = 1234577;

    long long mod(long long a, long long b)
    {
        long long r = a % b;
        return r < 0 ? r + b : r;
    }

    long long gcd(long long a, long long b)
    {
        if(b == 0) {
                return a;
        }
        else {
            return gcd(b, a % b);
        }
    }
    
    long long expo(long long a, long long b){
      long long result = 1;
    
      while (b){
        if (b%2==1){
          result *= a;
          result = mod(result, order);
        }
        b /= 2;
        a *= a;
        a = mod(a, order);
      }
    
      return result;
    }

    //rozszerzony algorytm euklidesa
    long long inv(long long a, long long b){
        long long u = 1, w = a, x = 0, z = b;
        while(w != 0){
            if(w < z){
                long long tmp = u;
                u = x;
                x = tmp;
                tmp = w;
                w = z;
                z = tmp;
            }
            long long q = w / z;
            u = u - q * x;
            w = w - q * z;
        }
        if(x < 0) x += b;
        return x;
    }
%}
%token NUM
%left '+' '-'
%left '*' '/' '%'
%right '^' MINPOW
%precedence UMINUS
%%

input
: %empty
| input line
;

line
: '\n'
| exp '\n' {
    if(err == 0){
        printf("%s\nWynik: %lld\n",ss.str().c_str(),mod($1, order));
    } else {
        err = false;
    }
    //clear stringstream
    ss.str(std::string());
}
| error '\n' {
    yyerrok;
     ss.str(std::string());
 }
;

exp
: NUM           {$$ = mod($1, order); ss << $$ << " ";}
| '-' NUM %prec UMINUS	{ $$ = mod(-$2, order); ss << $$ << " ";}
| exp '+' exp   {$$ = mod($1 + $3, order); ss << "+ ";}
| exp '-' exp   {$$ = mod($1 - $3, order); ss << "- ";}
| exp '*' exp   {$$ = mod((long long)$1 * (long long)$3, order); ss << "* ";}
| exp '/' exp	{
    $3 = mod($3, order);
	if($3 > 0){
	    long long g = gcd($3, order);
	    if(g != 1){
	        yyerror("brak el. odwrotnego w ciele");
            err = true;
	    } else {
            $$ = mod((long long)$1 * inv($3, order), order);
	    }
		ss << "/ ";
	} else {
        yyerror("dzielenie przez 0");
		err = true;
	}
}
| exp '^' exp   {
    cout << $1 << " " << $3 << endl;
    $$ = expo($1, $3);
    ss << "^ ";
}
| exp MINPOW exp   {
    long long g = gcd($1, order);
    if(g != 1){
        yyerror("brak el. odwrotnego w ciele");
        err = true;
    } else {
        long long num = inv($1, order);
        $$ = expo(num, mod($3, order));
    }
    ss << "^- ";
}
| '(' exp ')'			{ $$ = $2;}
;

%%


void yyerror (char const *s){
    fprintf(stderr, "Błąd - %s\n", s);
}

int main(void) {
    yyparse();
}
