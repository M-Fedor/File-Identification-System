CURRENTDIR=$(HOME)/git/File-System-Scanner/
INSTDIR=$(HOME)/git/File-System-Scanner/build

testExec: src/testExec.cpp libscanner.so libsha2.so
	gcc -o testExec src/testExec.cpp -L$(CURRENTDIR) -lscanner -lstdc++ -pedantic -Wall -Wextra

libscanner.so: scanner.o
	gcc -shared -o libscanner.so scanner.o -pedantic -Wall -Wextra

libhashAlgorithm.so: hashAlgorithm.o
	gcc -shared -o libhashAlgorithm.so hashAlgorithm.o -pedantic -Wall -Wextra

libsha2.so: sha2.o 
	gcc -shared -o libsha2.so sha2.o -pedantic -Wall -Wextra

scanner.o: src/Scanner.cpp
	gcc -c -fPIC -o scanner.o src/Scanner.cpp -lstdc++ -pedantic -Wall -Wextra

hashAlgorithm.o: src/HashAlgorithm.cpp
	gcc -c -fPIC -o hashAlgorithm.o src/HashAlgorithm.cpp -lstdc++ -pedantic -Wall -Wextra

sha2.o: src/SHA2.cpp libhashAlgorithm.so
	gcc -c -fPIC -o sha2.o src/SHA2.cpp -L$(CURRENTDIR) -lhashAlgorithm -lstdc++ -pedantic -Wall -Wextra

PHONY: clean install
clean: 
	-rm scanner.o
	-rm sha2.o
	-rm hashAlgorithm.o
	-rm libscanner.so
	-rm libsha2.so
	-rm libhashAlgorithm.so
	-rm testExec

install:
	mkdir -p $(INSTDIR)/.
	-mv scanner.o $(INSTDIR)/.
	-mv sha2.o $(INSTDIR)/.
	-mv hashAlgorithm.o $(INSTDIR)/.
	-mv libscanner.so $(INSTDIR)/.
	-mv libsha2.so $(INSTDIR)/.
	-mv libhashAlgorithm.so $(INSTDIR)/.
	-mv testExec $(INSTDIR)/.