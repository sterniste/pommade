#include <algorithm>
#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "xml_graph.h"
#include "xml_parser.h"

namespace pommade {
using namespace std;
using namespace xml_graph;

xml_node
rewrite_model_version(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
rewrite_group_id(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
rewrite_artifact_id(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
rewrite_parent_version(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
rewrite_relative_path(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
rewrite_parent(const xml_node& node) {
  assert(!node.content && node.tree() && node.tree()->node_cnt() <= 4);

  const vector<const xml_node*> parent_tree{node.tree()->find_in({"groupId", "artifactId", "version", "relativePath"})};
  assert(parent_tree.size() == 4 && parent_tree[0] && parent_tree[1] && parent_tree[2]);

  xml_node rw_parent{node.lineno, node.level, node.name, node.comment.get()};
  rw_parent.add_subnode(rewrite_group_id(*parent_tree[0]));
  rw_parent.add_subnode(rewrite_artifact_id(*parent_tree[1]));
  rw_parent.add_subnode(rewrite_parent_version(*parent_tree[2]));
  if (parent_tree[3])
    rw_parent.add_subnode(rewrite_relative_path(*parent_tree[3]));

  return rw_parent;
}

xml_node
rewrite_version(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
rewrite_packaging(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
rewrite_property(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
rewrite_properties(const xml_node* node) {
  assert(node);
  xml_node rw_properties{node->lineno, node->level, node->name, node->comment.get()};
  if (!node->tree())
    return rw_properties;

  vector<xml_node> subnodes;
  for (auto cit = node->tree()->cbegin(); cit != node->tree()->cend(); ++cit)
    subnodes.push_back(move(rewrite_property(*cit)));
  sort(subnodes.begin(), subnodes.end(), [](const xml_node& a, const xml_node& b) { return a.name < b.name; });
  for (auto&& subnode : subnodes)
    rw_properties.add_subnode(move(subnode));

  return rw_properties;
}

xml_node
rewrite_scm_element(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
rewrite_scm(const xml_node& node) {
  xml_node rw_scm{node.lineno, node.level, node.name, node.comment.get()};
  if (!node.tree())
    return rw_scm;

  vector<xml_node> subnodes;
  for (auto cit = node.tree()->cbegin(); cit != node.tree()->cend(); ++cit)
    subnodes.push_back(move(rewrite_scm_element(*cit)));
  sort(subnodes.begin(), subnodes.end(), [](const xml_node& a, const xml_node& b) { return a.name < b.name; });
  for (auto&& subnode : subnodes)
    rw_scm.add_subnode(move(subnode));

  return rw_scm;
}

xml_node
rewrite_distribution_management_element(const xml_node& node) {
  assert(!node.content && node.tree());
  return xml_node{node};
}

xml_node
rewrite_distribution_management(const xml_node& node) {
  xml_node rw_distribution_management{node.lineno, node.level, node.name, node.comment.get()};
  if (!node.tree())
    return rw_distribution_management;

  vector<xml_node> subnodes;
  for (auto cit = node.tree()->cbegin(); cit != node.tree()->cend(); ++cit)
    subnodes.push_back(move(rewrite_distribution_management_element(*cit)));
  sort(subnodes.begin(), subnodes.end(), [](const xml_node& a, const xml_node& b) { return a.name < b.name; });
  for (auto&& subnode : subnodes)
    rw_distribution_management.add_subnode(move(subnode));

  return rw_distribution_management;
}

xml_node
rewrite_scope(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
rewrite_dependency(const xml_node& node) {
  assert(node.name == "dependency" && !node.content && node.tree() && node.tree()->node_cnt() <= 5);

  const vector<const xml_node*> dependency_tree{node.tree()->find_in(vector<const char*>{"groupId", "artifactId", "version", "packaging", "scope"})};
  assert(dependency_tree.size() == 5 && dependency_tree[0] && dependency_tree[1]);

  xml_node rw_dependency{node.lineno, node.level, node.name, node.comment.get()};
  rw_dependency.add_subnode(rewrite_group_id(*dependency_tree[0]));
  rw_dependency.add_subnode(rewrite_artifact_id(*dependency_tree[1]));
  if (dependency_tree[2])
    rw_dependency.add_subnode(rewrite_version(*dependency_tree[2]));
  if (dependency_tree[3])
    rw_dependency.add_subnode(rewrite_packaging(*dependency_tree[3]));
  if (dependency_tree[4])
    rw_dependency.add_subnode(rewrite_scope(*dependency_tree[4]));

  return rw_dependency;
}

xml_node
rewrite_dependencies(const xml_node& node) {
  assert(node.name == "dependencies" && !node.content && node.tree());

  xml_node rw_dependencies{node.lineno, node.level, node.name, node.comment.get()};
  vector<xml_node> subnodes;
  for (auto cit = node.tree()->cbegin(); cit != node.tree()->cend(); ++cit)
    subnodes.push_back(move(rewrite_dependency(*cit)));
  sort(subnodes.begin(), subnodes.end(), [](const xml_node& a, const xml_node& b) {
    auto a_cit = a.tree()->cbegin(), b_cit = b.tree()->cbegin();
    if (*a_cit->content < *b_cit->content)
      return true;
    if (*a_cit->content == *b_cit->content)
      return *(++a_cit)->content < *(++b_cit)->content;
    return false;
  });
  for (auto&& subnode : subnodes)
    rw_dependencies.add_subnode(move(subnode));

  return rw_dependencies;
}

xml_node
rewrite_dependency_management(const xml_node& node) {
  xml_node rw_dependency_management{node.lineno, node.level, node.name, node.comment.get()};
  if (!node.tree())
    return rw_dependency_management;

  rw_dependency_management.add_subnode(move(rewrite_dependencies(*node.tree()->cbegin())));

  return rw_dependency_management;
}

xml_node
rewrite_build(const xml_node& node) {
  return xml_node{node};
}

xml_node
rewrite_modules(const xml_node& node) {
  return xml_node{node};
}

xml_node
rewrite_profiles(const xml_node& node) {
  return xml_node{node};
}

xml_node
rewrite_project(const xml_node* node) {
  assert(node && !node->content && node->tree() && node->tree()->node_cnt() <= 14);

  const vector<const xml_node*> project_tree{node->tree()->find_in({"modelVersion", "parent", "groupId", "artifactId", "version", "packaging", "properties", "scm", "distributionManagement", "dependencyManagement", "dependencies", "build", "modules", "profiles"})};
  assert(project_tree.size() == 14 && project_tree[0] && project_tree[2] && project_tree[3]);

  xml_node rw_project{node->lineno, node->level, node->name, node->comment.get()};
  rw_project.add_subnode(rewrite_model_version(*project_tree[0]));
  if (project_tree[1])
    rw_project.add_subnode(rewrite_parent(*project_tree[1]));
  rw_project.add_subnode(rewrite_group_id(*project_tree[2]));
  rw_project.add_subnode(rewrite_artifact_id(*project_tree[3]));
  if (project_tree[4])
    rw_project.add_subnode(rewrite_version(*project_tree[4]));
  if (project_tree[5])
    rw_project.add_subnode(rewrite_packaging(*project_tree[5]));
  if (project_tree[6])
    rw_project.add_subnode(rewrite_properties(project_tree[6]));
  if (project_tree[7])
    rw_project.add_subnode(rewrite_scm(*project_tree[7]));
  if (project_tree[8])
    rw_project.add_subnode(rewrite_distribution_management(*project_tree[8]));
  if (project_tree[9])
    rw_project.add_subnode(rewrite_dependency_management(*project_tree[9]));
  if (project_tree[10])
    rw_project.add_subnode(rewrite_dependencies(*project_tree[10]));
  if (project_tree[11])
    rw_project.add_subnode(rewrite_build(*project_tree[11]));
  if (project_tree[12])
    rw_project.add_subnode(rewrite_modules(*project_tree[12]));
  if (project_tree[13])
    rw_project.add_subnode(rewrite_profiles(*project_tree[13]));
  return rw_project;
}

xml_node
rewrite_pom(const xml_node* node) {
  assert(node);
  if (node->name != "project" || !node->tree())
    throw runtime_error{"root project node missing or empty"};
  return rewrite_project(node);
}
}
