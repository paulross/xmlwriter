# Table of contents
1. [Introduction](#Introduction)
2. [Discoveries](#Discoveries)
    1. [Build Time](#Build_Time)
    2. [Passing Python File Objects into C++](#Passing_Python_File_Objects_into_Cpp)
3. [Results](#Results)
    1. [The ``pybind11`` Project](#The_pybind11_Project)
    2. [Development Time](#Development_Time)
    3. [Build System](#Build_System)
    4. [Code Maintainability](#Code_Maintainability)
    5. [Performance](#Performance)
        1. [Selected Benchmarks](#Performance_Selected_Benchmarks)
        2. [Optimisation](#Performance_Optimisation)
        3. [Summary](#Performance_Summary)
    6. [Documentation](#Documentation)
4. [Conclusions](#Conclusions)
5. [History](#History)
6. [Boilerplate Footnotes](#Boilerplate_Footnotes)

<a name="Introduction"></a>
# Introduction

A Python XML/HTML/SVG writer originally implemented in Python and now migrated to C++ with [pybind11](https://github.com/pybind/pybind11).

The aim of this project was:

* To get familiar with the nuts and bolts of a pybind11 project.
* See how long the migration would take.
* Get a feel for the build system.
* Have a look at what the final code looked like and how maintainable it would be.
* Measure the relative performance of the Python and pybind11 code.
* Have a look at the auto-generated documentation by pybind11.
* To create a faster XML/XHTML/SVG reader, the pure Python implementation was presumed to be slow in other projects (``cpip``, ``TotalDepth`` etc.).

This project is based on the [pybind11 example](https://github.com/pybind/python_example).

<a name="Discoveries"></a>
# Discoveries

<a name="Build_Time"></a>
## Build Time

The absolute minimal example takes a significant time to build compared to a minimal CPython extension, seven seconds compared to sub-second build time.
Xcode is much faster, possibly multiprocessing is at work here.

<a name="Passing_Python_File_Objects_into_Cpp"></a>
## Passing Python File Objects into C++

The original `XmlWrite.XmlStream` took as the first argument a string or file like object.
If the former it was treated as the file path to write to.
I haven't been able to reproduce this pattern by using a `PyObject*` and deciding within the constructor what to do:

```c
    py::class_<XmlStream>(m, "XmlStream")
        .def(py::init<PyObject*, const std::string &, const std::string &, int, bool>());
```

This complains with:

```python
>>> s= "test.xml"
>>> xs = cXmlWrite.XmlStream(s, "utf-8", "", 0, True)
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
TypeError: __init__(): incompatible constructor arguments. The following argument types are supported:
    1. cXmlWrite.XmlStream(arg0: _object, arg1: str, arg2: str, arg3: int, arg4: bool)

Invoked with: 'test.xml', 'utf-8', '', 0, True
```

I can not find a way to map Python's file object to an internal C++ stream: http://pybind11.readthedocs.io/en/stable/advanced/pycpp/object.html

So that I could move on to the other aspects of the project I rewrote both Python and C++ code to write to an internal buffer.
The caller can retrieve this and write it to file.

---

NOTE: Only ``XmlWrite.py`` was converted to C++ as this was deemed sufficient to satisfy the project goals.
The other Python files (``SVGWriter.py``, ``Coord.py``) were not converted is it was not believe that they would add anything.

---

<a name="Results"></a>
# Results

## The ``pybind11`` Project <a name="The_pybind11_Project"></a>

This project is based on the [pybind11 example](https://github.com/pybind/python_example), it can also build under Xcode which is useful for error checking.

A degree of effort went into separating pure C++ code from CPython and pybind11 code.
The intention was that code in the ``cpp/`` directory was independent of CPython and pybind11.
In practice pybind11 headers were needed in the ``cpp/`` directory to provide ``*args`` for the ``__exit__`` methods. Perhaps there is a workaround for this.

<a name="Development_Time"></a>
## Development Time

About 2 to 3 days of work.
This should be reduced in future projects as there was a fair amount of fiddling around understanding pybind11 corner cases, such as having to use a lambda to convert a ``std::string`` to a bytes object.

This development time would be what I would expect for a 'C' extension, but see [code maintainability](Code_Maintainability) below.

<a name="Build_System"></a>
## Build System

I used the ``setup.py`` and this was entirely satisfactory.
It is worth creating the project to build under Xcode (or equivalent) as this is faster to build and find compile/link time bugs.

<a name="Code_Maintainability"></a>
## Code Maintainability

This is rather nice. ``XmlWrite.cpp`` is written in C++11 and reads almost like Python.
``XmlWrite.cpp`` is 284 lines of code compared to the original pure Python implementation of around 300 lines (if you ignore the documentation strings).
Of course you have the header file ``XmlWrite.h`` (145 lines) but this is pretty simple and generally does not change much over time.
There is some additional C++ code to do base64 encoding/decoding that C++ needs, Python does not need this of course as it is in the Python standard library.

Then there is the ``cXmlWrite.cpp`` file that is the pybind11 interface, this is about 120 lines (ignoring documentation strings).
Of course it is written in pybind11 style that takes a little getting used to but is perfectly readable.

One notable feature of pybind11 is that if you get it wrong the compilation messages can come from template meta-programming (or just template creation) and are thus pretty obscure and you can take a long time to track the problem down.
This glue code is definitely better than normal C Extension code or optimised Cython code.

<a name="Performance"></a>
## Performance

This is pretty disappointing. The pybind11 code is consistently faster at writing documents but not by much.
Typically the pybind11 module ``cXmlWrite`` takes about 70% to 80% of the time of the pure Python module ``XmlWrite``.
This performance is not worth moving away from the convenience of writing in pure Python.

For base64 encoding the pybind11 code is actually slower, taking more than twice as long.

I think that the reasons for this disappointment are these:

* The design of the XML writer required the construction and destruction of many small objects and perhaps pybind11 does not shine here. To be fair any C/C++ solution would, most likely, behave the same way.
* The Python standard library for base64 is written directly in C so one would not expect an improvement when going through pybind11's machinery.

It just goes to show how careful you must be in your choice before you set out to migrate to pybind11.

It might be worth trying pybind11 out on TotalDepth's LIS file indexer.
This spends more time in C/C++ land so should show an great improvement.
Also we have a C reference implementation that is 100x faster than the pure Python one so we could compare pybind11 with that.

<a name="Performance_Selected_Benchmarks"></a>
### Selected Benchmarks

Here are the median values of the benchmarks measured by ``pytest-benchmark`` for selected operations.
Values are the median execution time in microseconds rounded to 3 S.F.:

| Operation                             | Python implementation (us)    | C++ implementation (us)   | Ratio C++/Python  |
| ------------------------------------- | ----------------------------: | ------------------------: | ----------------: |
| Encode text                           | 4.25                          | 9.24                      | 2.18              |
| Decode text                           | 4.39                          | 11.80                     | 2.69              |
| Create stream                         | 2.83                          | 4.31                      | 1.52              |
| Write two elements                    | 17.9                          | 12.4                      | 0.695             |
| Small XHTML document (60 kb)          | 1,970                         | 1,510                     | 0.769             |
| Large XHTML document (1.14 Mb)        | 33,800                        | 26,500                    | 0.784             |
| Very large XHTML document (14.5 Mb)   | 441,000                       | 352,000                   | 0.798             |

<a name="Performance_Optimisation"></a>
### Optimisation

As Ewan Higgs pointed out having checked this code with ``callgrind`` it shows a very large number of calls to ``XmlStream::_encode()`` (which translates certain characters to entities).
This function was also implemented rather inefficiently with poor locality of reference and many small mallocs.

``XmlStream::_encode()`` was initially reimplemented to use a switch/case statement, this gave a 14% speedup.
A further optimisation was added to lazily evaluate the input string and only create a new string if there was any character that needed expanding to an entity, this gave an extra 30 % speed up.
Finally a perfomance test was done where ``XmlStream::_encode()`` was not used at all which establishes a baseline performance for the CPython implementation.

NOTE: Time in milliseconds for clarity.

| Implementation                | Write 14.5 Mb XHTML document (ms)     | Factor (Original pybind11 is x1)      |
| ----------------------------- | ------------------------------------: | ------------------------------------: |
| Python                        | 441                                   | 1.25                                  |
| Original pybind11 and C++     | 352                                   | 1.0                                   |
| Use a switch/case statememt   | 304                                   | 0.864                                 |
| Lazily evaluate entities      | 234                                   | 0.665                                 |
| No entity encoding at all     | 209                                   | 0.594                                 |

To get an understanding of the overhead I implemented some tests in [main.cpp](https://github.com/paulross/xmlwriter/blob/master/xmlwriter/cpp/main.cpp) that reproduces the ``write_..._XHTML_document()`` tests in [test_XmlWrite.py](https://github.com/paulross/xmlwriter/blob/master/tests/unit/test_cXmlWrite.py) but using the C++ code directly. These can be compared with the latest, optimised, pybind11 code.

| Implementation                | Write 14.5 Mb XHTML document (ms)     | Factor (Original pybind11 is x1)      |
| ----------------------------- | ------------------------------------: | ------------------------------------: |
| Current best pybind11/C++     | 234                                   | 1.0                                   |
| Pure C++                      | 119                                   | 0.509                                 |
| Pure C++, no entity encoding  | 103                                   | 0.440                                 |

So it looks like the encoding costs 16 ms and the cost of going through pybind11 is 115 ms.

<a name="Performance_Summary"></a>
### Summary

The current state of play is:

| Implementation                | Write 14.5 Mb XHTML document (ms)     | Factor (Python is x1)                             |
| ----------------------------- | ------------------------------------: | :------------------------------------------------ |
| Python                        | 441                                   | 1.0                                               |
| Current best pybind11/C++     | 234                                   | 0.531                                             |
| Pure C++                      | 119                                   | 0.270                                             |

Of course these figures are only reflective of *this particular* problem.
I still suspect that the many small objects problem is not allowing pybind11 to shine more brightly.

<a name="Documentation"></a>
## Documentation

I got this started with commit [b41afc0](https://github.com/paulross/xmlwriter/commit/b41afc0414a7157c1dbc42819181b1cb9b9b0fad).
It is a bit fiddly to set up and is not entirely complete yet, it does illustrate what is possible however.
~~Migrating the documentation across from Python strings to C strings is a bit tedious but perhaps this could be automated.
Possibly the C documentation strings should go in their own header file to reduce the clutter.~~

This is now done with the `pydoc2cppdoc.py` script:

`python pydoc2cppdoc.py <module code>` will extract Python documentation strings and write them out as C++ string literals. Try:

```
cd py/xmlwriter
python pydoc2cppdoc.py ExampleDocstrings.py
```

Capture the output of `python pydoc2cppdoc.py XmlWrite.py` to a header file such as `cpy/cXmlWrite_docs.h` and include that in the pybind11 code at `cpy/cXmlWrite.cpp`. Example documentation at https://paulross.github.io/xmlwriter/index.html

The nice thing is that type annotations are generated in the documentation automatically from the C++ code.

One noticeable thing was the slow turnaround when editing the documentation, you have to edit in C++, build the project (which takes several seconds) and then ``make html``.

<a name="Conclusions"></a>
# Conclusions

* [``pybind11``](https://github.com/pybind/pybind11) provides a quick and simple interface between Python and C++ code.
   This is particularly true when creating new types (classes).
   It is clearly a very competent project and I have only scratched the surface here.
* The relative performance of ``pybind11`` is a bit disappointing in this *particular* project.
* I suspect that ``pybind11`` would show much better performance on a project where the bulk of the time is spent in C++ and the Python/C++ boundary is infrequently crossed.
* Libraries that are in the Python standard library will have to be replaced with their C/C++ equivalents (as happened here with base64).
   The disadvantage with this is:
  * The C/C++ libraries might not be exactly equivalent and require workarounds.
  * This adds to the dependencies
  * There might be possible licencing conflicts.

   All of this increases the creation and maintenance cost.

* ``pybind11``'s ability to generate type specific documentation is very attractive for environments and IDEs that can make use of this.
* I'd certainly consider ``pybind11`` as a first class option when there is a large body of Python code to move into C++ or when a Python interface is needed to an existing C++ library.

<a name="History"></a>
# History (latest at top)

## 2018-02-26 - Python to C++ Documentation

Added automatic migration of python documentation strings with `pydoc2cppdoc.py`.

## 2017-12-05 11:45 - Made repository public

Made public around: Tue  5 Dec 2017 11:45:14 GMT

## 2017-12-05 11:44 - Last private commit

```
commit 517f5267709029fe9f651bf3e0b88655a40ae052
Author: Paul Ross <apaulross@gmail.com>
Date:   Tue Dec 5 11:44:17 2017 +0000
```
    Last private commit.


## 2017-11-27

```
commit f4267ff0eefe9a99c27a9b84ff22087e1ff29f1c
Author: paulross <apaulross@gmail.com>
Date:   Mon Nov 27 09:19:37 2017 +0000
```

    Initial commit.

<a name="Boilerplate_Footnotes"></a>
# Boilerplate Footnotes

## Installation

**On Unix (Linux, OS X)**

 - clone this repository
 - `pip install ./xmlwriter`

**On Windows (Requires Visual Studio 2015)**

 - For Python 3.5:
     - clone this repository
     - `pip install ./xmlwriter`
 - For earlier versions of Python, including Python 2.7:

   Pybind11 requires a C++11 compliant compiler (i.e. Visual Studio 2015 on
   Windows). Running a regular `pip install` command will detect the version
   of the compiler used to build Python and attempt to build the extension
   with it. We must force the use of Visual Studio 2015.

     - clone this repository
     - `"%VS140COMNTOOLS%\..\..\VC\vcvarsall.bat" x64`
     - `set DISTUTILS_USE_SDK=1`
     - `set MSSdk=1`
     - `pip install ./xmlwriter`

   Note that this requires the user building `xmlwriter` to have registry edition
   rights on the machine, to be able to run the `vcvarsall.bat` script.


## Windows runtime requirements

On Windows, the Visual C++ 2015 redistributable packages are a runtime
requirement for this project. It can be found [here](https://www.microsoft.com/en-us/download/details.aspx?id=48145).

If you use the Anaconda python distribution, you may require the Visual Studio
runtime as a platform-dependent runtime requirement for you package:

```yaml
requirements:
  build:
    - python
    - setuptools
    - pybind11

  run:
   - python
   - vs2015_runtime  # [win]
```


## Building the documentation

Documentation for the example project is generated using Sphinx. Sphinx has the
ability to automatically inspect the signatures and documentation strings in
the extension module to generate beautiful documentation in a variety formats.
The following command generates HTML-based reference documentation; for other
formats please refer to the Sphinx manual:

 - `cd xmlwriter/docs`
 - `make html`

## License

pybind11 is provided under a BSD-style license that can be found in the LICENSE
file. By using, distributing, or contributing to this project, you agree to the
terms and conditions of this license.

## Test call

```python
import cXmlWrite
```
