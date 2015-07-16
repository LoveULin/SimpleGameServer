#!/bin/sh
aclocal
libtoolize --automake
autoconf
automake --add-missing --copy
