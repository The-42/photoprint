#!/bin/sh

: ${AUTORECONF=autoreconf}
AUTORECONFFLAGS="--warnings=all --verbose --install --force $ACLOCAL_FLAGS"

${AUTORECONF} ${AUTORECONFFLAGS} "$@" || exit 1
