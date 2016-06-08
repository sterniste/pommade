#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "xml_graph.h"

namespace xml_graph {
using namespace std;

xml_node::xml_node(const xml_node& that) : lineno{that.lineno}, level{that.level}, name{that.name}, comment{that.comment ? new string{*that.comment} : nullptr}, content{that.content ? new string{*that.content} : nullptr}, subtree{that.subtree ? new xml_tree{*that.subtree} : nullptr} {}

xml_node*
xml_node::add_subnode(xml_node&& subnode) {
  if (!subtree)
    subtree.reset(new xml_tree{});
  return subtree->add_node(move(subnode));
}

void
xml_node::add_subnodes(vector<unique_ptr<const xml_node>>&& subnodes) {
  if (!subtree)
    subtree.reset(new xml_tree{});
  return subtree->add_nodes(move(subnodes));
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
  names.insert(node.name);
  xml_node* nodep{};
  nodes.push_back(unique_ptr<const xml_node>{nodep = new xml_node{move(node)}});
  return nodep;
}

void
xml_tree::add_nodes(vector<unique_ptr<const xml_node>>&& nodes) {
  for (const auto& node : nodes)
    names.insert(node->name);
  move(nodes.begin(), nodes.end(), back_inserter(this->nodes));
}

vector<const xml_node*>
xml_tree::find_in(const vector<const char*>& name_in) const {
  vector<const xml_node*> found{};
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

vector<const xml_node*>
xml_tree::find_not_in(const vector<const char*>& name_not_in) const {
  vector<const xml_node*> found{};
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

xml_tree::xml_tree(const xml_tree& that) : names{that.names} {
  for (const auto& node : that.nodes)
    nodes.push_back(node ? unique_ptr<xml_node>{new xml_node{*node}} : unique_ptr<xml_node>{});
}

ostream& operator<<(ostream& os, const xml_tree& tree) {
  for (const auto& node : tree.nodes)
    os << *node;
  return os;
}
}
