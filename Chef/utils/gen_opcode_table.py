#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2014 EPFL. All rights reserved.

"""Parse Python's opcode definition table."""

__author__ = 'stefan.bucur@epfl.ch (Stefan Bucur)'


import argparse
import re
import sys


NUM_COLUMNS = 3


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("infile", nargs='?', type=argparse.FileType('r'),
                        default=sys.stdin)

    args = parser.parse_args()

    define_re = re.compile(r'#define\s+([A-Z_]+)\s*(\d+)')

    opcodes = {}

    for line in args.infile:
        match = define_re.match(line)
        if match:
            opcodes[int(match.group(2))] = match.group(1)

    max_opcode = max(opcodes.keys())

    print "{"
    for i in range(max_opcode+1):
        print "    {%s}, // %s" % (", ".join(["0"] * NUM_COLUMNS),
                                   opcodes.get(i, "unused"))
    print "}"


if __name__ == "__main__":
    main()
