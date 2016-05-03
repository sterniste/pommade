#include <algorithm>
#include <cassert>
#include <cctype>
#include <iostream>
#include <utility>
#include <vector>

#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

#include "pom_xml_handler.h"
#include "xml_tag.h"

namespace pommade {
using namespace std;
using namespace xercesc_3_1;

const XMLCh pom_xml_handler::end_comment[] = {xercesc::chDash, xercesc::chDash, xercesc::chCloseAngle, xercesc::chNull};
const XMLCh pom_xml_handler::end_element[] = {xercesc::chOpenAngle, xercesc::chForwardSlash, xercesc::chNull};
const XMLCh pom_xml_handler::end_pi[] = {xercesc::chQuestion, xercesc::chCloseAngle, xercesc::chNull};
const XMLCh pom_xml_handler::start_comment[] = {xercesc::chOpenAngle, xercesc::chBang, xercesc::chDash, xercesc::chDash, xercesc::chNull};
const XMLCh pom_xml_handler::start_pi[] = {xercesc::chOpenAngle, xercesc::chQuestion, xercesc::chNull};
const XMLCh pom_xml_handler::xml_decl1[] = {xercesc::chOpenAngle, xercesc::chQuestion, xercesc::chLatin_x, xercesc::chLatin_m, xercesc::chLatin_l, xercesc::chSpace, xercesc::chLatin_v, xercesc::chLatin_e, xercesc::chLatin_r, xercesc::chLatin_s, xercesc::chLatin_i, xercesc::chLatin_o, xercesc::chLatin_n, xercesc::chEqual, xercesc::chDoubleQuote, xercesc::chDigit_1, xercesc::chPeriod, xercesc::chDigit_0, xercesc::chDoubleQuote, xercesc::chSpace, xercesc::chLatin_e, xercesc::chLatin_n, xercesc::chLatin_c, xercesc::chLatin_o, xercesc::chLatin_d, xercesc::chLatin_i, xercesc::chLatin_n, xercesc::chLatin_g, xercesc::chEqual, xercesc::chDoubleQuote, xercesc::chNull};
const XMLCh pom_xml_handler::xml_decl2[] = {xercesc::chDoubleQuote, xercesc::chQuestion, xercesc::chCloseAngle, xercesc::chLF, xercesc::chNull};

pom_xml_handler::pom_xml_handler() : formatter("UTF8", 0, this, XMLFormatter::NoEscapes, XMLFormatter::UnRep_CharRef) {
  formatter << xml_decl1 << formatter.getEncodingName() << xml_decl2;
}

void
pom_xml_handler::writeChars(const XMLByte* const buf, const XMLSize_t len, XMLFormatter* const) {}

int
pom_xml_handler::ignorable_newlines(const string& content) {
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
pom_xml_handler::characters(const XMLCh* const buf, const XMLSize_t len) {
  const xmlstring content{buf, len};
  const int nl_cnt{ignorable_newlines(content)};
  if (nl_cnt < 0) {
    assert(!tag_path.empty());
    auto* const tagp = tagp_stack.top();
    assert(tagp->subtag_cnt() == 0);
    tagp->content.reset(new xmlstring{buf, len});
    tag_comment.reset();
  }
}

void
pom_xml_handler::endDocument() {
  assert(tag_path.empty() && tagp_stack.empty());
  if (tag_comment) {
    cerr << "discarding comment before document end" << endl;
    cerr.flush();
    tag_comment.reset();
  }
  assert(root_tag);
  root_tag->sort_subtags();
  cout << *root_tag;
}

void
pom_xml_handler::startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const Attributes& attrs) {
  xml_tag* tagp{};
  if (!root_tag) {
    assert(tagp_stack.empty());
    root_tag.reset(new xml_tag{tag_path, xmlstring{qname}, move(tag_comment)});
    tagp = root_tag.get();
  } else {
    assert(!tagp_stack.empty() && !tagp->content);
    tagp = tagp_stack.top()->add_subtag(xml_tag{tag_path, xmlstring{qname}, move(tag_comment)});
  }
  tagp_stack.push(tagp);

  tag_path += '/' + xmlstring{qname};
}

void
pom_xml_handler::endElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname) {
  const string::size_type pos{tag_path.rfind('/')};
  assert(pos != string::npos && xmlstring{qname} == tag_path.substr(pos + 1));
  if (tag_comment) {
    cerr << "discarding comment before '" + tag_path + "' end" << endl;
    cerr.flush();
    tag_comment.reset();
  }

  assert(!tagp_stack.empty());
  tagp_stack.pop();

  tag_path = tag_path.substr(0, pos);
}

void
pom_xml_handler::comment(const XMLCh* const buf, const XMLSize_t len) {
  tag_comment.reset(new xmlstring{buf, len});
}

void
pom_xml_handler::processingInstruction(const XMLCh* const target, const XMLCh* const data) {}

void
pom_xml_handler::error(const SAXParseException& e) {
  cerr << "error at file " << xmlstring{e.getSystemId()} << ", line " << e.getLineNumber() << ", col " << e.getColumnNumber() << ": " << xmlstring{e.getMessage()} << endl;
}

void
pom_xml_handler::fatalError(const SAXParseException& e) {
  cerr << "fatal error at file " << xmlstring{e.getSystemId()} << ", line " << e.getLineNumber() << ", col " << e.getColumnNumber() << ": " << xmlstring{e.getMessage()} << endl;
}

void
pom_xml_handler::warning(const SAXParseException& e) {
  cerr << "warning at file " << xmlstring{e.getSystemId()} << ", line " << e.getLineNumber() << ", col " << e.getColumnNumber() << ": " << xmlstring{e.getMessage()} << endl;
}
}
