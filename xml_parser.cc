#include <string>

#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUni.hpp>

#include "xml_parser.h"

namespace xml_parser {
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
}
