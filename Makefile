CURRENTDIR=$(HOME)/git/File-System-Scanner/
INSTDIR=$(HOME)/git/File-System-Scanner/build

testExec: src/testExec.cpp libscanner.so
	gcc -o testExec src/testExec.cpp -L$(CURRENTDIR) -lscanner -lstdc++ -pedantic -Wall -Wextra

libscanner.so: scanner.o
	gcc -shared -o libscanner.so scanner.o -lstdc++ -pedantic -Wall -Wextra

scanner.o: src/Scanner.cpp
	gcc -c -fPIC -o scanner.o src/Scanner.cpp -lstdc++ -pedantic -Wall -Wextra

PHONY: clean install
clean: 
	-rm scanner.o
	-rm libscanner.so
	-rm testExec

install:
	mkdir -p $(INSTDIR)/.
	mv scanner.o $(INSTDIR)/.
	mv libscanner.so $(INSTDIR)/.
	mv testExec $(INSTDIR)/.