ifeq ($(OS), Windows_NT)
	INSTDIR=$(HOMEPATH)/git/File-System-Scanner/build
	CRYPTOPP_INCLUDE_DIR=C:/Program\ Files/Cryptopp
	MYSQL_INCLUDE_DIR=C:/Program\ Files/MariaDB/MariaDB\ Connector\ C\ 64-bit/include
	TARGET=fss.exe
endif
ifeq ($(shell uname), Linux)
	INSTDIR=$(HOME)/git/File-System-Scanner/build
	TARGET=fss
endif

default: $(TARGET)

fss: src/FSS.cpp src/Input.cpp src/InputFile.cpp src/InputScanner.cpp src/Output.cpp src/OutputDBConnection.cpp\
	src/OutputOffline.cpp src/HashAlgorithm.cpp src/SHA2.cpp src/ParallelExecutor.cpp
	gcc -o $@ $? -lcryptopp -lmysqlclient -lpthread -lstdc++ -pedantic -Wall -Wextra

fss.exe: src/FSS.cpp src/Input.cpp src/InputFile.cpp src/InputScanner.cpp src/Output.cpp src/OutputDBConnection.cpp\
	src/OutputOffline.cpp src/HashAlgorithm.cpp src/SHA2.cpp src/ParallelExecutor.cpp
	gcc -o $@ $? -I $(MYSQL_INCLUDE_DIR) -I $(CRYPTOPP_INCLUDE_DIR) -lcryptopp -lmysqlclient -lpthread -lstdc++ -pedantic -Wall -Wextra

.PHONY: clean install
clean:
	-rm $(TARGET)

install:
	mkdir -p $(INSTDIR)/.
	-mv $(TARGET) $(INSTDIR)/.
