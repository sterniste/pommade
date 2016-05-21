#include <algorithm>
#include <cassert>
#include <cctype>
#include <iostream>
#include <utility>
#include <vector>

#include <xercesc/sax/Locator.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/util/XMLString.hpp>

#include "xml_handler.h"
#include "xml_node.h"

namespace pommade {
using namespace std;
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
xml_handler::ignorable_newlines(const string& content) {
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
xml_handler::characters(const XMLCh* const buf, const XMLSize_t len) {
  const xmlstring content{buf, len};
  const int nl_cnt{ignorable_newlines(content)};
  if (nl_cnt < 0) {
    assert(!node_path.empty());
    auto* const nodep = nodep_stack.top();
    assert(!nodep->content && !nodep->tree());
    nodep->content.reset(new xmlstring{content});
    node_comment.reset();
  }
}

void
xml_handler::endDocument() {
  assert(node_path.empty() && nodep_stack.empty());
  if (node_comment) {
    cerr << "discarding comment before document end; line " << locator->getLineNumber() << endl;
    node_comment.reset();
  }
  assert(root_node);
  cout << *root_node;
  cout.flush();
}

void
xml_handler::startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const Attributes& attrs) {
  xml_node* nodep{};
  if (!root_node) {
    assert(nodep_stack.empty());
    root_node.reset(new xml_node{0, xmlstring{qname}, move(node_comment)});
    nodep = root_node.get();
  } else {
    assert(!nodep_stack.empty() && !nodep_stack.top()->content);
    nodep = nodep_stack.top()->add_subnode(xml_node{nodep_stack.top()->level + 1, xmlstring{qname}, move(node_comment)});
  }
  nodep_stack.push(nodep);

  node_path += '/' + xmlstring{qname};
}

void
xml_handler::endElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname) {
  const string::size_type pos{node_path.rfind('/')};
  assert(pos != string::npos && xmlstring{qname} == node_path.substr(pos + 1));
  if (node_comment) {
    cerr << "discarding comment before '" + node_path + "' end; line " << locator->getLineNumber() << endl;
    node_comment.reset();
  }

  assert(!nodep_stack.empty());
  nodep_stack.pop();

  node_path = node_path.substr(0, pos);
}

void
xml_handler::comment(const XMLCh* const buf, const XMLSize_t len) {
  node_comment.reset(new xmlstring{buf, len});
}

void
xml_handler::processingInstruction(const XMLCh* const target, const XMLCh* const data) {}

void
xml_handler::error(const SAXParseException& e) {
  cerr << "error at file " << xmlstring{e.getSystemId()} << ", line " << e.getLineNumber() << ", col " << e.getColumnNumber() << ": " << xmlstring{e.getMessage()} << endl;
}

void
xml_handler::fatalError(const SAXParseException& e) {
  cerr << "fatal error at file " << xmlstring{e.getSystemId()} << ", line " << e.getLineNumber() << ", col " << e.getColumnNumber() << ": " << xmlstring{e.getMessage()} << endl;
}

void
xml_handler::warning(const SAXParseException& e) {
  cerr << "warning at file " << xmlstring{e.getSystemId()} << ", line " << e.getLineNumber() << ", col " << e.getColumnNumber() << ": " << xmlstring{e.getMessage()} << endl;
}
}
