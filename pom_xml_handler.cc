#include <algorithm>
#include <cctype>
#include <iostream>

#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

#include "pom_xml_handler.h"

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


pom_xml_handler::pom_xml_handler() : formatter("UTF8", 0, this, XMLFormatter::NoEscapes, XMLFormatter::UnRep_CharRef), level{}, element_level{}, nl_cnt{} {
  formatter << xml_decl1 << formatter.getEncodingName() << xml_decl2;
}

void
pom_xml_handler::writeChars(const XMLByte* const buf, const XMLSize_t len, XMLFormatter* const) {
  cout.write(reinterpret_cast<const char*>(buf), len);
  cout.flush();
}

int
pom_xml_handler::ignorable_newlines(const XMLCh* const buf, const XMLSize_t len) {
  const xmlstring ws{buf, len};
  unsigned int nl_cnt{};
  for (const auto c : ws) {
    if (!isspace(c))
      return -1;
    if (c == '\n')
      ++nl_cnt;
  }
  return nl_cnt;
}

void
pom_xml_handler::characters(const XMLCh* const buf, const XMLSize_t len) {
  const int nl_cnt{ignorable_newlines(buf, len)};
  if (nl_cnt < 0)
    formatter.formatBuf(buf, len, XMLFormatter::CharEscapes);
  else
    this->nl_cnt += nl_cnt;
}

void
pom_xml_handler::startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const Attributes& attrs) {
  if (level && nl_cnt > 1)
    formatter << XMLFormatter::NoEscapes << chLF;
  formatter << XMLFormatter::NoEscapes << chLF;
  if (level) {
    for (auto i = 0U; i < level; ++i)
      formatter << chSpace << chSpace;
  }
  element_level = level;

  formatter << XMLFormatter::NoEscapes << chOpenAngle << qname;
  const XMLSize_t len = attrs.getLength();
  for (XMLSize_t i = 0; i < len; i++)
    formatter << chSpace << attrs.getQName(i) << chEqual << chDoubleQuote << XMLFormatter::AttrEscapes << attrs.getValue(i) << XMLFormatter::NoEscapes << chDoubleQuote;
  formatter << chCloseAngle;

  ++level;
  nl_cnt = 0;
}

void
pom_xml_handler::endElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname) {
  if (--level > element_level) {
    formatter << XMLFormatter::NoEscapes << chLF;
    if (level) {
      for (auto i = 0U; i < level; ++i)
        formatter << chSpace << chSpace;
    }
  }
  formatter << XMLFormatter::NoEscapes << end_element << qname << chCloseAngle;

  element_level = nl_cnt = 0;
}

void
pom_xml_handler::comment(const XMLCh* const buf, const XMLSize_t len) {
  if (level && nl_cnt > 1)
    formatter << XMLFormatter::NoEscapes << chLF;
  if (nl_cnt)
    formatter << XMLFormatter::NoEscapes << chLF;
  if (level) {
    for (auto i = 0U; i < level; ++i)
      formatter << chSpace << chSpace;
  } else if (!nl_cnt)
    formatter << XMLFormatter::NoEscapes << chSpace;
  formatter << XMLFormatter::NoEscapes << start_comment;
  for (const auto* bufp = buf; bufp < buf + len; ++bufp)
    formatter << *bufp;
  formatter << end_comment;

  nl_cnt = 0;
}

void
pom_xml_handler::processingInstruction(const XMLCh* const target, const XMLCh* const data) {
  formatter << XMLFormatter::NoEscapes << start_pi << target;
  if (data)
    formatter << chSpace << data;
  formatter << XMLFormatter::NoEscapes << end_pi;
}

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

