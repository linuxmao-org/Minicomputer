#!/bin/sh -e
set -x

po_update() {
    if test -f "$1"; then
        msgmerge -U "$1" "$1"t
    else
        msginit -i "$1"t -l "$locale" -o "$1"
    fi
}

for locale in "fr"; do
    mkdir -p po/"$locale"
    xgettext -k_ -L C++ -c -s -o po/"$locale"/minicomputer.pot editor/*.{h,cpp,cxx}
    po_update po/"$locale"/minicomputer.po
    rm -f po/"$locale"/*.pot
done
