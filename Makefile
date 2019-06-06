INSTDIR=$(HOME)/git/File-System-Scanner/build

testExec: src/testExec.cpp src/Input.cpp src/InputFile.cpp src/InputScanner.cpp src/Output.cpp src/OutputDBConnection.cpp src/OutputOffline.cpp\
	src/HashAlgorithm.cpp src/SHA2.cpp src/ParallelExecutor.cpp
	gcc -o $@ $? -lcryptopp -lmysqlclient -lpthread -lstdc++ -pedantic -Wall -Wextra

.PHONY: clean install
clean:
	-rm testExec

install:
	mkdir -p $(INSTDIR)/.
	-mv testExec $(INSTDIR)/.