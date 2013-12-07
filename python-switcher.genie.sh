#!/bin/bash
#
# Copyright 2013 EPFL. All rights reserved.
#
# Switching utility between different Python versions

DIR="$( cd "$( dirname "$0" )" && pwd )"

if [ -z "$PYTHONSYMBEXOPT" ]; then
    PYTHONSYMBEXOPT="4"
fi

export PYTHONSYMBEXOPT

exec ${DIR}/python-opt${PYTHONSYMBEXOPT}-* "$@"
