ifeq ($(OS), Windows_NT)
	INSTDIR=$(USERPROFILE)/git/File-System-Scanner/build
	CRYPTOPP_INCLUDE_DIR=C:/Program\ Files/Cryptopp
	CRYPTOPP_LIB_DIR=$(CRYPTOPP_INCLUDE_DIR)
	MYSQL_INCLUDE_DIR=C:/Program\ Files/MariaDB/MariaDB\ Connector\ C\ 64-bit/include
	MYSQL_LIB_DIR=C:/Program\ Files/MariaDB/MariaDB\ Connector\ C\ 64-bit/lib
	TARGET=fis.exe
endif
ifeq ($(shell uname), Linux)
	INSTDIR=$(HOME)/git/File-System-Scanner/build
	TARGET=fis
endif

default: $(TARGET)

fis: src/FIS.cpp src/Input.cpp src/InputFile.cpp src/InputScanner.cpp\
	src/HashAlgorithm.cpp src/SHA2.cpp\
	src/DBConnection.cpp src/Output.cpp src/OutputOffline.cpp src/OutputValidateDB.cpp\
	src/ParallelExecutor.cpp src/Utils.cpp
	gcc -o $@ $? -lcryptopp -lmysqlclient -lpthread -lstdc++ -pedantic -Wall -Wextra

fis.exe: src/FIS.cpp src/Input.cpp src/InputFile.cpp src/InputScanner.cpp\
	src/HashAlgorithm.cpp src/SHA2.cpp\
	src/DBConnection.cpp src/Output.cpp src/OutputOffline.cpp src/OutputUpdateDB.cpp src/OutputValidateDB.cpp\
	src/ParallelExecutor.cpp src/Utils.cpp
	gcc -o $@ $? -I $(CRYPTOPP_INCLUDE_DIR) -I $(MYSQL_INCLUDE_DIR) -L $(CRYPTOPP_LIB_DIR) -lcryptopp -L $(MYSQL_LIB_DIR)\
    -llibmariadb -lpthread -lstdc++ -lversion -pedantic -Wall -Wextra

.PHONY: clean install
clean:
	-rm $(TARGET)

install:
	mkdir -p $(INSTDIR)/.
	-mv $(TARGET) $(INSTDIR)/.
	-cp conf/$(TARGET).manifest $(INSTDIR)/.
