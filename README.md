xmlwriter
==============

Python XML/HTML/SVG writer implemented in C++

An XML/HTML/SVG writer project built with [pybind11](https://github.com/pybind/pybind11).

Project is based on the [pybind11 example](https://github.com/pybind/python_example).

Installation
------------

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


Windows runtime requirements
----------------------------

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


Building the documentation
--------------------------

Documentation for the example project is generated using Sphinx. Sphinx has the
ability to automatically inspect the signatures and documentation strings in
the extension module to generate beautiful documentation in a variety formats.
The following command generates HTML-based reference documentation; for other
formats please refer to the Sphinx manual:

 - `cd xmlwriter/docs`
 - `make html`

License
-------

pybind11 is provided under a BSD-style license that can be found in the LICENSE
file. By using, distributing, or contributing to this project, you agree to the
terms and conditions of this license.

Test call
---------

```python
import cXmlWrite
```

Discoveries
===========

The original `XmlWrite.XmlStream` took as the first argument a string or file like object. If the former it was treated as the file path to write to. I havn't been able to reproduce this pattern by using a `PyObject*` and deciding within the constructor what to do:

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

Rewrote both Python and C++ code to write to an internal buffer, the caller can retrieve this and write it to file.





