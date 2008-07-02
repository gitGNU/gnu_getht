#!/bin/bash
#
# Copyright 2006,2008 Nick White
#
# This file is part of GetHT.
#
# GetHT is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# GetHT is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GetHT.  If not, see <http://www.gnu.org/licenses/>.

TMPDIR="/tmp"
ORIGDIR=$(pwd)

VERSION=$(gawk -F \" '{print $2}' src/version.h | \
	sed -e ':a;N;$!ba;s/\n//g')

echo Packaging GetHT version $VERSION

echo Copying sources to a temporary directory
rm -rf $TMPDIR/getht-$VERSION && \
git clone . $TMPDIR/getht-$VERSION && \
cd $TMPDIR/getht-$VERSION

# ensure version number in configure.ac is set correctly
sed -i "s|^AC_INIT(getht, .*)$|AC_INIT(getht, $VERSION)|" configure.ac

echo Building necessary autotools parts
autoreconf -i

echo Cleaning make environment
make clean

echo Cleaning up working directory
rm -rf autom4te.cache
rm -rf .git* */.git*
rm "$0"

echo Packaging into a tarball
cd ..
tar -cjf getht-$VERSION.tar.bz2 getht-$VERSION

echo Removing temporary directory
rm -rf getht-$VERSION

cd $ORIGDIR
mv $TMPDIR/getht-$VERSION.tar.bz2 .

echo Signing package
gpg -b getht-$VERSION.tar.bz2

echo Checksumming package
sha1sum getht-$VERSION.tar.bz2 > getht-$VERSION.tar.bz2.sha1

echo "Packaging of GetHT $VERSION complete."
echo "The tarball resides at ./getht-$VERSION.tar.bz2"
