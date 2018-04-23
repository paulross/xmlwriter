'''
Created on 23 Apr 2018

@author: paulross

Example of extracted data:

{'tests/unit/test_XmlWrite.py::test_XmlWrite_create_stream': 3.0645023798570037e-06,
 'tests/unit/test_XmlWrite.py::test_XmlWrite_decode_text': 4.685003659687936e-06,
 'tests/unit/test_XmlWrite.py::test_XmlWrite_encode_text': 4.531990271061659e-06,
 'tests/unit/test_XmlWrite.py::test_XmlWrite_large_XHTML_doc': 0.03668952500447631,
 'tests/unit/test_XmlWrite.py::test_XmlWrite_large_XHTML_doc_attrs': 0.0841250344819855,
 'tests/unit/test_XmlWrite.py::test_XmlWrite_small_XHTML_doc': 0.002258329011965543,
 'tests/unit/test_XmlWrite.py::test_XmlWrite_small_XHTML_doc_attrs': 0.005926209007157013,
 'tests/unit/test_XmlWrite.py::test_XmlWrite_two_elements': 2.0231003873050213e-05,
 'tests/unit/test_XmlWrite.py::test_XmlWrite_very_large_XHTML_doc': 0.49019894999219105,
 'tests/unit/test_XmlWrite.py::test_XmlWrite_very_large_XHTML_doc_attrs': 1.110087487992132,
 'tests/unit/test_cXmlWrite.py::test_XmlWrite_create_stream': 2.0463019609451292e-06,
 'tests/unit/test_cXmlWrite.py::test_XmlWrite_decode_text': 8.822011295706034e-06,
 'tests/unit/test_cXmlWrite.py::test_XmlWrite_encode_text': 7.021008059382439e-06,
 'tests/unit/test_cXmlWrite.py::test_XmlWrite_large_XHTML_doc': 0.012037309003062546,
 'tests/unit/test_cXmlWrite.py::test_XmlWrite_large_XHTML_doc_attrs': 0.029281631985213608,
 'tests/unit/test_cXmlWrite.py::test_XmlWrite_small_XHTML_doc': 0.0006714795017614961,
 'tests/unit/test_cXmlWrite.py::test_XmlWrite_small_XHTML_doc_attrs': 0.001663103001192212,
 'tests/unit/test_cXmlWrite.py::test_XmlWrite_two_elements': 5.9019948821514845e-06,
 'tests/unit/test_cXmlWrite.py::test_XmlWrite_very_large_XHTML_doc': 0.183633100008592,
 'tests/unit/test_cXmlWrite.py::test_XmlWrite_very_large_XHTML_doc_attrs': 0.3463930379948579,
 'tests/unit/test_pbXmlWrite.py::test_XmlWrite_create_stream': 4.818502929992974e-06,
 'tests/unit/test_pbXmlWrite.py::test_XmlWrite_decode_text': 9.212992154061794e-06,
 'tests/unit/test_pbXmlWrite.py::test_XmlWrite_encode_text': 7.399008609354496e-06,
 'tests/unit/test_pbXmlWrite.py::test_XmlWrite_large_XHTML_doc': 0.022217548001208343,
 'tests/unit/test_pbXmlWrite.py::test_XmlWrite_large_XHTML_doc_attrs': 0.04281273747619707,
 'tests/unit/test_pbXmlWrite.py::test_XmlWrite_small_XHTML_doc': 0.0013533039891626686,
 'tests/unit/test_pbXmlWrite.py::test_XmlWrite_small_XHTML_doc_attrs': 0.002492767496732995,
 'tests/unit/test_pbXmlWrite.py::test_XmlWrite_two_elements': 1.4860008377581835e-05,
 'tests/unit/test_pbXmlWrite.py::test_XmlWrite_very_large_XHTML_doc': 0.3029421890096273,
 'tests/unit/test_pbXmlWrite.py::test_XmlWrite_very_large_XHTML_doc_attrs': 0.5157630419998895}

C++ results:
pauls-mbp-4:Products paulross$ ./Release/xmlwriter
Hello world
                 test_XmlWrite__encode_no_encoding time:        1.463 (us)
               test_XmlWrite__encode_with_encoding time:        3.357 (us)
                        test_XmlWrite_encodeString time:        8.090 (us)
                        test_XmlWrite_decodeString time:       18.882 (us)
                   test_write_small_XHTML_document time:      436.172 (us) size:        61069 result: 1
                   test_write_large_XHTML_document time:     9283.854 (us) size:      1193497 result: 1
              test_write_very_large_XHTML_document time:   140583.742 (us) size:     15205585 result: 1
        test_write_small_XHTML_document_attributes time:     1392.958 (us) size:       109193 result: 1
        test_write_large_XHTML_document_attributes time:    23029.205 (us) size:      1907185 result: 1
   test_write_very_large_XHTML_document_attributes time:   264157.906 (us) size:     23635457 result: 1
Bye, bye!



Rearrange as:
# Size  C++             C++,attrs       Python          Python,attrs    Pybind          Pybind,attrs    CPython     CPython, attrs
#"Small"    731.198e-6    1412.637e-6
#"Medium"    10412.371e-6    20756.986
#"Large"    149181.365e-6    277061.839

128     731.198e-6      1412.637e-6     2231.5890e-6    5237.0640e-6    1290.9170e-6    2365.3230e-6    606.7420e-6     1556.0640e-6
2560    10412.371e-6    20756.986e-6    36113.6600e-6   82145.0460e-6   21161.3930e-6   38042.0760e-6   10763.2500e-6   25751.0330e-6
32768   149181.365e-6   277061.839e-6   475908.8950e-6  1045429.1920e-6 281937.6000e-6  458238.8430e-6  148290.4980e-6  326430.3360e-6
'''
import json
import re
import sys

