#! /bin/sh
$EXTRACTRC *.ui >> rc.cpp
$XGETTEXT *.cpp -o $podir/kcmkwallet.pot
