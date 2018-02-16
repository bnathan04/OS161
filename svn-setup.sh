#!/bin/bash

PROG=$(basename $0)

function usage() {
    echo "Usage: svn-setup.sh group-nr" 1>&2;
    exit 1;
}

if [ $# -ne 1 ]; then
    usage;
fi

SVN_REPOSITORY_SERVER=ug250.eecg
GROUPNAME="$1"
GROUPDIR=/srv/ece344f/$GROUPNAME
SVNDIR=$GROUPDIR/svn

HOST=$(uname -n)
if [ "$HOST" != $SVN_REPOSITORY_SERVER ]; then
    echo "$PROG: run svn setup on $SVN_REPOSITORY_SERVER" 1>&2;
fi

if [ ! -d $GROUPDIR ]; then
    echo "$PROG: $GROUPDIR does not exist on $SVN_REPOSITORY_SERVER" 1>&2;
    usage;
fi

if ! touch $GROUPDIR > /dev/null 2>&1; then
    echo "$PROG: $GROUPDIR is not writable by you on $SVN_REPOSITORY_SERVER" 1>&2;
    usage;
fi

if [ -d $SVNDIR ]; then
    echo "$PROG: $SVNDIR exists. remove it on $SVN_REPOSITORY_SERVER before running this script." 1>&2;
    usage;
fi

echo "Creating directory: $SVNDIR"
mkdir -p $SVNDIR

echo "Creating svn repository: $SVNDIR"
svnadmin --pre-1.6-compatible create $SVNDIR

cd $SVNDIR
chgrp -R $GROUPNAME .; chmod -R g+w .; chmod g+s db

rm -rf /tmp/importdir$GROUPNAME
mkdir /tmp/importdir$GROUPNAME
cd /tmp/importdir$GROUPNAME
mkdir trunk
mkdir tags
svn import . file://$SVNDIR -m "initial import"
rm -rf /tmp/importdir$GROUPNAME
exit 0
