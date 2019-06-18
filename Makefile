INSTDIR=$(HOME)/git/File-System-Scanner/build

fss: src/FSS.cpp src/Input.cpp src/InputFile.cpp src/InputScanner.cpp src/Output.cpp src/OutputDBConnection.cpp\
	src/OutputOffline.cpp src/HashAlgorithm.cpp src/SHA2.cpp src/ParallelExecutor.cpp
	gcc -o $@ $? -lcryptopp -lmysqlclient -lpthread -lstdc++ -pedantic -Wall -Wextra

.PHONY: clean install
clean:
	-rm fss

install:
	mkdir -p $(INSTDIR)/.
	-mv fss $(INSTDIR)/.