#include <iostream>
#include <memory>
#include <stdexcept>

#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLException.hpp>
#include <xercesc/util/XMLUni.hpp>

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
  XMLPlatformUtils::Initialize();
  try {
    unique_ptr<SAX2XMLReader> parser{XMLReaderFactory::createXMLReader()};
    parser->setFeature(XMLUni::fgSAX2CoreValidation, false);
    parser->setFeature(XMLUni::fgSAX2CoreNameSpaces, false);

    xml_handler handler;
    parser->setContentHandler(&handler);
    parser->setErrorHandler(&handler);
    parser->setLexicalHandler(&handler);

    parser->parse(argv[1]);
    const unique_ptr<const xml_node> root_node{handler.root()};
    const xml_node rw_pom{rewrite_pom(root_node.get())};
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
  XMLPlatformUtils::Terminate();
}
