#include <algorithm>
#include <cassert>
#include <cctype>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <xercesc/sax/Locator.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUni.hpp>

#include "xml_graph.h"
#include "xml_parser.h"

namespace xml_parser {
using namespace std;
using namespace xml_graph;
using namespace xml_parser;  
using namespace xercesc_3_1;

xmlstring::xmlstring(const XMLCh* buf) {
  char* cp{XMLString::transcode(buf)};
  string::operator=(cp);
  XMLString::release(&cp);
}

xmlstring::xmlstring(const XMLCh* buf, XMLSize_t len) {
  char* const cp = new char[3 * len + 1];
  XMLString::transcode(buf, cp, 3 * len);
  string::operator=(cp);
  delete[] cp;
}

int
basic_xml_doc_handler::ignorable_newlines(const string& content) {
  unsigned int nl_cnt{};
  for (const auto c : content) {
    if (!isspace(c))
      return -1;
    if (c == '\n')
      ++nl_cnt;
  }
  return nl_cnt;
}

void
basic_xml_doc_handler::handle_content(const Locator& locator, const XMLCh* const buf, const XMLSize_t len) {
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

void
basic_xml_doc_handler::handle_end_document(const Locator& locator) {
  assert(node_path.empty() && nodep_stack.empty());
  if (node_comment) {
    cerr << "discarding comment before document end; line " << locator.getLineNumber() << endl;
    node_comment.reset();
  }
  assert(root_node);
}

void
basic_xml_doc_handler::handle_start_element(const Locator& locator, const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const Attributes& attrs) {
  xml_node* nodep{};
  if (!root_node) {
    assert(nodep_stack.empty());
    root_node.reset(new xml_node{locator.getLineNumber(), 0, xmlstring{qname}, node_comment.get()});
    nodep = root_node.get();
  } else {
    assert(!nodep_stack.empty() && !nodep_stack.top()->get_content());
    nodep = nodep_stack.top()->add_subnode(xml_node{locator.getLineNumber(), nodep_stack.top()->level + 1, xmlstring{qname}, node_comment.get()});
  }
  node_comment.reset();
  nodep_stack.push(nodep);

  node_path += '/' + xmlstring{qname};
}

void
basic_xml_doc_handler::handle_end_element(const Locator& locator, const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname) {
  const string::size_type pos{node_path.rfind('/')};
  assert(pos != string::npos && xmlstring{qname} == node_path.substr(pos + 1));
  if (node_comment) {
    cerr << "discarding comment before '" + node_path + "' end; line " << locator.getLineNumber() << endl;
    node_comment.reset();
  }

  assert(!nodep_stack.empty());
  nodep_stack.pop();

  node_path = node_path.substr(0, pos);
}

void
basic_xml_doc_handler::handle_comment(const Locator& locator, const XMLCh* const buf, const XMLSize_t len) {
  node_comment.reset(new xmlstring{buf, len});
}

void
basic_xml_doc_handler::handle_processing_instruction(const Locator& locator, const XMLCh* const target, const XMLCh* const data) {}

void
basic_xml_doc_handler::handle_error(const SAXParseException& e) {
  cerr << "error at file " << xmlstring{e.getSystemId()} << ", line " << e.getLineNumber() << ", col " << e.getColumnNumber() << ": " << xmlstring{e.getMessage()} << endl;
}

void
basic_xml_doc_handler::handle_fatal_error(const SAXParseException& e) {
  cerr << "fatal error at file " << xmlstring{e.getSystemId()} << ", line " << e.getLineNumber() << ", col " << e.getColumnNumber() << ": " << xmlstring{e.getMessage()} << endl;
}

void
basic_xml_doc_handler::handle_warning(const SAXParseException& e) {
  cerr << "warning at file " << xmlstring{e.getSystemId()} << ", line " << e.getLineNumber() << ", col " << e.getColumnNumber() << ": " << xmlstring{e.getMessage()} << endl;
}

xml_doc_parser::~xml_doc_parser() {
  XMLPlatformUtils::Terminate();
}

xml_doc_parser::xml_doc_parser(xml_doc_handler& doc_handler) : doc_handler(doc_handler) {
  XMLPlatformUtils::Initialize();
}

class xml_doc_delegator : public DefaultHandler {
  xml_doc_handler& doc_handler;
  const Locator* locator;

  void characters(const XMLCh* const buf, const XMLSize_t len) override { doc_handler.handle_content(*locator, buf, len); }
  void endDocument() override { doc_handler.handle_end_document(*locator); }
  void startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const Attributes& attrs) override { doc_handler.handle_start_element(*locator, uri, localname, qname, attrs); }
  void endElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname) override { doc_handler.handle_end_element(*locator, uri, localname, qname); }
  void comment(const XMLCh* const buf, const XMLSize_t cnt) override { doc_handler.handle_comment(*locator, buf, cnt); }
  void processingInstruction(const XMLCh* const target, const XMLCh* const data) override { doc_handler.handle_processing_instruction(*locator, target, data); }
  void warning(const SAXParseException& e) override { doc_handler.handle_warning(e); }
  void error(const SAXParseException& e) override { doc_handler.handle_error(e); }
  void fatalError(const SAXParseException& e) override { doc_handler.handle_fatal_error(e); }

  void setDocumentLocator(const Locator* locator) override { this->locator = locator; }

 public:
  xml_doc_delegator(xml_doc_handler& doc_handler) : doc_handler{doc_handler}, locator{} {}
};

unique_ptr<const xml_node>  
xml_doc_parser::parse_doc(const char* file) {
  unique_ptr<SAX2XMLReader> parser{XMLReaderFactory::createXMLReader()};
  parser->setFeature(XMLUni::fgSAX2CoreValidation, false);
  parser->setFeature(XMLUni::fgSAX2CoreNameSpaces, false);

  xml_doc_delegator doc_delegator{doc_handler};
  parser->setContentHandler(&doc_delegator);
  parser->setErrorHandler(&doc_delegator);
  parser->setLexicalHandler(&doc_delegator);

  parser->parse(file);
  return doc_handler.doc();
}
}
