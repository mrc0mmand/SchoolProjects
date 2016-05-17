#!/bin/python3

#XQR:xsumsa01
import xml.etree.ElementTree as ET
import argparse
import sys
import re

def cast(v, to_type, default=None):
    """ 
    Type cast variable to specified type 

    If given variable cannot be cast to specified type, default value
    is returned.
    """
    try:
        return to_type(v)
    except ValueError as e:
        return default

def list_remove(l, item):
    """ Remove all occurencies of item in list l """
    l[:] = list(filter((item).__ne__, l))

class InputError(Exception):
    """ Input error """
    pass

class OutputError(Exception):
    """ Output error """
    pass

class QueryParserError(Exception):
    """ QueryParser error """
    pass

class XMLParserError(Exception):
    """ XMLParser error """
    pass

class QueryParser(object):
    """ SELECT query paser class """

    def __init__(self, query, query_file):
        """ Initialize query (from string or from file) """
        if query is None:
            if query_file is None:
                raise QueryParserError("both query and query_file "
                                       "are unspecified")
            else:
                try:
                    with open(query_file, 'r') as qf:
                        self._query = qf.read().replace('\n', ' ').strip()
                except Exception as e:
                    raise InputError(e)
        else:
            self._query = query.strip()

        self.qargs = dict()

    def process(self):
        """ Parse important query parts into qargs dictionary """
        # This should be definitely split into multiple regexes...
        r = re.compile("^SELECT\s+(?P<select>[^ ]+?)"
                       "(\s+LIMIT\s+(?P<limit>[0-9]+?))?"
                       "\s+FROM\s*(?P<from>((?P<elem>[^ .]+?)?"
                                             "(\.(?P<att>[^ ]+?))?))?"
                       "(\s+WHERE\s+(?P<not>NOT\s+)?"
                            "(?P<where>((?P<welem>[^ .]+?)?"
                                       "(\.(?P<watt>[^ ]+?))?))\s+"
                            "(?P<wop>(=|<|>|CONTAINS))\s+"
                            "(?P<wliteral>(\"[^\"]*\"|\d+(\.\d+)?)))?"
                       "(\s+ORDER BY\s+(?P<orderby>((?P<oelem>[^ .]+?)?"
                                            "(\.(?P<oatt>[^ ]+?))?)(?<!\s))"
                       "\s+(?P<order>ASC|DESC))?$")
        m = re.match(r, self._query)

        if m is None:
            raise QueryParserError("invalid query: {}".format(self))

        self.qargs = m.groupdict()

        # Convert literal to string or float
        if self.qargs["wliteral"] is not None:
            if not self.qargs["wliteral"].startswith("\""):
                try:
                    self.qargs["wliteral"] = float(self.qargs["wliteral"])
                except ValueError:
                    raise QueryParserError("Invalid literal {}"
                                            .format(self.qargs["wliteral"]))
            else:
                self.qargs["wliteral"] = self.qargs["wliteral"][1:-1]

        # CONTAINS operator can be used only on strings
        if self.qargs["wop"] == "CONTAINS" and \
           type(self.qargs["wliteral"]) is not str:
                raise QueryParserError("Invalid operator '{}' for type '{}'"
                                .format(self.qargs["wop"], 
                                        type(self.qargs["wliteral"]).__name__))

        return self.qargs

    def __str__(self):
        """ Query string version for printing """
        return self._query

