#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright (C) 2014 EPFL.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

"""Lightweight symbolic test framework."""

__author__ = "stefan.bucur@epfl.ch (Stefan Bucur)"


import argparse
import cStringIO
import logging
import re
import struct
import sys
import traceback

from chef import symbex
from chef.chef_data_pb2 import TestCase as TestCase_pb2


class SymbolicTest(object):
    """Base class for symbolic tests"""

    def __init__(self, replay=False, replay_assgn=None):
        self.replay = replay
        self.replay_assgn = replay_assgn or {}
        self._log_roll = []

    @property
    def log_roll(self):
        return "".join(self._log_roll)

    def getInt(self, name, default, max_value=None, min_value=None):
        if self.replay:
            if name not in self.replay_assgn:
                logging.info("Key '%s' not found in assignment. Using default '%s'." % (name, default))
                return default
            return int(self.replay_assgn[name])
        elif not (max_value is None and min_value is None):
            return symbex.symint(default, name, max_value, min_value)
        else:
            return symbex.symint(default, name)

    def getString(self, name, default, max_size=None, min_size=None, ascii=False):
        if not isinstance(default, basestring):
            raise ValueError("Default value must be string or unicode")

        if self.replay:
            if name not in self.replay_assgn:
                logging.info("Key '%s' not found in assignment. Using default '%s'." % (name, default))
                return default
            return self.replay_assgn[name]
        elif not (max_size is None and min_size is None):
            value = symbex.symsequence(default, name, max_size, min_size)
        else:
            value = symbex.symsequence(default, name)

        if ascii:
            symbex.assumeascii(value)

        return value

    def log(self, message):
        print "*log* %s" % message
        if self.replay:
            self._log_roll.append(message)
        else:
            symbex.log(message)
    
    def concretize(self, value):
        if self.replay:
            return value
        
        return symbex.concrete(value)

    def setUp(self):
        """Called once before the test execution."""
        pass

    def runTest(self):
        pass


def runFromArgs(symbolic_test, **test_args):
    parser = argparse.ArgumentParser(description="Run or replay symbolic tests.")

    replay_mode = parser.add_mutually_exclusive_group()
    replay_mode.add_argument("-a", action="append", nargs=2, dest='assgn',
                             help="Symbolic value assignment")
    replay_mode.add_argument("-r", action="store_true", dest="replay", default=False,
                             help="Replay with default concolics")
    replay_mode.add_argument("-f", dest="replay_file",
                             help="Replay from file with test cases")
    args = parser.parse_args()

    assignment = {key: value.decode("string-escape") for key, value in (args.assgn or [])}

    if args.replay or assignment or args.replay_file:
        replayer = TestCaseReplayer(symbolic_test, **test_args)
        if args.replay_file:
            with open(args.replay_file, "r") as f:
                test_cases = SymbolicTestCase.from_file(f)
                replayer.replay(test_cases)
        elif args.replay:
            replayer.replay_assignment({})
        else:
            replayer.replay_assignment(assignment)

        replayer.collect()
    else:
        runSymbolic(symbolic_test, **test_args)


def runSymbolic(symbolic_test, max_time=0,  **test_args):
    """Runs a symbolic test in symbolic mode"""

    test_inst = symbolic_test(**test_args)
    test_inst.setUp()

    concolic_session = False
    try:
        symbex.startconcolic(max_time)
        concolic_session = True
    except symbex.SymbexError:
        logging.warning("Cannot start the ConcolicSession. Proceeding without interpreter support.")

    try:
        test_inst.runTest()
    except:
        traceback.print_exc()
        raise
    finally:
        if concolic_session:
            symbex.endconcolic(False)
        else:
            symbex.killstate(0, "Symbolic test ended")


class SymbolicTestCase(object):
    _assignment_name_re = re.compile(r"([^.]*)(?:[.]([is])(?:#(\w+))?)?")  # Values of form: name.k#value

    def __init__(self):
        self._proto_msg = None

        self.time_stamp = None
        self.assignment = {}
        self.output = None

        self.high_level_path_id = None

    @classmethod
    def _decode_assignment(cls, name, value):
        match = cls._assignment_name_re.match(name)
        base_name, kind = match.group(1), (match.group(2) or "s")
        if kind == "s":
            return base_name, value
        elif kind == "i":
            return base_name, struct.unpack("<i", value)[0]
        else:
            raise ValueError("Invalid assignment encoding")

    @classmethod
    def from_protobuf(cls, data):
        message = TestCase_pb2()
        message.ParseFromString(data)

        test_case = cls()
        test_case._proto_msg = message

        test_case.time_stamp = message.time_stamp
        test_case.assignment = dict(cls._decode_assignment(assgn.name, assgn.value)
                                    for assgn in message.input.var_assignment)
        test_case.output = message.output
        test_case.high_level_path_id = message.high_level_path_id

        return test_case

    @classmethod
    def from_file(cls, f):
        header_fmt = "=I"
        header_size = struct.calcsize(header_fmt)

        while True:
            header = f.read(header_size)
            if len(header) < header_size:
                break
            msg_size = struct.unpack(header_fmt, header)[0]
            message = f.read(msg_size)
            if len(message) < msg_size:
                break

            yield cls.from_protobuf(message)


class TestCaseReplayer(object):
    cov_line_re = re.compile(r"""^(.+\S+)\s+  # The file name
                                  (\d+)\s+    # No. of statements
                                  (\d+)\s+    # Missed statements
                                  (\d+)%\s*$  # Total coverage""", re.X)

    def __init__(self, symbolic_test, **test_args):
        self.symbolic_test = symbolic_test
        self.test_args = test_args
        self.errors = []

        import coverage
        self._cov = coverage.coverage(cover_pylib=True, branch=False,
                                      config_file=None, source=None)
        self._cov.start()

    def replay_assignment(self, assignment):
        logging.info("Replaying %s" % assignment)

        test_inst = self.symbolic_test(replay=True, replay_assgn=assignment, **self.test_args)
        test_inst.setUp()

        try:
            test_inst.runTest()
        except:
            logging.exception("Error detected")
            self.errors.append((sys.exc_info()[0].__name__,
                                str(assignment),
                                repr(traceback.format_exc())))

        return test_inst

    def replay_test_case(self, test_case):
        test_inst = self.replay_assignment(test_case.assignment)

        if test_inst.log_roll != test_case.output:
            logging.warning("Mismatched test case output output:")
            logging.warning("Original: %s" % test_case.output)
            logging.warning("Replayed: %s" % test_inst.log_roll)

    def replay(self, test_cases):
        for test_case in test_cases:
            self.replay_test_case(test_case)

    def collect(self):
        self._cov.stop()
        
        result = {}

        buff = cStringIO.StringIO()
        self._cov.report(morfs=None, show_missing=False,
                         file=buff,
                         omit=None, include=None)

        for line in buff.getvalue().splitlines():
            match = self.cov_line_re.match(line)
            if not match:
                continue

            file_name = match.group(1)
            if file_name == "TOTAL":
                continue # XXX: Hack, hack, there might be a file named "TOTAL"
            logging.debug("  Processing coverage for '%s'" % file_name)

            # XXX: Not very nice either, but the coverage module is quite
            # cumbersome to use for non-trivial tasks.
            analysis = self._cov.analysis2(file_name + ".py")

            result[file_name] = {
                "executable": analysis[1],
                "excluded": analysis[2],
                "missing": analysis[3],
            }

        buff.close()

        return result
