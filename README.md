# File Identification System

# General concepts

The goal of this project is to develop set of applications used for a file integrity checking, system file validation and suspicious file identification.
Applications perform scan of an user-defined file space in order to compute unique identifier for every file, based on its contents.
Then query on well maintained reference database is executed and all the information available for particular file is given to an user.
Using database with knowledge of all the necessary valid system files based on system version, this software is able to identify 
suspicious system files modified by malware or not permitted subject that can be potencially dangerous for the system and difficult
to detect.

# Linux Instalation

## Install all dependencies

### User

Redhat:

`dnf install gcc-c++ make cryptopp mariadb-connector-c`

Debian:

`apt-get install build-essential libcrypto++ libmariadb2`

### Developer

Redhat:

`dnf install gcc-c++ make cryptopp cryptopp-devel mariadb-connector-c mariadb-connector-c-devel`

Debian:

`apt-get install build-essential libcrypto++ libcrypto++-dev libmariadb2 libmariadb-dev`

## Compile

```
cd <project-dir>

make
make install
```

### Makefile options

| Option         | Description                                                           |
| :--------------| :---------------------------------------------------------------------|
| `install`      | Install to `$INSTDIR` after compilation.                              |
| `clean`        | Remove compiled files for new clean building.                         |
| `install-web`  | Install web-application source code to web server's content directory |

Web-server's content directory is by `/var/www/html/` by default and can be redefined in `Makefile`.

# Windows Instalation

## Install all dependencies

Typical Windows OS installation doesn't contain some tools necessary for project build.

### Compilers and tools

`File-Identification-System` is built using `Mingw-w64`. Installation wizard can be obtained at 
`https://vorboss.dl.sourceforge.net/project/mingw-w64/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/installer/mingw-w64-install.exe`.

`SysUpdate.exe` is built using `cl.exe`. MSI package can be downloaded at `https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2019`.
Choose `Build Tools for Visual Studio 2019` to download installation wizard for command-line tools only.

Build management is performed using `make` tool. Installation wizard can be downloaded at 
`https://sourceforge.net/projects/gnuwin32/files/make/3.81/make-3.81.exe/download?use_mirror=jztkft&download=`.

`Cmake` needs to be installed to build some appliation dependencies. MSI pakage can be downloaded at `https://cmake.org/download/`.

### Application dependencies

Various dependencies need to be installed:

`MariaDB-Connector-C` library can be installed using MSI packages. Package targeting correct architecture can be downloaded at
`https://mariadb.com/downloads/#connectors`.

`Cryptopp` cryptographic library sources can be downloaded at `https://www.cryptopp.com/`.
Source files need to be built using cmake. 

## Compile

Once all dependenies are installed, using Microsoft Build Tools command-line perform:

```
cd <project-dir>

make.exe all
make.exe install
```

### Makefile options

| Option         | Description                                                           |
| :------------- | :---------------------------------------------------------------------|
| `all`          | Build all applications                                                |
| `default`      | Build `File-Identification-System.exe` application                    |
| `util`         | Build `SysUpdate.exe` application                                     |
| `install`      | Install to `$INSTDIR` after compilation.                              |
| `clean`        | Remove compiled files for new clean building.                         |
| `install-web`  | Install web-application source code to web server's content directory |

Web-server's content directory is `UNDEFINED` by default and can be redefined in `Makefile`.


# Applications and components

## File-Identification-System

This is a key application that performs recursive search through the observed file system, hash values computation based on file contents
and file metadata processing in three modes. 

`Offline mode` creates an output file containing a list of found files and their corresponding hash values. This mode can be useful, if no 
internet connection is available for connetion with reference database. Output list can be used as the application input later in order to 
analyze found files.

`Validation mode` is a `DEFAULT` mode. It loads files for observation either from file system or from a file created in `Offline mode`.
Input files and their corresponding hash values are compared against pre-created a referenced database.
Output information contains file status and file metadata that could be obtained from database and should help to further identify the file.
File can be granted one of several statuses:

* VALID - file and its contents are known and exactly the same compared to the record in reference database,
* WARNING - file contents are known to reference database, but its name or location in the file system is different,
* SUSPICIOUS - file name and location in file system is known to reference database, however its corresponding file contents are unknown and could have been modified,
* UNKNOWN - reord describing the file does not exist in reference database,
* ERROR - error occured when trying to evaluate file

`Update mode` loads files for observation either from file system or from a file created in `Offline mode`. Further it obtains as much file-related metadata from 
file system as possible and inserts found information to reference database. This mode is used for maintenance of reference database and should be used on trusted
devices exclusively.

To list application options, type (Linux):

```
cd <project-dir>
./build/File-Identification-System -h
```

To list application options, type (Windows):

```
cd <project-dir>
./build/File-Identification-System.exe -h
```

Once options are set, application will obtain all the necessary configuration during user-interactive setup.

## SysUpdate.exe

This is a simple application used to perform controlled update of Windows OS (Windows XP and later).
It triggers download and installation of all non-installed updates. Can be used also to list available updates.

To list application options type:

```
cd <project-dir>
./build/SysUpdate.exe -h
```

## SystemSnaphotManager

`SystemSnapshotManager` is a script used to automate continuous maintenance of reference database. It triggers system update using `SysUpdate.exe`, 
updates other appliations using custom scrips in `<project-dir>/scripts/custom/` folder and finaly records current state of observed portion of file system 
into reference database using `File-Identification-System.exe`. This script should be started on device boot and once the task is done, device is powered off automatically. The process is recorded in log file. One can create a vitualization server and periodially boot all virtual machines with different architectures 
one after another in order to populate reference database with new data.

To list script options, type:

```
cd <project-dir>
./scripts/SystemSnapshotManager.ps1 -h
```

## Reference database

To install a reference database, MariaDB database system must be installed on a device.

Scripts in `<project-dir>/sql/` directory can be then used to build a reference database. Script `db-schema.sql` creates relational schema of database and
related SQL views and procedures. Sript `db-index.sql` creates indexes to speed up the search in database. Indexes can slow down the process of database population,
so this script should be used once, after major initial inserts are done.

## Web application

To install web application, a web server must be intalled on a device.
Web application is used to visualize outputs of `File Identification System` console applications. It also provides functionality to upload and validate file
from file system as well as perform validation of list of files created in `Offline mode`.

Web application source code needs to be copied to web server's content directory. Web server's content directory can be set in `Makefile`.
Necessary source code can be then copied using following command:

* Linux
```
make install-web
```
* Windows
```
make.exe install-web
```

# Contacts

For further information, please contact `<matej.fedor.mf@gmail.com>`.
