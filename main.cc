#include <iostream>

#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/util/XMLException.hpp>

#include "rewrite_pom.h"
#include "xml_parser.h"

namespace {
using namespace std;
using namespace pommade;
using namespace xml_parser;
using namespace xercesc_3_1;
}

int
main(int argc, const char* argv[]) {
  try {
    basic_xml_doc_handler doc_handler;
    cout << pom_rewriter{}.rewrite_pom(xml_doc_parser{doc_handler}.parse_doc(argv[1]).get());
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
