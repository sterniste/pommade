#include <ostream>
#include <utility>

#include "xml_node.h"

namespace pommade {
using namespace std;

xml_node& xml_node::operator=(xml_node&& that) {
  level = that.level;
  name = move(that.name);
  comment = move(that.comment);
  content = move(that.content);
  subtree = move(that.subtree);
  return *this;
}

xml_node*
xml_node::add_subnode(xml_node&& subnode) {
  if (!subtree)
    subtree.reset(new xml_tree{});
  return subtree->add_node(move(subnode));
}

ostream& operator<<(ostream& os, const xml_node& node) {
  if (node.comment) {
    for (auto i = 0U; i < node.level; ++i)
      os << '\t';
    os << "<!--" << *node.comment << "-->" << endl;
  }
  for (auto i = 0U; i < node.level; ++i)
    os << '\t';
  os << '<' << node.name << '>';
  if (node.subtree) {
    os << endl << *node.subtree;
    for (auto i = 0U; i < node.level; ++i)
      os << '\t';
  } else if (node.content)
    os << *node.content;
  os << "</" << node.name << '>' << endl;
  return os;
}

xml_node*
xml_tree::add_node(xml_node&& node) {
  if (!names.insert(node.name).second) {
  }
  nodes.push_back(unique_ptr<xml_node>{new xml_node{move(node)}});
  return nodes.crbegin()->get();
}

ostream& operator<<(ostream& os, const xml_tree& tree) {
  for (const auto& node : tree.nodes)
    os << *node;
  return os;
}
}
