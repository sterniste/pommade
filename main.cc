#include <iostream>
#include <memory>

#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLException.hpp>
#include <xercesc/util/XMLUni.hpp>

#include "xml_handler.h"
#include "xml_node.h"

namespace {
using namespace std;
using namespace pommade;
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
    unique_ptr<const xml_node> root_node{handler.root()};
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
