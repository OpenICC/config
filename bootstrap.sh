#!/bin/sh
aclocal
libtoolize -f
automake --add-missing
autoconf
