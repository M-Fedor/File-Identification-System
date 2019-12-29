# File Identification System

# General concepts

The goal of this project is to develop set of applications used for a file integrity checking, system file validation and suspicious file identification.
Applications perform scan of an user-defined file space in order to compute unique identifier for every file based on its contents.
Then query on well maintained reference database is executed and all the information available for particular file is given to an user.
Using database with knowledge of all the necessary valid system files based on system version, this software is able to identify 
suspicious system files modified by malware or not permitted subject that can be potencially dangerous for the system and difficult
to detect.

# Instalation

## Install all dependencies

### User

Redhat:

`dnf install cryptopp mariadb-connector-c`

Debian:

`apt-get install libcrypto++ libmariadb2`

### Developer

Redhat:

`dnf install cryptopp cryptopp-devel mariadb-connector-c mariadb-connector-c-devel`

Debian:

`apt-get install libcrypto++ libcrypto++-dev libmariadb2 libmariadb-dev`

## Compile

Place this project directory in `$HOME/git/` or define respective custom directory in `Makefile`.

```
INSTDIR=$(HOME)/git/File-System-Scanner/build
```
Then change to project directory on your local system.

```
cd <project-dir>

make
make install
```

### Makefile options

| Option    | Description                                   |
| :-------- | :-------------------------------------------- |
| `install` | Install to `$INSTDIR` after compilation.      |
| `clean`   | Remove compiled files for new clean building. |

## Example of running File Identification System

```
cd <project-dir>
./build/fis <options>
```

Application will obtain all the necessary configuration during user-interactive setup.

For further information, please contact `<matej.fedor.mf@gmail.com>`.
