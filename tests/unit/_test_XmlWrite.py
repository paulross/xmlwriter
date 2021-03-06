#!/usr/bin/env python
# CPIP is a C/C++ Preprocessor implemented in Python.
# Copyright (C) 2008-2017 Paul Ross
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
# 
# Paul Ross: apaulross@gmail.com
__author__  = 'Paul Ross'
__date__    = '2009-09-15'
__rights__  = 'Copyright (c) Paul Ross'

"""Tests XmlWrite.

Usage, from project root:

MACOSX_DEPLOYMENT_TARGET=10.9 python setup.py build_ext -f --inplace
PYTHONPATH=. pytest -vs --benchmark-name=long --benchmark-sort=name tests/unit/
"""

import os
import sys
import time
import logging
import io

# try:
#     import cXmlWrite as XmlWrite
# except ImportError:
#     from xmlwriter import XmlWrite


######################
# Section: Unit tests.
######################
import unittest

class TestXmlWrite_encode_decode(unittest.TestCase):
    """Tests XmlWrite encode and decode string."""
    def test_encodeString(self):
        self.assertEqual(XmlWrite.encodeString('foo'), '_Zm9v')
        self.assertEqual(XmlWrite.encodeString('foo', '_'), '_Zm9v')
        self.assertEqual(XmlWrite.encodeString('foo', '+'), '+Zm9v')
        self.assertEqual(XmlWrite.encodeString('http://www.w3.org/TR/1999/REC-html401-19991224/types.html#type-cdata'),
                         '_aHR0cDovL3d3dy53My5vcmcvVFIvMTk5OS9SRUMtaHRtbDQwMS0xOTk5MTIyNC90eXBlcy5odG1sI3R5cGUtY2RhdGE_')

    def test_encodeString_bad_prefix_raises(self):
        self.assertTrue(XmlWrite.RAISE_ON_ERROR)
        self.assertRaises(XmlWrite.ExceptionXml, XmlWrite.encodeString, 'foo', 'bar')

    def test_decodeString(self):
        self.assertEqual(XmlWrite.decodeString('_Zm9v'), b'foo')

    def test_nameFromString(self):
        self.assertEqual(XmlWrite.nameFromString('foo'), 'ZZm9v')
        self.assertEqual(XmlWrite.nameFromString('http://www.w3.org/TR/1999/REC-html401-19991224/types.html#type-cdata'),
                         'ZaHR0cDovL3d3dy53My5vcmcvVFIvMTk5OS9SRUMtaHRtbDQwMS0xOTk5MTIyNC90eXBlcy5odG1sI3R5cGUtY2RhdGE_')

