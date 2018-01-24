Piotr Przestrzelski
24 stycznia 2017

Projekt kompilatora na kurs Języki formalne i techniki translacji

#### Sposób użycia

1. Kompilacja kompilatora jest automatyczna poprzez użycie komendy 
`make`

2. Aby skompilować program napisany w **Prostym języku imperatywnym** należy użyć komendy
`./compiler [flagi] < input > output`
* *Standardowe wejście* - plik z programem napisanym w tym języku
* *Standardowe wyjście* - plik dla kodu wynikowego **maszyny rejestrowej**
* *Standardowe wyjście błędów* - komunikaty z kompilacji

3. Flagi jakie możemy podać do kompilacji to
* `-h, --help` wyświetla pomoc
* `-v, --verbose` drukuje na wyjście błędów pełny log kompilacji - domyślnie **drukowanie wyłączone**
* `-w n, --while n` rozwija pętle WHILE w IF z zagnieżdżonym WHILE - domyślnie **50**
* `-nf, --no-for` wyłącza rozwijanie pętli FOR ze stałą liczbą iteracji - domyślnie **rozwijanie włączone**
* `-nm, --no-mem` wyłącza optymalizowanie adresów tablic bliżej początku pamięci - domyślnie **optymalizacja włączona**
* `-nn --no-num` nie optymalizuje liczb jeśli występuje dużo tych samych i warto je przechować w zmiennej - domyślnie **optymalizacja włączona**
* `-np, --no-propagation` wyłącza propagowanie wartości (zmiennych i stałych) - domyślnie **propagacja włączona**
* `-nc, --no-opt-code` wyłącza wyświetlanie końcowego zoptymalizowanego kodu - domyślnie **wyświetlanie włączone**;

#### Opis plików
* Folder **cmds**
  * `Command.h` - zawiera klasę bazową **Command** dla wszystkich komend do budowania drzewa AST
  * `Assignment.h` - zawiera klasę **Assignment** reprezentującą przypisanie w drzewie AST
  * `CommandsBlock.h` - zawiera klasę **CommandsBlock** reprezentującą blok komend
  * `For.h` - zawiera klasę **For** reprezentującą pętlę FOR
  * `If.h` - zawiera klasę **If** reprezentującą instrukcję warunkową z blokiem kodu jeśli warunek jest spełniony
  * `IfElse.h` - zawiera klasę **IfElse** reprezentującą instrukcję warunkową z blokami kodu jeśli warunek jest spełniony i jeśli nie
  * `Read.h` - zawiera klasę **Read** reprezentującą instrukcję wczytania od użytkownika wartości
  * `While.h` - zawiera klasę **While** reprezentującą pętle WHILE
  * `Write.h` - zawiera klasę **Write** reprezentującą instrukcję wypisania wartości
* Folder **cmdParts**
  * `Variable.h` - zawiera klasę **Variable** reprezentującą miejsce w pamięci dla danej zmiennej
  * `Number.h` - zawiera klasę **Number** reprezentującą stałą w kodzie
  * `NumberValueStats.h` - zawiera pomocniczą klasę używaną przy zbieraniu liczby występowania stałych do poźniejszej optymalizacji
  * `Identifier.h` - zawiera klasę **Identifier** reprezentującą identyfikator zmiennej występującej w kodzie
  * `IdentifiersAssignmentsHelper.h` - zawiera pomocniczą klasę używaną do zebrania przypisań do wszystkich identyfikatorów z postaci Single Static Assignment
  * `IdentifierSSAHelper.h` - zawiera pomocniczą klasę używaną do stworzenia kodu postaci Single Static Assignments
  * `IdentifiersUsagesHelper.h` - zawiera pomocniczą klasę używaną do zebrania danych o ilości użyć identyfikatorów postaci Single Static Assignment
  * `Value.h` - zawiera klasę **Value** reprezentującą wartość (zmienną lub stałą) w kodzie
  * `Expression.h, Expression.cpp` - zawierają klasę **Expression** reprezentującą instrukcję wyrażeniową
  * `Condition.h` - zawiera klasę **Condition** reprezentującą warunek
* `Assembly.h` - zawiera klasy odpowiedzialne za wytworzenie instrukcji wynikowych do **maszyny rejestrowej** oraz obsługę skoków
* `MachineContext.h` - zawiera klasę typu Singleton to hold whole assembly code, jumps and methods to optimize result code
* `MemoryManager.h` - zawiera klasę do obsługi działań na pamięci, przechowywania obiektów typu **Variable** i metody do optymalizacji tablic
* `Program.h` - zawiera klasę przechowującą całe drzewo AST kodu oraz metody optymalizujące te drzewo
* `ProgramFlags.h` - zawiera klasę typu Singleton przechowującą flagi kompilacji
* `Utils.h` - zawiera przydatne makra i deklaracje funkcji zdefiniowanych w pliku compiler.y
* `compiler.y` - zawiera zasady do budowy parsera przy użyciu programu **Bison**
* `compiler.l` - zawiera zasady do budowy leksera przy użyciu programu **Flex**

