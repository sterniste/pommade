#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
namespace xercesc_3_1 { class Attributes; }

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

struct xml_tag {
  std::string path;
  std::unique_ptr<const std::string> content;

  xml_tag(const std::string& path, std::unique_ptr<const std::string>&& content) : path{path}, content{std::move(content)} {}
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
  unsigned int level, element_level, nl_cnt;
  std::string tag_path, tag_comment, tag_content;
  std::vector<xml_tag> tags;

  static int ignorable_newlines(const XMLCh* const buf, const XMLSize_t len);

 public:
  ~pom_xml_handler() {}
  pom_xml_handler();

  void writeChars(const XMLByte* const buf, const XMLSize_t len, xercesc::XMLFormatter* const formatter) override;
  void characters(const XMLCh* const buf, const XMLSize_t len) override;

  void startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const xercesc_3_1::Attributes& attrs) override;
  void endElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname) override;

  void comment(const XMLCh* const buf, const XMLSize_t cnt) override;
  void processingInstruction(const XMLCh* const target, const XMLCh* const data) override;

  void warning(const xercesc::SAXParseException& e) override;
  void error(const xercesc::SAXParseException& e) override;
  void fatalError(const xercesc::SAXParseException& e) override;
};
}