class TestXmlWrite(unittest.TestCase):
    """Tests XmlWrite."""
    def test_00(self):
        """TestXmlWrite.test_00(): construction."""
        with XmlWrite.XmlStream() as xS:
            pass
        #print
        #print myF.getvalue()
        self.assertEqual(xS.getvalue(), """<?xml version='1.0' encoding="utf-8"?>\n""")
        
    def test_01(self):
        """TestXmlWrite.test_01(): simple elements."""
        with XmlWrite.XmlStream() as xS:
            with XmlWrite.Element(xS, 'Root', {'version' : '12.0'}):
                with XmlWrite.Element(xS, 'A', {'attr_1' : '1'}):
                    pass
        #print
        #print myF.getvalue()
        self.assertEqual(xS.getvalue(), """<?xml version='1.0' encoding="utf-8"?>
<Root version="12.0">
  <A attr_1="1" />
</Root>
""")
       
    def test_02(self):
        """TestXmlWrite.test_02(): mixed content."""
        with XmlWrite.XmlStream() as xS:
            with XmlWrite.Element(xS, 'Root', {'version' : '12.0'}):
                with XmlWrite.Element(xS, 'A', {'attr_1' : '1'}):
                    xS.characters(u'<&>')
        #print
        #print myF.getvalue()
        self.assertEqual(xS.getvalue(), """<?xml version='1.0' encoding="utf-8"?>
<Root version="12.0">
  <A attr_1="1">&lt;&amp;&gt;</A>
</Root>
""")
       
    def test_03(self):
        """TestXmlWrite.test_03(): processing instruction."""
        with XmlWrite.XmlStream() as xS:
            with XmlWrite.Element(xS, 'Root', {'version' : '12.0'}):
                with XmlWrite.Element(xS, 'A', {'attr_1' : '1'}):
                    xS.pI('Do <&> this')
        #print
        #print myF.getvalue()
        self.assertEqual(xS.getvalue(), """<?xml version='1.0' encoding="utf-8"?>
<Root version="12.0">
  <A attr_1="1"><?Do &lt;&amp;&gt; this?></A>
</Root>
""")
        
    def test_04(self):
        """TestXmlWrite.test_04(): raise on endElement when empty."""
        with XmlWrite.XmlStream() as xS:
            pass
        #print
        #print myF.getvalue()
        self.assertRaises(XmlWrite.ExceptionXmlEndElement, xS.endElement, '')
        
    def test_05(self):
        """TestXmlWrite.test_05(): raise on endElement missmatch."""
        with XmlWrite.XmlStream() as xS:
            with XmlWrite.Element(xS, 'Root', {'version' : '12.0'}):
                self.assertRaises(XmlWrite.ExceptionXmlEndElement, xS.endElement, 'NotRoot')
                with XmlWrite.Element(xS, 'A', {'attr_1' : '1'}):
                    self.assertRaises(XmlWrite.ExceptionXmlEndElement, xS.endElement, 'NotA')
        #print
        #print myF.getvalue()
        self.assertEqual(xS.getvalue(), """<?xml version='1.0' encoding="utf-8"?>
<Root version="12.0">
  <A attr_1="1" />
</Root>
""")
                       
    def test_06(self):
        """TestXmlWrite.test_06(): encoded text in 'latin-1'."""
        with XmlWrite.XmlStream('latin-1') as xS:
            with XmlWrite.Element(xS, 'Root'):
                with XmlWrite.Element(xS, 'A'):
                    xS.characters("""<&>"'""")
#                 with XmlWrite.Element(xS, 'A'):
#                     xS.characters('%s' % chr(147))
#                 with XmlWrite.Element(xS, 'A'):
#                     xS.characters(chr(65))
#                 with XmlWrite.Element(xS, 'A'):
#                     xS.characters(chr(128))
#         print()
#         print(repr(myF.getvalue()))
        # FIXME: This test is correct
#         self.assertEqual("""<?xml version='1.0' encoding="latin-1"?>
# <Root>
#   <A>&lt;&amp;&gt;&quot;&apos;</A>
#   <A>&#147;</A>
#   <A>A</A>
#   <A>&#128;</A>
# </Root>
# """,
#             myF.getvalue(),
#         )
       
    def test_07(self):
        """TestXmlWrite.test_07(): comments."""
        with XmlWrite.XmlStream() as xS:
            with XmlWrite.Element(xS, 'Root', {'version' : '12.0'}):
                xS.comment(u' a comment ')
        #print
        #print myF.getvalue()
        self.assertEqual(xS.getvalue(), """<?xml version='1.0' encoding="utf-8"?>
<Root version="12.0"><!-- a comment -->
</Root>
""")
       
    def test_08(self):
        """TestXmlWrite.test_08(): raise during write."""
        try:
            with XmlWrite.XmlStream() as xS:
                with XmlWrite.Element(xS, 'Root', {'version' : '12.0'}):
                    self.assertRaises(XmlWrite.ExceptionXmlEndElement, xS.endElement, 'NotRoot')
                    with XmlWrite.Element(xS, 'E', {'attr_1' : '1'}):
                        xS._elemStk.pop()
                        xS._elemStk.append('F')
                        # raise Exception('Some exception')
        except Exception as e:
            # print(e)
            pass
        else:
            print('No exception raised')