#: Match
#: 'tests/unit/test_pbXmlWrite.py::test_XmlWrite_small_XHTML_doc'
RE_LONG_NAME_MATCH = re.compile(r'tests/unit/test_(\S+?)\.py::(.+)')

CPP_RESULTS = {
    128 : [436.172e-6, 1392.958e-6],
    2560 : [9283.854e-6, 23029.205e-6],
    32768 : [140583.742e-6, 264157.906e-6]
}


def read_json(in_path):
    """Given a filepath this returns the JSON data."""
    with open(in_path) as f:
        return json.load(f)


def extract(data, key):
    """Return a dict of {long_name : value, ...} for the given stats key."""
    return {bm['fullname']:bm['stats'][key] for bm in data['benchmarks']}


def test_size(m):
    assert m is not None
    size = 0
    if m.group(2).startswith('test_XmlWrite_small'):
        size = 128
    elif m.group(2).startswith('test_XmlWrite_large'):
        size = 2560
    elif m.group(2).startswith('test_XmlWrite_very_large'):
        size = 32768
    return size


def transpose(data_dict):
    # 'tests/unit/test_pbXmlWrite.py::test_XmlWrite_small_XHTML_doc'
    column_order = [
        'XmlWrite', 'pbXmlWrite', 'cXmlWrite',
    ]
    # {size : [values, ...], ...}
    table_dict = {}
    for k, v in data_dict.items():
        m = RE_LONG_NAME_MATCH.match(k)
        if m is not None:
            size = test_size(m)
            if size:
                if size not in table_dict:
                    table_dict[size] = ['N/A'] * (2 * len(column_order))
                name = m.group(1)
                idx = 2 * column_order.index(name)
                if m.group(2).endswith('_attrs'):
                    idx += 1
                table_dict[size][idx] = '{:18.9f}'.format(v)
    print('{:10s}'.format('# Size'), end='')
    for col in ['C++'] + column_order:
        print('{:>18s}'.format(col), end='')
        print('{:>18s}'.format(col+',attrs'), end='')
    print()
    for k in sorted(table_dict.keys()):
        print('{:<10d}'.format(k), end='')
        print(''.join(['{:18.9f}'.format(v) for v in CPP_RESULTS[k]]), end='')
        print(''.join(table_dict[k]))


def main(path):
    j = read_json(path)
    data = extract(j, 'min')
    transpose(data)
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1]))