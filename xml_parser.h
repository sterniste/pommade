#ifndef XML_PARSER_H
#define XML_PARSER_H

#include <memory>
#include <stack>
#include <string>
#include <utility>

#include <xercesc/sax/SAXParseException.hpp>
namespace xercesc_3_1 {
class Attributes;
class Locator;
}

#include "xml_graph.h"

namespace xml_parser {

struct xmlstring : public std::string {
  xmlstring(const XMLCh* buf);
  xmlstring(const XMLCh* buf, XMLSize_t len);
  xmlstring(const std::string& that) : std::string{that} {}
};

struct xml_doc_handler {
  virtual ~xml_doc_handler() {}

  virtual void handle_content(const xercesc::Locator& locator, const XMLCh* const buf, const XMLSize_t len) = 0;
  virtual void handle_end_document(const xercesc::Locator& locator) = 0;
  virtual void handle_start_element(const xercesc::Locator& locator, const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const xercesc_3_1::Attributes& attrs) = 0;
  virtual void handle_end_element(const xercesc::Locator& locator, const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname) = 0;
  virtual void handle_comment(const xercesc::Locator& locator, const XMLCh* const buf, const XMLSize_t cnt) = 0;
  virtual void handle_processing_instruction(const xercesc::Locator& locator, const XMLCh* const target, const XMLCh* const data) = 0;
  virtual void handle_warning(const xercesc::SAXParseException& e) = 0;
  virtual void handle_error(const xercesc::SAXParseException& e) = 0;
  virtual void handle_fatal_error(const xercesc::SAXParseException& e) = 0;
    
  virtual std::unique_ptr<const xml_graph::xml_node> doc() = 0;
};

class basic_xml_doc_handler : public xml_doc_handler {
  friend class xml_doc_parser;
  
  std::string node_path;
  std::unique_ptr<const std::string> node_comment;
  std::stack<xml_graph::xml_node*> nodep_stack;
  std::unique_ptr<xml_graph::xml_node> root_node;

  static int ignorable_newlines(const std::string& content);
  
  void handle_content(const xercesc::Locator& locator, const XMLCh* const buf, const XMLSize_t len) override;
  void handle_end_document(const xercesc::Locator& locator) override;
  void handle_start_element(const xercesc::Locator& locator, const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const xercesc_3_1::Attributes& attrs) override;
  void handle_end_element(const xercesc::Locator& locator, const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname) override;
  void handle_comment(const xercesc::Locator& locator, const XMLCh* const buf, const XMLSize_t cnt) override;
  void handle_processing_instruction(const xercesc::Locator& locator, const XMLCh* const target, const XMLCh* const data) override;
  void handle_warning(const xercesc::SAXParseException& e) override;
  void handle_error(const xercesc::SAXParseException& e) override;
  void handle_fatal_error(const xercesc::SAXParseException& e) override;

  std::unique_ptr<const xml_graph::xml_node> doc() override { return std::move(root_node); }
};

class xml_doc_parser {
  xml_doc_handler& doc_handler;
  
 public:
  ~xml_doc_parser();
  xml_doc_parser(xml_doc_handler& doc_handler);

  std::unique_ptr<const xml_graph::xml_node> parse_doc(const char* file);
};
}

#endif