#        print()
#        print(myF.getvalue())
        self.assertEqual(xS.getvalue(), """<?xml version='1.0' encoding="utf-8"?>
<Root version="12.0">
  <E attr_1="1" />
</Root>
""")
                       
    def test_09(self):
        """TestXmlWrite.test_09(): literal."""
        with XmlWrite.XmlStream() as xS:
            with XmlWrite.Element(xS, 'Root', {'version' : '12.0'}):
                xS.literal(u'literal&nbsp;text')
        # print()
        # print(myF.getvalue())
        self.assertEqual(xS.getvalue(), """<?xml version='1.0' encoding="utf-8"?>
<Root version="12.0">literal&nbsp;text</Root>
""")



class TestXhtmlWrite(unittest.TestCase):
    """Tests TestXhtmlWrite."""
    def test_00(self):
        """TestXhtmlWrite.test_00(): construction."""
        with XmlWrite.XhtmlStream() as xS:
            pass
        result = xS.getvalue()
        expected = """<?xml version='1.0' encoding="utf-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html lang="en" xml:lang="en" xmlns="http://www.w3.org/1999/xhtml" />
"""
#         print()
#         print(result)
#         print(expected)
        self.assertEqual(result, expected)
        
    def test_01(self):
        """TestXhtmlWrite.test_01(): simple example."""
        with XmlWrite.XhtmlStream() as xS:
            with XmlWrite.Element(xS, 'head'):
                with XmlWrite.Element(xS, 'title'):
                    xS.characters(u'Virtual Library')
            with XmlWrite.Element(xS, 'body'):
                with XmlWrite.Element(xS, 'p'):
                    xS.characters(u'Moved to ')
                    with XmlWrite.Element(xS, 'a', {'href' : 'http://example.org/'}):
                        xS.characters(u'example.org')
                    xS.characters(u'.')
        #print
        #print myF.getvalue()
        self.assertEqual(xS.getvalue(), """<?xml version='1.0' encoding="utf-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html lang="en" xml:lang="en" xmlns="http://www.w3.org/1999/xhtml">
  <head>
    <title>Virtual Library</title>
  </head>
  <body>
    <p>Moved to <a href="http://example.org/">example.org</a>.</p>
  </body>
</html>
""")

    def test_02(self):
        attrs = {
            'id' : 'ZaHR0cDovL3d3dy53My5vcmcvVFIvMTk5OS9SRUMtaHRtbDQwMS0xOTk5MTIyNC90eXBlcy5odG1sI3R5cGUtY2RhdGE_',
            'foo' : 'bar',
            'baz' : 'long_attribute_that_goes_on_and_on_and_on_and_on_and_on_and_on_and_on',
            'name' : 'George "Shotgun" Ziegler',
        }
        with XmlWrite.XhtmlStream() as xS:
            with XmlWrite.Element(xS, 'head', attrs):
                pass
#         print()
#         print(xS.getvalue())
        expected = """<?xml version='1.0' encoding="utf-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html lang="en" xml:lang="en" xmlns="http://www.w3.org/1999/xhtml">
  <head baz="long_attribute_that_goes_on_and_on_and_on_and_on_and_on_and_on_and_on" foo="bar" id="ZaHR0cDovL3d3dy53My5vcmcvVFIvMTk5OS9SRUMtaHRtbDQwMS0xOTk5MTIyNC90eXBlcy5odG1sI3R5cGUtY2RhdGE_" name="George &quot;Shotgun&quot; Ziegler" />
</html>
"""
        assert xS.getvalue() == expected
       
    def test_charactersWithBr_00(self):
        """TestXhtmlWrite.test_00(): simple example."""
        with XmlWrite.XhtmlStream() as xS:
            with XmlWrite.Element(xS, 'head'):
                pass
            with XmlWrite.Element(xS, 'body'):
                with XmlWrite.Element(xS, 'p'):
                    xS.charactersWithBr(u'No break in this line.')
                with XmlWrite.Element(xS, 'p'):
                    xS.charactersWithBr(u"""Several
breaks in
this line.""")           
                with XmlWrite.Element(xS, 'p'):
                    xS.charactersWithBr(u'\nBreak at beginning.')
                with XmlWrite.Element(xS, 'p'):
                    xS.charactersWithBr(u'Break at end\n')
                with XmlWrite.Element(xS, 'p'):
                    xS.charactersWithBr(u'\nBreak at beginning\nmiddle and end\n')
        # print()
        # print(xS.getvalue())
        # self.maxDiff = None
        self.assertEqual(xS.getvalue(), """<?xml version='1.0' encoding="utf-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html lang="en" xml:lang="en" xmlns="http://www.w3.org/1999/xhtml">
  <head />
  <body>
    <p>No break in this line.</p>
    <p>Several<br />breaks in<br />this line.</p>
    <p><br />Break at beginning.</p>
    <p>Break at end<br /></p>
    <p><br />Break at beginning<br />middle and end<br /></p>
  </body>
</html>
""")

