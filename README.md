# File System Scanner

# General concepts

The goal of this project is to develop set of applications used for a system file validation and suspicious file identification.
Applications perform scan of an user-defined file space in order to compute unique identifier for every file based on its contents.
Then query on well maintained database is executed and all the information available for particular file is given to an user.
Using database with knowledge of all the necessary valid system files based on system version, this software is able to identify 
suspicious system files modified by malware or not permitted subject that can be potencially dangerous for the system and difficult
to detect.

# Instalation

## Install all dependencies

Redhat:

`dnf install cryptopp cryptopp-devel mariadb-connector-c mariadb-connector-c-devel`

Debian:

`apt-get install libcrypto++ libcrypto++-dev libmariadb2 libmariadb-dev`

## Compile

Place this project directory in `$HOME/git/` or define respective custom directories in `Makefile`.

```
CURRENTDIR=$(HOME)/git/File-System-Scanner/
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

## Example of running testing executable

```
cd <project-dir>
./build/testExec <search-root-directory>
```
