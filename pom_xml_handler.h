#ifndef POM_XML_HANDLER_H
#define POM_XML_HANDLER_H

#include <memory>
#include <stack>
#include <string>

#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
namespace xercesc_3_1 {
class Attributes;
}

#include "xml_tag.h"

namespace pommade {

struct xmlstring : public std::string {
  xmlstring(const XMLCh* buf) {
    char* cp{xercesc::XMLString::transcode(buf)};
    std::string::operator=(cp);
    xercesc::XMLString::release(&cp);
  }

  xmlstring(const XMLCh* buf, XMLSize_t len) {
    char* const cp = new char[3 * len + 1];
    xercesc::XMLString::transcode(buf, cp, 3 * len);
    std::string::operator=(cp);
    delete[] cp;
  }
};

class pom_xml_handler : public xercesc::DefaultHandler, private xercesc::XMLFormatTarget {
  static const XMLCh end_comment[];
  static const XMLCh end_element[];
  static const XMLCh end_pi[];
  static const XMLCh start_comment[];
  static const XMLCh start_pi[];
  static const XMLCh xml_decl1[];
  static const XMLCh xml_decl2[];

  xercesc::XMLFormatter formatter;
  std::string tag_path;
  std::unique_ptr<const std::string> tag_comment;
  std::stack<xml_tag*> tagp_stack;
  std::unique_ptr<xml_tag> root_tag;

  static int ignorable_newlines(const std::string& content);

 public:
  ~pom_xml_handler() {}
  pom_xml_handler();

  void writeChars(const XMLByte* const buf, const XMLSize_t len, xercesc::XMLFormatter* const formatter) override;
  void characters(const XMLCh* const buf, const XMLSize_t len) override;

  void endDocument() override;
  void startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const xercesc_3_1::Attributes& attrs) override;
  void endElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname) override;

  void comment(const XMLCh* const buf, const XMLSize_t cnt) override;
  void processingInstruction(const XMLCh* const target, const XMLCh* const data) override;

  void warning(const xercesc::SAXParseException& e) override;
  void error(const xercesc::SAXParseException& e) override;
  void fatalError(const xercesc::SAXParseException& e) override;
};
}

#endif
