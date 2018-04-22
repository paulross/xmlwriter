#!/usr/bin/env python
"""Tests cXmlWrite."""
import os

import pytest

import cXmlWrite as XmlWrite


with open(os.path.join(os.path.dirname(__file__), '_test_XmlWrite.py')) as f:
    code = compile(f.read(), __file__, 'exec')
#     exec(code)

if __name__ == "__main__":
    pytest.main()
