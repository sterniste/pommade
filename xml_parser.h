#ifndef XML_PARSER_H
#define XML_PARSER_H

#include <cassert>
#include <cctype>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <utility>

#include <xercesc/sax/Locator.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLUni.hpp>

namespace xercesc_3_1 {
class Attributes;
}
namespace xml_graph {
struct xml_node;
}

namespace xml_parser {

struct xmlstring : public std::string {
  xmlstring(const XMLCh* buf);
  xmlstring(const XMLCh* buf, XMLSize_t len);
  xmlstring(const std::string& that) : std::string{that} {}
};

template <typename Node> struct basic_xml_doc_handler {
  virtual ~basic_xml_doc_handler() {}

  virtual void handle_content(const xercesc::Locator& locator, const XMLCh* const buf, const XMLSize_t len) = 0;
  virtual void handle_end_document(const xercesc::Locator& locator) = 0;
  virtual void handle_start_element(const xercesc::Locator& locator, const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const xercesc_3_1::Attributes& attrs) = 0;
  virtual void handle_end_element(const xercesc::Locator& locator, const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname) = 0;
  virtual void handle_comment(const xercesc::Locator& locator, const XMLCh* const buf, const XMLSize_t cnt) = 0;
  virtual void handle_processing_instruction(const xercesc::Locator& locator, const XMLCh* const target, const XMLCh* const data) = 0;
  virtual void handle_warning(const xercesc::SAXParseException& e) = 0;
  virtual void handle_error(const xercesc::SAXParseException& e) = 0;
  virtual void handle_fatal_error(const xercesc::SAXParseException& e) = 0;

  virtual std::unique_ptr<const Node> doc() = 0;
};

using xml_doc_handler = basic_xml_doc_handler<xml_graph::xml_node>;

template <typename Node> class basic_default_xml_doc_handler : public basic_xml_doc_handler<Node> {
  std::string node_path;
  std::unique_ptr<const std::string> node_comment;
  std::stack<Node*> nodep_stack;
  std::unique_ptr<Node> root_node;

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

  std::unique_ptr<const Node> doc() override { return std::move(root_node); }
};

template <typename Node>
int
basic_default_xml_doc_handler<Node>::ignorable_newlines(const std::string& content) {
  int nl_cnt{};
  for (const auto c : content) {
    if (!isspace(c))
      return -1;
    if (c == '\n')
      ++nl_cnt;
  }
  return nl_cnt;
}

template <typename Node>
void
basic_default_xml_doc_handler<Node>::handle_content(const xercesc::Locator& locator, const XMLCh* const buf, const XMLSize_t len) {
  const xmlstring content{buf, len};
  const int nl_cnt{ignorable_newlines(content)};
  if (nl_cnt < 0) {
    assert(!node_path.empty());
    auto* const nodep = nodep_stack.top();
    assert(!nodep->tree());
    if (nodep->get_content())
      nodep->append_content(xmlstring{content});
    else {
      nodep->set_content(xmlstring{content});
      node_comment.reset();
    }
  }
}

template <typename Node>
void
basic_default_xml_doc_handler<Node>::handle_end_document(const xercesc::Locator& locator) {
  assert(node_path.empty() && nodep_stack.empty());
  if (node_comment) {
    std::cerr << "discarding comment before document end; line " << locator.getLineNumber() << std::endl;
    node_comment.reset();
  }
  assert(root_node);
}

template <typename Node>
void
basic_default_xml_doc_handler<Node>::handle_start_element(const xercesc::Locator& locator, const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const xercesc_3_1::Attributes& attrs) {
  Node* nodep{};
  if (!root_node) {
    assert(nodep_stack.empty());
    root_node.reset(new Node{static_cast<unsigned short>(locator.getLineNumber()), 0, xmlstring{qname}, node_comment.get()});
    nodep = root_node.get();
  } else {
    assert(!nodep_stack.empty() && !nodep_stack.top()->get_content());
    nodep = nodep_stack.top()->add_subnode(Node{static_cast<unsigned short>(locator.getLineNumber()), static_cast<unsigned short>(nodep_stack.top()->level + 1), xmlstring{qname}, node_comment.get()});
  }
  node_comment.reset();
  nodep_stack.push(nodep);

  node_path += '/' + xmlstring{qname};
}

template <typename Node>
void
basic_default_xml_doc_handler<Node>::handle_end_element(const xercesc::Locator& locator, const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname) {
  const std::string::size_type pos{node_path.rfind('/')};
  assert(pos != std::string::npos && xmlstring{qname} == node_path.substr(pos + 1));
  if (node_comment) {
    std::cerr << "discarding comment before '" + node_path + "' end; line " << locator.getLineNumber() << std::endl;
    node_comment.reset();
  }

  assert(!nodep_stack.empty());
  nodep_stack.pop();

  node_path = node_path.substr(0, pos);
}

template <typename Node>
void
basic_default_xml_doc_handler<Node>::handle_comment(const xercesc::Locator& locator, const XMLCh* const buf, const XMLSize_t len) {
  node_comment.reset(new xmlstring{buf, len});
}

template <typename Node>
void
basic_default_xml_doc_handler<Node>::handle_processing_instruction(const xercesc::Locator& locator, const XMLCh* const target, const XMLCh* const data) {}

template <typename Node>
void
basic_default_xml_doc_handler<Node>::handle_error(const xercesc::SAXParseException& e) {
  std::cerr << "error at file " << xmlstring{e.getSystemId()} << ", line " << e.getLineNumber() << ", col " << e.getColumnNumber() << ": " << xmlstring{e.getMessage()} << std::endl;
}

template <typename Node>
void
basic_default_xml_doc_handler<Node>::handle_fatal_error(const xercesc::SAXParseException& e) {
  std::cerr << "fatal error at file " << xmlstring{e.getSystemId()} << ", line " << e.getLineNumber() << ", col " << e.getColumnNumber() << ": " << xmlstring{e.getMessage()} << std::endl;
}

template <typename Node>
void
basic_default_xml_doc_handler<Node>::handle_warning(const xercesc::SAXParseException& e) {
  std::cerr << "warning at file " << xmlstring{e.getSystemId()} << ", line " << e.getLineNumber() << ", col " << e.getColumnNumber() << ": " << xmlstring{e.getMessage()} << std::endl;
}

using default_xml_doc_handler = basic_default_xml_doc_handler<xml_graph::xml_node>;

template <typename Node> class basic_xml_doc_parser {
  basic_xml_doc_handler<Node>& doc_handler;

 public:
  ~basic_xml_doc_parser();
  basic_xml_doc_parser(basic_xml_doc_handler<Node>& doc_handler);

  std::unique_ptr<const Node> parse_doc(const char* file);
};

template <typename Node> basic_xml_doc_parser<Node>::~basic_xml_doc_parser() {
  xercesc::XMLPlatformUtils::Terminate();
}

template <typename Node> basic_xml_doc_parser<Node>::basic_xml_doc_parser(basic_xml_doc_handler<Node>& doc_handler) : doc_handler(doc_handler) {
  xercesc::XMLPlatformUtils::Initialize();
}

template <typename Node> class xml_doc_delegator : public xercesc::DefaultHandler {
  basic_xml_doc_handler<Node>& doc_handler;
  const xercesc::Locator* locator;

  void characters(const XMLCh* const buf, const XMLSize_t len) override { doc_handler.handle_content(*locator, buf, len); }
  void endDocument() override { doc_handler.handle_end_document(*locator); }
  void startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const xercesc_3_1::Attributes& attrs) override { doc_handler.handle_start_element(*locator, uri, localname, qname, attrs); }
  void endElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname) override { doc_handler.handle_end_element(*locator, uri, localname, qname); }
  void comment(const XMLCh* const buf, const XMLSize_t cnt) override { doc_handler.handle_comment(*locator, buf, cnt); }
  void processingInstruction(const XMLCh* const target, const XMLCh* const data) override { doc_handler.handle_processing_instruction(*locator, target, data); }
  void warning(const xercesc::SAXParseException& e) override { doc_handler.handle_warning(e); }
  void error(const xercesc::SAXParseException& e) override { doc_handler.handle_error(e); }
  void fatalError(const xercesc::SAXParseException& e) override { doc_handler.handle_fatal_error(e); }

  void setDocumentLocator(const xercesc::Locator* locator) override { this->locator = locator; }

 public:
  xml_doc_delegator(basic_xml_doc_handler<Node>& doc_handler) : doc_handler{doc_handler}, locator{} {}
};

template <typename Node>
std::unique_ptr<const Node>
basic_xml_doc_parser<Node>::parse_doc(const char* file) {
  std::unique_ptr<xercesc::SAX2XMLReader> parser{xercesc::XMLReaderFactory::createXMLReader()};
  parser->setFeature(xercesc::XMLUni::fgSAX2CoreValidation, false);
  parser->setFeature(xercesc::XMLUni::fgSAX2CoreNameSpaces, false);

  xml_doc_delegator<Node> doc_delegator{doc_handler};
  parser->setContentHandler(&doc_delegator);
  parser->setErrorHandler(&doc_delegator);
  parser->setLexicalHandler(&doc_delegator);

  parser->parse(file);
  return doc_handler.doc();
}

using xml_doc_parser = basic_xml_doc_parser<xml_graph::xml_node>;
}
#endif
