#include <iostream>
#include <memory>

#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/util/XMLException.hpp>

#include "rewrite_pom.h"
#include "xml_graph.h"
#include "xml_parser.h"

namespace {
using namespace std;
using namespace pommade;
using namespace xml_graph;
using namespace xml_parser;
using namespace xercesc_3_1;
}

int
main(int argc, const char* argv[]) {
  try {
    basic_xml_doc_handler doc_handler;
    xml_doc_parser doc_parser{doc_handler};
    const unique_ptr<const xml_node> doc{doc_parser.parse_doc(argv[1])};
    const xml_node rw_pom{rewrite_pom(doc.get())};
    cout << rw_pom;
  } catch (const XMLException& e) {
    cout << "caught XMLException: " << xmlstring{e.getMessage()} << endl;
    return 1;
  } catch (const SAXParseException& e) {
    cout << "caught SAXParseException: " << xmlstring{e.getMessage()} << endl;
    return 1;
  } catch (...) {
    cout << "caught exception" << endl;
    return 1;
  }
}
