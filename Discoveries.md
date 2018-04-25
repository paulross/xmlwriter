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

<a name="The_pybind11_Project"></a>
## The ``pybind11`` Project

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

<a name="Miscellaneous"></a>
## Miscellaneous

### Debug Builds

Pybind11 uses `#if !defined(NDEBUG)` for debug code so un-defining `NDEBUG` can be useful for creating a debug build and getting better error messages.

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