# ---------- Benchmarks -----------------
def _encode_text(text):
    XmlWrite.encodeString(text)


# Length 445 bytes
BENCHMARK_TEXT = """Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.
Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.
Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."""

BENCHMARK_ATTRIBUTES = {
    'id' : 'ZaHR0cDovL3d3dy53My5vcmcvVFIvMTk5OS9SRUMtaHRtbDQwMS0xOTk5MTIyNC90eXBlcy5odG1sI3R5cGUtY2RhdGE_',
    'foo' : 'bar',
    'baz' : 'long_attribute_that_goes_on_and_on_and_on_and_on_and_on_and_on_and_on',
    'name' : 'George "Shotgun" Ziegler'
}

def test_XmlWrite_encode_text(benchmark):
    benchmark(_encode_text, BENCHMARK_TEXT)

def _decode_text(text):
    XmlWrite.decodeString(text)

def test_XmlWrite_decode_text(benchmark):
    encoded = XmlWrite.encodeString(BENCHMARK_TEXT)
    benchmark(_encode_text, encoded)

def create_XML_stream():
    with XmlWrite.XmlStream() as xS:
        pass

def test_XmlWrite_create_stream(benchmark):
    benchmark(create_XML_stream)

def create_XML_stream_write_two_elements():
    with XmlWrite.XmlStream() as xS:
        with XmlWrite.Element(xS, 'Root', {'version' : '12.0'}):
            with XmlWrite.Element(xS, 'A', {'attr_1' : '1'}):
                pass

def test_XmlWrite_two_elements(benchmark):
    benchmark(create_XML_stream_write_two_elements)

def write_small_XHTML_document(attributes):
    # Number of elements: 4*4*4*2 = 128
    headings = 4
    with XmlWrite.XhtmlStream() as xS:
        for i in range(headings):
            with XmlWrite.Element(xS, 'h1', attributes):
                for j in range(headings):
                    with XmlWrite.Element(xS, 'h2', attributes):
                        for k in range(headings):
                            with XmlWrite.Element(xS, 'h3', attributes):
                                for l in range(2):
                                    with XmlWrite.Element(xS, 'p', attributes):
                                        xS.characters(BENCHMARK_TEXT)
    result = xS.getvalue()
    return result

def test_XmlWrite_small_XHTML_doc(benchmark):
    # About 60kb
    result = benchmark(write_small_XHTML_document, {})
    # print()
    # print(result)
    assert len(result) == 61069

def write_large_XHTML_document(attributes):
    # Number of elements: 8*8*8*5 = 2560
    headings = 8
    with XmlWrite.XhtmlStream() as xS:
        for i in range(headings):
            with XmlWrite.Element(xS, 'h1', attributes):
                for j in range(headings):
                    with XmlWrite.Element(xS, 'h2', attributes):
                        for k in range(headings):
                            with XmlWrite.Element(xS, 'h3', attributes):
                                for l in range(5):
                                    with XmlWrite.Element(xS, 'p', attributes):
                                        xS.characters(BENCHMARK_TEXT)
    result = xS.getvalue()
    return result

