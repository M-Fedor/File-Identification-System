ifeq ($(OS), Windows_NT)
	INSTDIR=build
	CRYPTOPP_INCLUDE_DIR=C:/Program\ Files/Cryptopp
	CRYPTOPP_LIB_DIR=$(CRYPTOPP_INCLUDE_DIR)
	MYSQL_INCLUDE_DIR=C:/Program\ Files/MariaDB/MariaDB\ Connector\ C\ 64-bit/include
	MYSQL_LIB_DIR=C:/Program\ Files/MariaDB/MariaDB\ Connector\ C\ 64-bit/lib
	PLATFORM=Win
	TARGET=File-Identification-System.exe
	WEB_INST_DIR=
endif
ifeq ($(shell uname), Linux)
	INSTDIR=build
	PLATFORM=Unix
	TARGET=File-Identification-System
	WEB_INST_DIR=/var/www/html/
endif

default: $(TARGET)

all: default util

util: sysUtility/SysUpdate.cpp sysUtility/SysUpdateImp.cpp  src/Utils.cpp
	cl.exe -W4 -EHsc -permissive- $? -link ole32.lib oleAut32.lib

File-Identification-System: src/FileIdentificationSystem.cpp src/Input.cpp src/InputFile.cpp src/InputScanner.cpp\
	src/HashAlgorithm.cpp src/SHA2.cpp\
	src/DBConnection.cpp src/Output.cpp src/OutputOffline.cpp src/OutputValidateDB.cpp\
	src/ParallelExecutor.cpp src/Utils.cpp
	gcc -o $@ $? -lcryptopp -lmysqlclient -lpthread -lstdc++ -std=c++11 -pedantic -Wall -Wextra

File-Identification-System.exe: src/FileIdentificationSystem.cpp src/Input.cpp src/InputFile.cpp src/InputScanner.cpp\
	src/HashAlgorithm.cpp src/SHA2.cpp\
	src/DBConnection.cpp src/Output.cpp src/OutputOffline.cpp src/OutputUpdateDB.cpp src/OutputValidateDB.cpp\
	src/ParallelExecutor.cpp src/Utils.cpp
	gcc -o $@ $? -I $(CRYPTOPP_INCLUDE_DIR) -I $(MYSQL_INCLUDE_DIR) -L $(CRYPTOPP_LIB_DIR) -Wl,-Bstatic -lcryptopp \
	-L $(MYSQL_LIB_DIR) -llibmariadb -lpthread -lstdc++ -std=c++11 -Wl,-Bdynamic -lversion -pedantic -Wall -Wextra

.PHONY: clean install install-web

clean: clean-$(PLATFORM)
install: install-$(PLATFORM)

clean-Unix:
	-rm $(TARGET)

clean-Win: 
	-rm $(TARGET)
	-rm SysUpdate.exe
	-rm SysUpdate.obj
	-rm SysUpdateImp.obj
	-rm Utils.obj

install-Unix:
	mkdir -p $(INSTDIR)/.
	-mv $(TARGET) $(INSTDIR)/.

install-web:
	cp -r webUtility/. $(WEB_INST_DIR)

install-Win:
	mkdir -p $(INSTDIR)/.
	-mv $(TARGET) $(INSTDIR)/.
	-mv SysUpdate.exe $(INSTDIR)/.
	-mv SysUpdate.obj $(INSTDIR)/.
	-mv SysUpdateImp.obj $(INSTDIR)/.
	-mv Utils.obj $(INSTDIR)/.
	-cp conf/$(TARGET).manifest $(INSTDIR)/.

