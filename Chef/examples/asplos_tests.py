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

"""Symbolic tests for ASPLOS'14."""

__author__ = "stefan.bucur@epfl.ch (Stefan Bucur)"

# TODO: Rename this file more appropriately.


import argparse
import cStringIO
import importlib
import logging
import os
import sys

from chef import light

# TODO: Devise more meaningful defaults.
# Idea: use a method to transform a concrete input into a wildcard of the same length.


class ConfigParserTest(light.SymbolicTest):
    def setUp(self):
        self.ConfigParser = importlib.import_module("ConfigParser")

    def runTest(self):
        input_string = self.getString("input", '\x00'*10)
        string_file = cStringIO.StringIO(input_string)
        
        config = self.ConfigParser.ConfigParser()
        config.readfp(string_file)
        
        for s in config.sections():
            config.options(s)


class ArgparseTest(light.SymbolicTest):
    def setUp(self):
        self.argparse = importlib.import_module("argparse")
    
    def runTest(self):
        parser = self.argparse.ArgumentParser(description="Symtest")
        parser.add_argument(self.getString("arg1_name", '\x00'*3))
        parser.add_argument(self.getString("arg2_name", '\x00'*3))
        
        parser.parse_args([self.getString("arg1", '\x00'*3),
                           self.getString("arg2", '\x00'*3)])


class HTMLParserTest(light.SymbolicTest):
    def setUp(self):        
        self.HTMLParser = importlib.import_module("HTMLParser")
    
    def runTest(self):
        parser = self.HTMLParser.HTMLParser()
        parser.feed(self.getString("html", '\x00'*15))
        parser.close()


################################################################################
# Third-party libraries

class SimpleJSONTest(light.SymbolicTest):
    def setUp(self):
        self.simplejson = importlib.import_module("simplejson")
        
    def runTest(self):
        self.simplejson.loads(self.getString("input", '\x00'*15))


class XLRDTest(light.SymbolicTest):
    def setUp(self):
        self.xlrd = importlib.import_module("xlrd")
        
    def runTest(self):
        i = self.getString("input", '\x00'*20)
        self.xlrd.open_workbook(file_contents=i)


class UnicodeCSVTest(light.SymbolicTest):
    def setUp(self):
        self.unicodecsv = importlib.import_module("unicodecsv")
        
    def runTest(self):        
        f = cStringIO.StringIO(self.getString("input", '\x00'*5))
        r = self.unicodecsv.reader(f, encoding="utf-8")
        for row in r:
            pass
        f.close()


##################
# Validation tests


class ValidationLeaningTree(light.SymbolicTest):
    def setUp(self):
        pass

    def runTest(self):
        i = self.getInt("value", 100, max_value=100, min_value=0)

        if i < 50:
            if i < 25:
                if i < 12:
                    if i < 6:
                        if i < 3:
                            self.log("1")
                        else:
                            self.log("2")
                    else:
                        self.log("3")
                else:
                    self.log("4")
            else:
                self.log("5")
        else:
            self.log("6")


class ValidationBalancedTree(light.SymbolicTest):
    def setUp(self):
        pass

    def runTest(self):
        i = self.getInt("value", 100, max_value=100, min_value=0)

        if i < 50:
            if i < 25:
                if i < 12:
                    self.log("1")
                else:
                    self.log("2")
            else:
                if i < 37:
                    self.log("3")
                else:
                    self.log("4")
        else:
            if i < 75:
                if i < 62:
                    self.log("5")
                else:
                    self.log("6")
            else:
                if i < 87:
                    self.log("7")
                else:
                    self.log("8")


class ValidationBinarySearch(light.SymbolicTest):
    def setUp(self):
        pass

    def runTest(self):
        i = self.getInt("value", 100, max_value=100, min_value=0)

        left, right = 0, 100
        steps = 0
        while True:
            middle = (left + right) / 2
            if middle == i:
                self.log("Found %d in %d steps" % (middle, steps))
                break
            elif middle < i:
                left = middle + 1
            else:
                right = middle - 1
            steps += 1


class ValidationStringOps(light.SymbolicTest):
    def setUp(self):
        pass

    def runTest(self):
        s = self.getString("value", "x" * self.sym_size)

        if "s" in s:
            if "t" in s:
                self.log("1")
            else:
                self.log("2")
        elif s.endswith("abcdef"):
            if s.startswith("xyzt"):
                self.log("3")
            else:
                self.log("4")
        elif "mymy" in s:
            self.log("5")
        else:
            self.log("6")


class ValidationObjectAttrs(light.SymbolicTest):
    def setUp(self):
        pass

    def runTest(self):
        class Namespace(object):
            def __init__(self):
                pass

        attr_set = self.getString("setter", "123")
        attr_get = self.getString("getter", "456")

        o = Namespace()
        setattr(o, attr_set, "Here I am!")
        v = getattr(o, attr_get, "Bad luck")

        if v == "Here I am!":
            self.log("Excellent")
        else:
            self.log("Oh well...")


class ValidationStringCapitalization(light.SymbolicTest):
    def setUp(self):
        pass

    def runTest(self):
        s = self.getString("value", "1234567890")
        if s.lower().startswith("abc"):
            self.log(self.concretize(s))
        elif s.upper().endswith("DEF"):
            self.log(self.concretize(s))
        elif s.lower() == s.upper():
            self.log(self.concretize(s))
        else:
            self.log(self.concretize(s))


def main():
    logging.basicConfig(level=logging.INFO, format="** %(message)s")

    parser = argparse.ArgumentParser(description="Run tests")
    parser.add_argument("--interactive", "-i", action="store_true", default=False,
                        help="Do not automatically end concolic session")
    parser.add_argument("test",
                        help="The test class to execute")
    args, remaining_args = parser.parse_known_args()

    test_class = globals().get(args.test)
    if not (test_class and
            isinstance(test_class, type) and
            issubclass(test_class, light.SymbolicTest)):
        print >>sys.stderr, "Invalid test name '%s'." % args.test
        sys.exit(1)

    light.runFromArgs(test_class, arg_list=remaining_args)


if __name__ == "__main__":
    main()
