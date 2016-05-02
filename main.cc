#include <iostream>

#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/framework/StdInInputSource.hpp>

#include "pom_xml_handler.h"

namespace {
using namespace std;
using namespace xercesc;
using namespace pommade;
}

int
main(int argc, const char* argv[]) {
  try {
    XMLPlatformUtils::Initialize();

    SAX2XMLReader* parser = XMLReaderFactory::createXMLReader();
    parser->setFeature(XMLUni::fgSAX2CoreValidation, false);
    parser->setFeature(XMLUni::fgSAX2CoreNameSpaces, false);

    pom_xml_handler handler;
    parser->setContentHandler(&handler);
    parser->setErrorHandler(&handler);
    parser->setLexicalHandler(&handler);

    if (argc >= 1)
      parser->parse(argv[1]);
    else {
      StdInInputSource siis;
      parser->parse(siis);
    }
    delete parser;
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
