#ifndef XML_TAG_H
#define XML_TAG_H

#include <iosfwd>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace pommade {

class xml_subtags;

class xml_tag {
  friend std::ostream& operator<<(std::ostream& os, const xml_tag& tag);

  std::unique_ptr<xml_subtags> subtags;

 public:
  std::string path, name;
  std::unique_ptr<const std::string> comment;
  std::unique_ptr<const std::string> content;

  xml_tag() {}
  xml_tag(xml_tag&& that) : path{std::move(that.path)}, name{std::move(that.name)}, comment{std::move(that.comment)}, content{std::move(that.content)} {}

  xml_tag(const std::string& path, const std::string& name) : path{path}, name{name} {}
  xml_tag(const std::string& path, const std::string& name, std::unique_ptr<const std::string>&& comment) : path{path}, name{name}, comment{std::move(comment)} {}

  bool operator==(const xml_tag& that) const { return path == that.path && name == that.name; }
  bool operator<(const xml_tag& that) const { return path < that.path || (path == that.path && name < that.name); }
  xml_tag& operator=(xml_tag&& that);

  xml_tag* add_subtag(xml_tag&& subtag);
  unsigned int subtag_cnt() const;
  void sort_subtags();
};

std::ostream& operator<<(std::ostream& os, const xml_tag& tag);

class xml_subtags {
  friend std::ostream& operator<<(std::ostream& os, const xml_subtags& subtags);

  std::unordered_map<std::string, unsigned int> name_order;
  std::vector<std::unique_ptr<xml_tag>> tags;

 public:
  xml_tag* add_tag(xml_tag&& tag);
  unsigned int tag_cnt() const { return tags.size(); }
  void sort_tags();
};

std::ostream& operator<<(std::ostream& os, const xml_subtags& subtags);
}

#endif