class XMLParser(object):

    def __init__(self, xmlfile, outroot="virtual"):
        """ Load XML document from file or standard input """
        self._out = None
        self._outroot = outroot
        # Workaround for assignment specialities
        self._header = "<?xml version='1.0' encoding='UTF-8'?>"

        try:
            self._tree = ET.parse(xmlfile)
        except (OSError, IOError) as e:
            raise InputError(e)
        except Exception as e:
            raise XMLParserError(e)

    def compare(self, op, val1, val2):
        """ Compare two values with specified operator """
        if op == "CONTAINS":
            return val2 in val1
        elif op == "<":
            return val1 < val2
        elif op == ">":
            return val1 > val2
        elif op == "=":
            return val1 == val2
        else:
            return False

    def query(self, query):
        """ Apply query from QueryParser on given XML document """
        if not query or query["from"] in (None, ''):
            self._generate_output([])
            return

        if query["elem"] is None or query["elem"] == "ROOT":
            query["elem"] = "*"
        if query["welem"] is None and query["watt"] is not None:
            query["welem"] = "*"

        # FROM clause
        elem = None
        for e in self._tree.iter(query["elem"]):
            if query["att"] is not None:
                att = e.get(query["att"])
                if att is not None:
                    elem = e
                    break
            else:
                elem = e
                break

        result = []
        valid = True

        # SELECT clause
        if elem is not None:
            for e in elem.iter(query["select"]):
                # WHERE clause
                if query['where'] is not None:
                    valid = False
                    for sub in e.iter(query["welem"]):
                        check = None
                        if query["watt"] is not None:
                            att = sub.get(query["watt"])
                            if att is not None:
                                check = att
                            else:
                                continue
                        else:
                            if not list(sub):
                                check = sub.text
                            else:
                                raise XMLParserError("element {} contains"
                                        "subelements".format(query["welem"]))

                        check = cast(check, type(query["wliteral"]))

                        # TypeError => WHERE clause is False
                        if check is None:
                            break

                        valid = self.compare(query["wop"], check,
                                             query["wliteral"])
                        # NOT clause
                        if query["not"] is not None:
                            valid = not valid

                if valid:
                    result.append(e)

        # TODO (extension ORD)
        # ORDER BY clause
        #if query["orderby"] is not None:
        #    for item in result:
        #        pass

        # LIMIT clause
        limit = int(query["limit"]) if query["limit"] else None
        if limit is not None:
            result = result[0:limit]

        self._generate_output(result)

    def _generate_output(self, result):
        # Generate final XML tree
        if not result and self._outroot is None:
            return

        self._out = ET.Element(self._outroot if self._outroot else "virt")
        self._out.extend(result)

    def output(self, outfile, header):
        """ Write processed XML document into specified output file """
        out = ""

        if header:
            out += self._header

        if self._out is not None:
            xml = ET.tostring(self._out, 'unicode')
            # Remove root element
            if self._outroot is None:
                xml = re.sub("^<[^>]+>(.+)<[^>]+>$", "\g<1>", xml, 0, re.DOTALL)

            out += xml

        try:
            with open(outfile, 'w') as fh:
                fh.write(out)
        except TypeError:
            sys.stdout.write(out)
        except (IOError, OSError):
            raise OutputError(e)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    group = parser.add_mutually_exclusive_group(required=True)

    group.add_argument("--query",
            action="store",
            help="Input query")
    group.add_argument("--qf",
            action="store",
            metavar="FILE",
            help="Input query file")

    parser.add_argument("--input",
            action="store",
            metavar="FILE",
            default=sys.stdin,
            help="Input XML file (default: STDIN)")
    parser.add_argument("--output",
            action="store",
            metavar="FILE",
            default=sys.stdout,
            help="Output XML file(default: STDOUT)")
    parser.add_argument("-n",
            action="store_true",
            dest="header",
            help="Don't generate XML header in output file")
    parser.add_argument("--root",
            action="store",
            metavar="ELEMENT",
            help="Root element wrapper name")

    # Override default exit code (2) due to assignment specification
    try:
        args = parser.parse_args();
    except SystemExit as e:
        if e.code == 2:
            sys.exit(1)
        # Nasty workaround for another assignment speciality
        # -h and --help options can't be combined with another options
        if "-h" in sys.argv or "--help" in sys.argv:
            # Ignore ValueError
            try:
                list_remove(sys.argv, "-h")
                list_remove(sys.argv, "--help")
            except ValueError:
                pass
            # First list item is a script name
            if len(sys.argv) > 1:
                print("-h and --help can't be combined with other parameters",
                        file=sys.stderr)
                sys.exit(1)

        sys.exit(e.code)

    try:
        # Parse query
        qp = QueryParser(args.query, args.qf)
        qp.process()
        # Process XML
        xmlp = XMLParser(args.input, args.root)
        xmlp.query(qp.qargs)
        xmlp.output(args.output, not args.header)
    except InputError as e:
        # Input error (EC: 2)
        print(e, file=sys.stderr)
        sys.exit(2)
    except OutputError as e:
        # Output error (EC: 3)
        print(e, file=sys.stderr)
        sys.exit(3)
    except QueryParserError as e:
        # Query errors (EC: 80)
        print("QUERY ERROR: {}".format(e), file=sys.stderr)
        sys.exit(80)
    except XMLParserError as e:
        # XML errors (EC: 4)
        print("XML ERROR: {}".format(e), file=sys.stderr)
        sys.exit(4)
    except Exception as e:
        # Other errors (EC: 100)
        print("GENERAL ERROR ({}): {}".format(type(e).__name__ , e),
                                              file=sys.stderr)
        sys.exit(100)
