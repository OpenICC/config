# OpenICC

The OpenICC configuration data base allows to store, share and manipulate
colour management informations.

Part of that is a file format based on JSON and a implementation of a 
according library for easy access.

### Features
* access to OpenICC device JSON DB

### Links
* [Copyright](docs/COPYING) - MIT
* [ChangeLog](docs/ChangeLog)
* [Authors](docs/AUTHORS)
* [Code](http://sourceforge.net/p/openicc/code/ci/master/tree/)
* [OpenICC](http://www.openicc.info) - open source color management discussion group
 

### Dependencies
* [Yajl](http://lloyd.github.com/yajl/) - a JSON parser library

### Building
Supported are autotools and cmake style builds.

    $ mkdir build && cd build
    $ ../configure # or
    $ cmake ..
    $ make
    $ make install

    The make check and make coverage targets are provided.

####Build Flags
... are typical cmake flags like CMAKE\_C\_FLAGS to tune compilation.

* CMAKE\_INSTALL\_PREFIX to install into paths and so on. Use on the command 
  line through -DCMAKE\_INSTALL\_PREFIX=/my/path .
* LIB\_SUFFIX - allows to append a architecture specific suffix like 
  LIB\_SUFFIX=64 for 64bit RedHat style Linux systems.

### Known Bugs
The source code provides currently no mechanism for a write lock.
