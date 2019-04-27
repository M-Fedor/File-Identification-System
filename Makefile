CURRENTDIR=$(HOME)/git/File-System-Scanner/
INSTDIR=$(HOME)/git/File-System-Scanner/build

testExec: src/testExec.cpp libinputFile.so libinputScanner.so liboutputDBConnection.so liboutputOffline.so libsha2.so libparallelExecutor.so
	gcc -o $@ $< -L$(CURRENTDIR) -linput -loutput -lhashAlgorithm -linputFile -linputScanner -loutputDBConnection -loutputOffline -lsha2 \
	-lcryptopp -lpthread -lmysqlclient -lstdc++ -pedantic -Wall -Wextra

libhashAlgorithm.so: hashAlgorithm.o
	gcc -shared -o $@ $< -pedantic -Wall -Wextra

libinput.so: input.o
	gcc -shared -o $@ $< -pedantic -Wall -Wextra

liboutput.so: output.o
	gcc -shared -o $@ $< -pedantic -Wall -Wextra

libinputFile.so: inputFile.o
	gcc -shared -o $@ $< -pedantic -Wall -Wextra

libinputScanner.so: inputScanner.o
	gcc -shared -o $@ $< -pedantic -Wall -Wextra

liboutputDBConnection.so: outputDBConnection.o
	gcc -shared -o $@ $< -pedantic -Wall -Wextra

liboutputOffline.so: outputOffline.o
	gcc -shared -o $@ $< -pedantic -Wall -Wextra

libparallelExecutor.so: parallelExecutor.o
	gcc -shared -o $@ $< -pedantic -Wall -Wextra

libsha2.so: sha2.o 
	gcc -shared -o $@ $< -pedantic -Wall -Wextra

hashAlgorithm.o: src/HashAlgorithm.cpp
	gcc -c -fPIC -o $@ $< -lstdc++ -pedantic -Wall -Wextra

input.o: src/Input.cpp
	gcc -c -fPIC -o $@ $< -lstdc++ -pedantic -Wall -Wextra

output.o: src/Output.cpp
	gcc -c -fPIC -o $@ $< -lstdc++ -pedantic -Wall -Wextra

inputFile.o: src/InputFile.cpp libinput.so
	gcc -c -fPIC -o $@ $< -L$(CURRENTDIR) -linput -lstdc++ -pedantic -Wall -Wextra

inputScanner.o: src/InputScanner.cpp libinput.so
	gcc -c -fPIC -o $@ $< -L$(CURRENTDIR) -linput -lstdc++ -pedantic -Wall -Wextra

outputDBConnection.o: src/OutputDBConnection.cpp liboutput.so 
	gcc -c -fPIC -o $@ $< -L$(CURRENTDIR) -loutput -lstdc++ -pedantic -Wall -Wextra

outputOffline.o: src/OutputOffline.cpp liboutput.so
	gcc -c -fPIC -o $@ $< -L$(CURRENTDIR) -loutput -lstdc++ -pedantic -Wall -Wextra

parallelExecutor.o: src/ParallelExecutor.cpp libinputFile.so libinputScanner.so liboutputDBConnection.so liboutputOffline.so libsha2.so
	gcc -c -fPIC -o $@ $< -L$(CURRENTDIR) -linputFile -linputScanner -loutputDBConnection -loutputOffline -lsha2 \
	-lstdc++ -pedantic -Wall -Wextra

sha2.o: src/SHA2.cpp libhashAlgorithm.so
	gcc -c -fPIC -o $@ $< -L$(CURRENTDIR) -lhashAlgorithm -lstdc++ -pedantic -Wall -Wextra

.PHONY: clean install
clean: 
	-rm hashAlgorithm.o
	-rm input.o
	-rm output.o
	-rm inputFile.o
	-rm inputScanner.o
	-rm outputDBConnection.o
	-rm outputOffline.o
	-rm parallelExecutor.o
	-rm sha2.o
	-rm libhashAlgorithm.so
	-rm libinput.so
	-rm liboutput.so
	-rm libinputFile.so
	-rm libinputScanner.so
	-rm liboutputDBConnection.so
	-rm liboutputOffline.so
	-rm libparallelExecutor.so
	-rm libsha2.so
	-rm testExec

install:
	mkdir -p $(INSTDIR)/.
	-mv hashAlgorithm.o $(INSTDIR)/.
	-mv input.o $(INSTDIR)/.
	-mv output.o $(INSTDIR)/.
	-mv inputFile.o $(INSTDIR)/.
	-mv inputScanner.o $(INSTDIR)/.
	-mv outputDBConnection.o $(INSTDIR)/.
	-mv outputOffline.o $(INSTDIR)/.
	-mv parallelExecutor.o $(INSTDIR)/.
	-mv sha2.o $(INSTDIR)/.
	-mv libhashAlgorithm.so $(INSTDIR)/.
	-mv libinput.so $(INSTDIR)/.
	-mv liboutput.so $(INSTDIR)/.
	-mv libinputFile.so $(INSTDIR)/.
	-mv libinputScanner.so $(INSTDIR)/.
	-mv liboutputDBConnection.so $(INSTDIR)/.
	-mv liboutputOffline.so $(INSTDIR)/.
	-mv libparallelExecutor.so $(INSTDIR)/.
	-mv libsha2.so $(INSTDIR)/.
	-mv testExec $(INSTDIR)/.