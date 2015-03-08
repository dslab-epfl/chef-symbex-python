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

"""Setup script for the Chef Python symbolic test library."""


__author__ = "stefan.bucur@epfl.ch (Stefan Bucur)"


# import os
# import subprocess
# import sys


from distutils.core import setup, Extension
# from distutils.command.build_py import build_py as _build_py
# from distutils.spawn import find_executable


# protoc = find_executable("protoc")


# # Taken from http://protobuf.googlecode.com/svn/trunk/python/setup.py
# def generate_proto(source):
#     """Invokes the Protocol Compiler to generate a _pb2.py from the given
#     .proto file.  Does nothing if the output already exists and is newer than
#     the input."""
#
#     output = source.replace(".proto", "_pb2.py")
#
#     if (not os.path.exists(output) or
#             (os.path.exists(source) and
#                      os.path.getmtime(source) > os.path.getmtime(output))):
#         print "Generating %s..." % output
#
#         if not os.path.exists(source):
#             sys.stderr.write("Can't find required file: %s\n" % source)
#             sys.exit(-1)
#
#         if protoc is None:
#             sys.stderr.write("protoc was not found.  Please compile it or install the binary package.\n")
#             sys.exit(-1)
#
#         protoc_command = [protoc, "-I.", "--python_out=.", source]
#         if subprocess.call(protoc_command) != 0:
#             sys.exit(-1)
#
#
# class build_py(_build_py):
#     def run(self):
#         # Generate necessary .proto file if it doesn't exist.
#         generate_proto("./lib/chef/chef_data.proto")
#
#         # _build_py is an old-style class, so super() doesn't work.
#         _build_py.run(self)


setup(name="ChefSymTest",
      version='0.1',
      description="The Chef symbolic test library",
      author="Stefan Bucur",
      author_email="stefan.bucur@epfl.ch",
      url="http://dslab.epfl.ch",
      package_dir={"": "lib"},
      packages=['chef'],
      ext_package='chef',
      ext_modules=[
          Extension('symbex',
                    sources=['src/symbexmodule.cc',
                             'src/ConcolicSession.cc',
                             'src/S2EGuest.cc',
                             'src/SymbolicUtils.cc'],
                    include_dirs=['include'])
      ],
      # cmdclass={"build_py": build_py}
      )