def test_XmlWrite_large_XHTML_doc(benchmark):
    # About 1Mb
    result = benchmark(write_large_XHTML_document, {})
    # print()
    # print(result)
    assert len(result) == 1193497

def write_very_large_XHTML_document(attributes):
    # Number of elements: 16*16*16*8 = 32768
    headings = 16
    with XmlWrite.XhtmlStream() as xS:
        for i in range(headings):
            with XmlWrite.Element(xS, 'h1', attributes):
                for j in range(headings):
                    with XmlWrite.Element(xS, 'h2', attributes):
                        for k in range(headings):
                            with XmlWrite.Element(xS, 'h3', attributes):
                                for l in range(8):
                                    with XmlWrite.Element(xS, 'p', attributes):
                                        xS.characters(BENCHMARK_TEXT)
    result = xS.getvalue()
    return result

def test_XmlWrite_very_large_XHTML_doc(benchmark):
    # About 15Mb
    result = benchmark(write_very_large_XHTML_document, {})
#     print()
#     print(len(result))
    assert len(result) == 15205585

def test_XmlWrite_small_XHTML_doc_attrs(benchmark):
    # About 100kb
    result = benchmark(write_small_XHTML_document, BENCHMARK_ATTRIBUTES)
#     print()
#     print(len(result))
    assert len(result) == 109193

def test_XmlWrite_large_XHTML_doc_attrs(benchmark):
    # About 2Mb
    result = benchmark(write_large_XHTML_document, BENCHMARK_ATTRIBUTES)
#     print()
#     print(len(result))
    assert len(result) == 1907185

def test_XmlWrite_very_large_XHTML_doc_attrs(benchmark):
    # About 23Mb
    result = benchmark(write_very_large_XHTML_document, BENCHMARK_ATTRIBUTES)
    assert len(result) == 23635457

# ---------- END: Benchmarks -----------------

class NullClass(unittest.TestCase):
    pass

def unitTest(theVerbosity=2):
    suite = unittest.TestLoader().loadTestsFromTestCase(NullClass)
    suite.addTests(unittest.TestLoader().loadTestsFromTestCase(TestXmlWrite))
    suite.addTests(unittest.TestLoader().loadTestsFromTestCase(TestXhtmlWrite))
    myResult = unittest.TextTestRunner(verbosity=theVerbosity).run(suite)
    return (myResult.testsRun, len(myResult.errors), len(myResult.failures))
##################
# End: Unit tests.
##################

def usage():
    """Send the help to stdout."""
    print("""TestXmlWrite.py - A module that tests StrTree module.
Usage:
python TestXmlWrite.py [-lh --help]

Options:
-h, --help  Help (this screen) and exit

Options (debug):
-l:         Set the logging level higher is quieter.
             Default is 20 (INFO) e.g.:
                CRITICAL    50
                ERROR       40
                WARNING     30
                INFO        20
                DEBUG       10
                NOTSET      0
""")

def main():
    """Invoke unit test code."""
    print('TestXmlWrite.py script version "%s", dated %s' % (__version__, __date__))
    print('Author: %s' % __author__)
    print(__rights__)
    print()
    import getopt
    try:
        opts, args = getopt.getopt(sys.argv[1:], "hl:", ["help",])
    except getopt.GetoptError:
        usage()
        print('ERROR: Invalid options!')
        sys.exit(1)
    logLevel = logging.INFO
    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit(2)
        elif o == '-l':
            logLevel = int(a)
    if len(args) != 0:
        usage()
        print('ERROR: Wrong number of arguments!')
        sys.exit(1)
    # Initialise logging etc.
    logging.basicConfig(level=logLevel,
                    format='%(asctime)s %(levelname)-8s %(message)s',
                    #datefmt='%y-%m-%d % %H:%M:%S',
                    stream=sys.stdout)
    clkStart = time.clock()
    unitTest()
    clkExec = time.clock() - clkStart
    print('CPU time = %8.3f (S)' % clkExec)
    print('Bye, bye!')

if __name__ == "__main__":
    main()
