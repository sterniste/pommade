#include <ostream>
#include <utility>

#include "xml_tag.h"

namespace pommade {
using namespace std;

xml_tag&
xml_tag::operator=(xml_tag&& that) {
  path = move(that.path);
  name = move(that.name);
  comment = move(that.comment);
  content = move(that.content);
  subtags = move(that.subtags);
  return *this;
}

unsigned int
xml_tag::subtag_cnt() const {
  return subtags ? subtags->tag_cnt() : 0;
}

xml_tag*
xml_tag::add_subtag(xml_tag&& subtag) {
  if (!subtags)
    subtags.reset(new xml_subtags{});
  return subtags->add_tag(move(subtag));
}

void
xml_tag::sort_subtags() {
}
  
ostream&
operator<<(ostream& os, const xml_tag& tag) {
}

xml_tag*
xml_subtags::add_tag(xml_tag&& tag) {
  name_order.insert(pair<string, unsigned int>{tag.name, tags.size()});
  tags.push_back(unique_ptr<xml_tag>{new xml_tag{move(tag)}});
}
  
void
xml_subtags::sort_tags() {
}
  
ostream&
operator<<(ostream& os, const xml_subtags& subtags) {
}
}
