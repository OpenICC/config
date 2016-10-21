# OpenICC Configuration README
<a href="https://travis-ci.org/OpenICC/config"><img src="https://travis-ci.org/OpenICC/config.svg?branch=master"/></a> <a href="https://codedocs.xyz/OpenICC/config/"><img src="https://codedocs.xyz/OpenICC/config.svg"/></a>

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
* [Code](https://github.com/OpenICC/config)
* [OpenICC](http://www.openicc.info) - open source color management discussion group
 

### Dependencies
* [Yajl](http://lloyd.github.com/yajl/) - a JSON parser library
#### Optional
* [gettext](https://www.gnu.org/software/gettext/) - i18n
* [LCOV](http://ltp.sourceforge.net/coverage/lcov.php) - coverage docu
* For the documentation use doxygen, graphviz and graphviz-gd packages.
  * [Doxygen v1.5.8 or higher is recommended](http://www.doxygen.org)

### Building
Supported are autotools and cmake style builds.

    $ mkdir build && cd build
    $ ../configure # or
    $ cmake ..
    $ make
    $ make install

####Build Flags
... are typical cmake flags like CMAKE\_C\_FLAGS to tune compilation.

* CMAKE\_INSTALL\_PREFIX to install into paths and so on. Use on the command 
  line through -DCMAKE\_INSTALL\_PREFIX=/my/path .
* LIB\_SUFFIX - allows to append a architecture specific suffix like 
  LIB\_SUFFIX=64 for 64bit RedHat style Linux systems.
* USE\_GCOV - enable gcov/lcov compiler flags on the Unix platform and the coverage target

### Known Bugs
* The source code provides currently no mechanism for a write lock.

### TODO
* cascading config files or keys in openiccDB\_s
