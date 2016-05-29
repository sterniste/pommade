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
  unsigned int lineno;
  unsigned int level;
  std::string name;
  std::unique_ptr<const std::string> comment;
  std::unique_ptr<const std::string> content;

 private:
  std::unique_ptr<xml_tree> subtree;

 public:
  xml_node() {}
  xml_node(const xml_node& that);

  xml_node(unsigned int lineno, unsigned int level, const std::string& name, const std::string* comment = nullptr, const std::string* content = nullptr) : lineno{lineno}, level{level}, name{name}, comment{comment ? new std::string{*comment} : nullptr}, content{content ? new std::string{*content} : nullptr} {}
  xml_node& operator=(const xml_node& that);

  bool operator==(const xml_node& that) const { return level == that.level && name == that.name; }
  bool operator<(const xml_node& that) const { return level < that.level || (level == that.level && name < that.name); }

  xml_node* add_subnode(xml_node&& subnode);
  const xml_tree* tree() const { return subtree ? subtree.get() : nullptr; }
};

std::ostream& operator<<(std::ostream& os, const xml_node& node);

class xml_tree_iterator {
  friend class xml_tree;

  std::vector<std::unique_ptr<xml_node>>::const_iterator nodes_it;

  xml_tree_iterator(std::vector<std::unique_ptr<xml_node>>::const_iterator nodes_it) : nodes_it{nodes_it} {}

 public:
  xml_tree_iterator operator++() { ++nodes_it; return *this; }
  bool operator!=(const xml_tree_iterator& that) const { return that.nodes_it != nodes_it; }
  const xml_node& operator*() const { return **nodes_it; }
  const xml_node* operator->() const { return nodes_it->get(); }
};

class xml_tree {
  friend std::ostream& operator<<(std::ostream& os, const xml_tree& tree);

  std::unordered_set<std::string> names;
  std::vector<std::unique_ptr<xml_node>> nodes;

 public:
  xml_tree() {}
  xml_tree(const xml_tree& that);
    
  xml_node* add_node(xml_node&& node);

  unsigned int node_cnt() const { return static_cast<unsigned int>(nodes.size()); }
  xml_tree_iterator cbegin() const { return xml_tree_iterator{nodes.cbegin()}; }
  xml_tree_iterator cend() const { return xml_tree_iterator{nodes.cend()}; }

  std::vector<const xml_node*> find_in(const std::vector<const char*>& name_in) const;
  std::vector<const xml_node*> find_not_in(const std::vector<const char*>& name_not_in) const;
};

std::ostream& operator<<(std::ostream& os, const xml_tree& tree);
}

#endif
