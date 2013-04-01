#! /bin/sh
$EXTRACTRC `find . -name '*.ui' -or -name '*.rc'` >> rc.cpp || exit 11
$XGETTEXT `find . -name '*.h' -o -name '*.cpp' | grep -v '/tests/'` -o $podir/kwalletmanager.pot
rm -f rc.cpp
