#!/bin/sh
aclocal
libtoolize -fc
automake --add-missing -c
autoconf
