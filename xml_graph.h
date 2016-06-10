#ifndef XML_GRAPH_H
#define XML_GRAPH_H

#include <iterator>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_set>
#include <vector>

namespace xml_graph {

template <typename Node> class xml_tree;
template <typename Node> std::ostream& operator<<(std::ostream& os, const xml_tree<Node>& tree);

template <typename Node> class basic_xml_node {
 public:
  const unsigned short lineno;
  const unsigned short level;
  const std::string name;
  const std::unique_ptr<const std::string> comment;

 private:
  std::unique_ptr<std::string> content;
  std::unique_ptr<xml_tree<Node>> subtree;

 public:
  basic_xml_node(unsigned short lineno, unsigned short level, const std::string& name, const std::string* comment = nullptr, const std::string* content = nullptr) : lineno{lineno}, level{level}, name{name}, comment{comment ? new std::string{*comment} : nullptr}, content{content ? new std::string{*content} : nullptr} {}
  basic_xml_node(const basic_xml_node& that) : lineno{that.lineno}, level{that.level}, name{that.name}, comment{that.comment ? new std::string{*that.comment} : nullptr}, content{that.content ? new std::string{*that.content} : nullptr}, subtree{that.subtree ? new xml_tree<Node>{*that.subtree} : nullptr} {}

  bool operator==(const basic_xml_node& that) const { return level == that.level && name == that.name; }
  bool operator<(const basic_xml_node& that) const { return level < that.level || (level == that.level && name < that.name); }

  const std::string* get_content() const { return content.get(); }
  void set_content(const std::string& s) { content.reset(new std::string{s}); }
  void append_content(const std::string& s) { content.get()->append(s); }

  Node* add_subnode(Node&& subnode);
  void add_subnodes(std::vector<std::unique_ptr<const Node>>&& subnodes);
  const xml_tree<Node>* tree() const { return subtree ? subtree.get() : nullptr; }

  friend std::ostream& operator<<(std::ostream& os, const basic_xml_node& node) {
    if (node.comment) {
      for (auto i = 0U; i < node.level; ++i)
        os << '\t';
      os << "<!--" << *node.comment << "-->" << std::endl;
    }
    for (auto i = 0U; i < node.level; ++i)
      os << '\t';
    os << '<' << node.name << '>';
    if (node.subtree) {
      os << std::endl << *node.subtree;
      for (auto i = 0U; i < node.level; ++i)
        os << '\t';
    } else if (node.content)
      os << *node.content;
    os << "</" << node.name << '>' << std::endl;
    return os;
  }
};

template <typename Node>
Node*
basic_xml_node<Node>::add_subnode(Node&& subnode) {
  if (!subtree)
    subtree.reset(new xml_tree<Node>{});
  return subtree->add_node(std::move(subnode));
}

template <typename Node>
void
basic_xml_node<Node>::add_subnodes(std::vector<std::unique_ptr<const Node>>&& subnodes) {
  if (!subtree)
    subtree.reset(new xml_tree<Node>{});
  return subtree->add_nodes(std::move(subnodes));
}

// TODO: is this necessary?
struct xml_node : public basic_xml_node<xml_node> {
  xml_node(const xml_node& that) : basic_xml_node{that} {}
  xml_node(unsigned short lineno, unsigned short level, const std::string& name, const std::string* comment = nullptr, const std::string* content = nullptr) : basic_xml_node{lineno, level, name, comment, content} {}
};

template <typename Node> class xml_tree_iterator {
  typename std::vector<std::unique_ptr<const Node>>::const_iterator nodes_it;

 public:
  xml_tree_iterator(typename std::vector<std::unique_ptr<const Node>>::const_iterator nodes_it) : nodes_it{nodes_it} {}

  xml_tree_iterator operator++();
  bool operator==(const xml_tree_iterator& that) const { return that.nodes_it == nodes_it; }
  bool operator!=(const xml_tree_iterator& that) const { return that.nodes_it != nodes_it; }
  const Node& operator*() const { return **nodes_it; }
  const Node* operator->() const { return nodes_it->get(); }
};

template <typename Node> xml_tree_iterator<Node> xml_tree_iterator<Node>::operator++() {
  ++nodes_it;
  return *this;
}

template <typename Node> class xml_tree {
  std::unordered_set<std::string> names;
  typename std::vector<std::unique_ptr<const Node>> nodes;

 public:
  xml_tree() {}
  xml_tree(const xml_tree& that);

  Node* add_node(Node&& node);
  void add_nodes(typename std::vector<std::unique_ptr<const Node>>&& nodes);

  unsigned int node_cnt() const { return static_cast<unsigned int>(nodes.size()); }
  xml_tree_iterator<Node> cbegin() const { return xml_tree_iterator<Node>{nodes.cbegin()}; }
  xml_tree_iterator<Node> cend() const { return xml_tree_iterator<Node>{nodes.cend()}; }

  std::vector<const Node*> find_in(const std::vector<const char*>& name_in) const;
  std::vector<const Node*> find_not_in(const std::vector<const char*>& name_not_in) const;
};

template <typename Node> xml_tree<Node>::xml_tree(const xml_tree& that) : names{that.names} {
  for (const auto& node : that.nodes)
    nodes.push_back(node ? std::unique_ptr<Node>{new Node{*node}} : std::unique_ptr<Node>{});
}

template <typename Node>
Node*
xml_tree<Node>::add_node(Node&& node) {
  names.insert(node.name);
  Node* nodep{};
  nodes.push_back(std::unique_ptr<const Node>{nodep = new Node{std::move(node)}});
  return nodep;
}

template <typename Node>
void
xml_tree<Node>::add_nodes(std::vector<std::unique_ptr<const Node>>&& nodes) {
  for (const auto& node : nodes)
    names.insert(node->name);
  std::move(nodes.begin(), nodes.end(), std::back_inserter(this->nodes));
}

template <typename Node>
std::vector<const Node*>
xml_tree<Node>::find_in(const std::vector<const char*>& name_in) const {
  std::vector<const Node*> found{};
  for (const auto& name : name_in) {
    bool found_name_in = false;
    for (auto cit = cbegin(); cit != cend(); ++cit) {
      if (cit->name == name) {
        found.push_back(&*cit);
        found_name_in = true;
      }
    }
    if (!found_name_in)
      found.push_back(nullptr);
  }
  return found;
}

template <typename Node>
std::vector<const Node*>
xml_tree<Node>::find_not_in(const std::vector<const char*>& name_not_in) const {
  std::vector<const Node*> found{};
  for (const auto& name : name_not_in) {
    bool found_name_not_in = false;
    for (auto cit = cbegin(); cit != cend(); ++cit) {
      if (cit->name != name) {
        found.push_back(&*cit);
        found_name_not_in = true;
      }
    }
    if (!found_name_not_in)
      found.push_back(nullptr);
  }
  return found;
}

template <typename Node> std::ostream& operator<<(std::ostream& os, const xml_tree<Node>& tree) {
  for (auto cit = tree.cbegin(); cit != tree.cend(); ++cit)
    os << *cit;
  return os;
}
}
#endif
