#ifndef POM_XML_HANDLER_H
#define POM_XML_HANDLER_H

#include <memory>
#include <stack>
#include <string>
#include <utility>

#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
namespace xercesc_3_1 {
class Attributes;
class Locator;
}

#include "xml_node.h"

namespace pommade {

struct xmlstring : public std::string {
  xmlstring(const XMLCh* buf);
  xmlstring(const XMLCh* buf, XMLSize_t len);
  xmlstring(const std::string& that) : std::string{that} {}
};

class xml_handler : public xercesc::DefaultHandler {
  std::string node_path;
  std::unique_ptr<const std::string> node_comment;
  std::stack<xml_node*> nodep_stack;
  std::unique_ptr<xml_node> root_node;
  const xercesc::Locator* locator;

  static int ignorable_newlines(const std::string& content);

  void characters(const XMLCh* const buf, const XMLSize_t len) override;

  void endDocument() override;
  void startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const xercesc_3_1::Attributes& attrs) override;
  void endElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname) override;

  void comment(const XMLCh* const buf, const XMLSize_t cnt) override;
  void processingInstruction(const XMLCh* const target, const XMLCh* const data) override;

  void warning(const xercesc::SAXParseException& e) override;
  void error(const xercesc::SAXParseException& e) override;
  void fatalError(const xercesc::SAXParseException& e) override;

  void setDocumentLocator(const xercesc::Locator* locator) override { this->locator = locator; }

 public:
  std::unique_ptr<const xml_node> root() { return std::move(root_node); }
};
}

#endif
