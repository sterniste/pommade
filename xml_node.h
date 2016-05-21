#ifndef XML_NODE_H
#define XML_NODE_H

#include <iosfwd>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace pommade {

class xml_tree;

class xml_node {
  friend std::ostream& operator<<(std::ostream& os, const xml_node& node);

 public:
  unsigned int level;
  std::string name;
  std::unique_ptr<const std::string> comment;
  std::unique_ptr<const std::string> content;

 private:
  std::unique_ptr<xml_tree> subtree;

 public:
  xml_node() {}
  xml_node(xml_node&& that) : level{that.level}, name{std::move(that.name)}, comment{std::move(that.comment)}, content{std::move(that.content)}, subtree{std::move(that.subtree)} {}

  xml_node(unsigned int level, const std::string& name) : level{level}, name{name} {}
  xml_node(unsigned int level, const std::string& name, std::unique_ptr<const std::string>&& comment) : level{level}, name{name}, comment{std::move(comment)}, subtree{std::move(subtree)} {}

  bool operator==(const xml_node& that) const { return level == that.level && name == that.name; }
  bool operator<(const xml_node& that) const { return level < that.level || (level == that.level && name < that.name); }
  xml_node& operator=(xml_node&& that);

  xml_node* add_subnode(xml_node&& subnode);
  const xml_tree* tree() const { return subtree ? subtree.get() : nullptr; }
};

std::ostream& operator<<(std::ostream& os, const xml_node& node);

class xml_tree_iterator {
  friend class xml_tree;

  std::vector<std::unique_ptr<xml_node>>::const_iterator nodes_it;

  xml_tree_iterator(std::vector<std::unique_ptr<xml_node>>::const_iterator nodes_it) : nodes_it{nodes_it} {}

 public:
  void operator++() { nodes_it++; }
  bool operator!=(const xml_tree_iterator& that) const { return that.nodes_it != nodes_it; }
  const xml_node& operator*() const { return **nodes_it; }
};

class xml_tree {
  friend std::ostream& operator<<(std::ostream& os, const xml_tree& tree);

  std::unordered_set<std::string> names;
  std::vector<std::unique_ptr<xml_node>> nodes;

 public:
  xml_node* add_node(xml_node&& node);
  unsigned int node_cnt() const { return static_cast<unsigned int>(nodes.size()); }
  xml_tree_iterator cbegin() const { return xml_tree_iterator{nodes.cbegin()}; }
  xml_tree_iterator cend() const { return xml_tree_iterator{nodes.cend()}; }
};

std::ostream& operator<<(std::ostream& os, const xml_tree& tree);
}

#endif
