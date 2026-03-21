# stask - simple task manager
# See LICENSE file for copyright and license details.

VERSION = 0.2

PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

PKG_CONFIG = pkg-config

# GTK4 and SQLite3
GTKINC = `$(PKG_CONFIG) --cflags gtk4`
GTKLIB = `$(PKG_CONFIG) --libs gtk4`
SQLITEINC = `$(PKG_CONFIG) --cflags sqlite3`
SQLITELIB = `$(PKG_CONFIG) --libs sqlite3`

INCS = ${GTKINC} ${SQLITEINC}
LIBS = ${GTKLIB} ${SQLITELIB}

CPPFLAGS = -D_POSIX_C_SOURCE=200809L -DVERSION=\"${VERSION}\"
CFLAGS   = -std=c99 -pedantic -Wall -Wextra -Os ${INCS} ${CPPFLAGS}
LDFLAGS  = ${LIBS}

CC = cc
