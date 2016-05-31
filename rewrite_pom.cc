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

void
add_nonempty_rewrite(xml_node& node, const xml_node* subnode, xml_node (*rewrite)(const xml_node&)) {
  if (subnode) {
    xml_node rw_subnode{rewrite(*subnode)};
    if (rw_subnode.content || rw_subnode.tree())
      node.add_subnode(move(rw_subnode));
  }
}

xml_node
rewrite_subnodes(const xml_node& node, xml_node (*rewrite)(const xml_node&)) {
  xml_node rw_node{node.lineno, node.level, node.name, node.comment.get()};
  if (node.tree()) {
    for (auto cit = node.tree()->cbegin(); cit != node.tree()->cend(); ++cit)
      rw_node.add_subnode(rewrite(*cit));
  }
  return rw_node;
}

xml_node
rewrite_sorted_subnodes(const xml_node& node, xml_node (*rewrite)(const xml_node&), bool (*compare)(const xml_node& a, const xml_node& b)) {
  xml_node rw_node{node.lineno, node.level, node.name, node.comment.get()};
  if (node.tree()) {
    vector<xml_node> subnodes;
    for (auto cit = node.tree()->cbegin(); cit != node.tree()->cend(); ++cit)
      subnodes.push_back(rewrite(*cit));
    sort(subnodes.begin(), subnodes.end(), compare);
    for (auto &&subnode : subnodes)
      rw_node.add_subnode(move(subnode));
  }
  return rw_node;
}

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
  add_nonempty_rewrite(rw_parent, parent_tree[3], [](const xml_node& node) { return rewrite_relative_path(node); });

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
rewrite_project_property(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
rewrite_project_properties(const xml_node& node) {
  assert(!node.content);
  return rewrite_sorted_subnodes(node, [](const xml_node& node) { return rewrite_project_property(node); }, [](const xml_node& a, const xml_node& b) { return a.name < b.name; });
}

xml_node
rewrite_scm_element(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
rewrite_scm(const xml_node& node) {
  assert(!node.content);
  return rewrite_sorted_subnodes(node, [](const xml_node& node) { return rewrite_scm_element(node); }, [](const xml_node& a, const xml_node& b) { return a.name < b.name; });
}

xml_node
rewrite_distribution_management_element(const xml_node& node) {
  assert(!node.content && node.tree());
  return xml_node{node};
}

xml_node
rewrite_distribution_management(const xml_node& node) {
  assert(!node.content);
  return rewrite_sorted_subnodes(node, [](const xml_node& node) { return rewrite_distribution_management_element(node); }, [](const xml_node& a, const xml_node& b) { return a.name < b.name; });
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
  add_nonempty_rewrite(rw_dependency, dependency_tree[2], [](const xml_node& node) { return rewrite_version(node); });
  add_nonempty_rewrite(rw_dependency, dependency_tree[3], [](const xml_node& node) { return rewrite_packaging(node); });
  add_nonempty_rewrite(rw_dependency, dependency_tree[4], [](const xml_node& node) { return rewrite_scope(node); });

  return rw_dependency;
}

xml_node
rewrite_dependencies(const xml_node& node) {
  assert(node.name == "dependencies" && !node.content);
  return rewrite_sorted_subnodes(node, [](const xml_node& node) { return rewrite_dependency(node); },
                                 [](const xml_node& a, const xml_node& b) {
                                   auto a_cit = a.tree()->cbegin(), b_cit = b.tree()->cbegin();
                                   if (*a_cit->content < *b_cit->content)
                                     return true;
                                   if (*a_cit->content == *b_cit->content)
                                     return *(++a_cit)->content < *(++b_cit)->content;
                                   return false;
                                 });
}

xml_node
rewrite_dependency_management(const xml_node& node) {
  assert(!node.content);

  xml_node rw_dependency_management{node.lineno, node.level, node.name, node.comment.get()};
  if (!node.tree())
    return rw_dependency_management;

  rw_dependency_management.add_subnode(rewrite_dependencies(*node.tree()->cbegin()));

  return rw_dependency_management;
}

xml_node
rewrite_module(const xml_node& node) {
  assert(node.name == "module" && node.content);
  return xml_node{node};
}

xml_node
rewrite_modules(const xml_node& node) {
  assert(!node.content && node.tree());
  return rewrite_subnodes(node, [](const xml_node& node) { return rewrite_module(node); });
}

xml_node
rewrite_id(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
rewrite_name(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
rewrite_value(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
rewrite_property(const xml_node& node, bool unvalued_ok = false) {
  assert(node.name == "property" && !node.content && node.tree());
  if (unvalued_ok)
    assert(node.tree()->node_cnt() <= 2);
  else
    assert(node.tree()->node_cnt() == 2);

  const vector<const xml_node*> property_tree{node.tree()->find_in(vector<const char*>{"name", "value"})};
  assert(property_tree.size() == 2 && property_tree[0]);
  if (!unvalued_ok)
    assert(property_tree[1]);

  xml_node rw_property{node.lineno, node.level, node.name, node.comment.get()};
  rw_property.add_subnode(rewrite_name(*property_tree[0]));
  if (!unvalued_ok)
    rw_property.add_subnode(rewrite_value(*property_tree[1]));
  else
    add_nonempty_rewrite(rw_property, property_tree[1], [](const xml_node& node) { return rewrite_value(node); });

  return rw_property;
}

xml_node
rewrite_properties(const xml_node& node) {
  assert(!node.content && node.tree());
  return rewrite_sorted_subnodes(node, [](const xml_node& node) { return rewrite_property(node); },
                                 [](const xml_node& a, const xml_node& b) {
                                   auto a_cit = a.tree()->cbegin(), b_cit = b.tree()->cbegin();
                                   return *a_cit->content < *b_cit->content;
                                 });
}

xml_node
rewrite_activation(const xml_node& node) {
  assert(!node.content && node.tree());
  return rewrite_sorted_subnodes(node, [](const xml_node& node) { return node.name == "property" ? rewrite_property(node, true) : xml_node{node}; }, [](const xml_node& a, const xml_node& b) { return a.name < b.name; });
}

xml_node
rewrite_configuration(const xml_node& node) {
  assert(!node.content && node.tree());
  return rewrite_sorted_subnodes(node, [](const xml_node& node) { return node.name == "properties" ? rewrite_properties(node) : xml_node{node}; }, [](const xml_node& a, const xml_node& b) { return a.name == "properties" || a.name < b.name; });
}

xml_node
rewrite_phase(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
rewrite_goal(const xml_node& node) {
  assert(node.name == "goal" && node.content && !node.tree());
  return xml_node{node};
}

xml_node
rewrite_goals(const xml_node& node) {
  assert(!node.content && node.tree());
  return rewrite_subnodes(node, [](const xml_node& node) { return rewrite_goal(node); });
}

xml_node
rewrite_execution(const xml_node& node) {
  assert(node.name == "execution" && !node.content && node.tree() && node.tree()->node_cnt() <= 4);

  const vector<const xml_node*> execution_tree{node.tree()->find_in(vector<const char*>{"id", "phase", "goals", "configuration"})};
  assert(execution_tree.size() == 4 && execution_tree[2]);

  xml_node rw_execution{node.lineno, node.level, node.name, node.comment.get()};
  add_nonempty_rewrite(rw_execution, execution_tree[0], [](const xml_node& node) { return rewrite_id(node); });
  add_nonempty_rewrite(rw_execution, execution_tree[1], [](const xml_node& node) { return rewrite_phase(node); });
  rw_execution.add_subnode(rewrite_goals(*execution_tree[2]));
  add_nonempty_rewrite(rw_execution, execution_tree[3], [](const xml_node& node) { return rewrite_configuration(node); });

  return rw_execution;
}

xml_node
rewrite_executions(const xml_node& node) {
  assert(!node.content && node.tree());
  return rewrite_subnodes(node, [](const xml_node& node) { return rewrite_execution(node); });
}

xml_node
rewrite_plugin(const xml_node& node) {
  assert(node.name == "plugin" && !node.content && node.tree() && node.tree()->node_cnt() <= 5);

  const vector<const xml_node*> plugin_tree{node.tree()->find_in(vector<const char*>{"groupId", "artifactId", "version", "configuration", "executions"})};
  assert(plugin_tree.size() == 5 && plugin_tree[1]);

  xml_node rw_plugin{node.lineno, node.level, node.name, node.comment.get()};
  add_nonempty_rewrite(rw_plugin, plugin_tree[0], [](const xml_node& node) { return rewrite_group_id(node); });
  rw_plugin.add_subnode(rewrite_artifact_id(*plugin_tree[1]));
  add_nonempty_rewrite(rw_plugin, plugin_tree[2], [](const xml_node& node) { return rewrite_version(node); });
  add_nonempty_rewrite(rw_plugin, plugin_tree[3], [](const xml_node& node) { return rewrite_configuration(node); });
  add_nonempty_rewrite(rw_plugin, plugin_tree[4], [](const xml_node& node) { return rewrite_executions(node); });

  return rw_plugin;
}

xml_node
rewrite_plugins(const xml_node& node) {
  assert(node.name == "plugins" && !node.content);
  return rewrite_subnodes(node, [](const xml_node& node) { return rewrite_plugin(node); });
}

xml_node
rewrite_plugin_management(const xml_node& node) {
  assert(!node.content && node.tree() && node.tree()->node_cnt() == 1);
  return rewrite_subnodes(node, [](const xml_node& node) { return rewrite_plugins(node); });
}

xml_node
rewrite_filtering(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
rewrite_include(const xml_node& node) {
  assert(node.name == "include" && node.content && !node.tree());
  return xml_node{node};
}

xml_node
rewrite_includes(const xml_node& node) {
  assert(!node.content);
  return rewrite_subnodes(node, [](const xml_node& node) { return rewrite_include(node); });
}

xml_node
rewrite_exclude(const xml_node& node) {
  assert(node.name == "exclude" && node.content && !node.tree());
  return xml_node{node};
}

xml_node
rewrite_excludes(const xml_node& node) {
  assert(!node.content);
  return rewrite_subnodes(node, [](const xml_node& node) { return rewrite_exclude(node); });
}

xml_node
rewrite_resource(const xml_node& node) {
  assert(node.name == "resource" && !node.content && node.tree() && node.tree()->node_cnt() <= 4);

  const vector<const xml_node*> resource_tree{node.tree()->find_in(vector<const char*>{"directory", "filtering", "includes", "excludes"})};
  assert(resource_tree.size() == 4 && resource_tree[0]);

  xml_node rw_resource{node.lineno, node.level, node.name, node.comment.get()};
  rw_resource.add_subnode(rewrite_artifact_id(*resource_tree[0]));
  add_nonempty_rewrite(rw_resource, resource_tree[1], [](const xml_node& node) { return rewrite_filtering(node); });
  add_nonempty_rewrite(rw_resource, resource_tree[2], [](const xml_node& node) { return rewrite_includes(node); });
  add_nonempty_rewrite(rw_resource, resource_tree[3], [](const xml_node& node) { return rewrite_excludes(node); });

  return rw_resource;
}

xml_node
rewrite_resources(const xml_node& node) {
  assert(!node.content && node.tree());
  return rewrite_subnodes(node, [](const xml_node& node) { return rewrite_resource(node); });
}

xml_node
rewrite_build(const xml_node& node) {
  assert(node.name == "build" && !node.content && node.tree() && node.tree()->node_cnt() <= 3);

  const vector<const xml_node*> build_tree{node.tree()->find_in({"pluginManagement", "plugins", "resources"})};
  assert(build_tree.size() == 3);

  xml_node rw_build{node.lineno, node.level, node.name, node.comment.get()};
  add_nonempty_rewrite(rw_build, build_tree[0], [](const xml_node& node) { return rewrite_plugin_management(node); });
  add_nonempty_rewrite(rw_build, build_tree[1], [](const xml_node& node) { return rewrite_plugins(node); });
  add_nonempty_rewrite(rw_build, build_tree[2], [](const xml_node& node) { return rewrite_resources(node); });

  return rw_build;
}

xml_node
rewrite_profile(const xml_node& node) {
  assert(node.name == "profile" && !node.content && node.tree() && node.tree()->node_cnt() <= 4);

  const vector<const xml_node*> profile_tree{node.tree()->find_in({"id", "properties", "activation", "build"})};
  assert(profile_tree.size() == 4 && profile_tree[0]);

  xml_node rw_profile{node.lineno, node.level, node.name, node.comment.get()};
  rw_profile.add_subnode(rewrite_id(*profile_tree[0]));
  add_nonempty_rewrite(rw_profile, profile_tree[1], [](const xml_node& node) { return rewrite_project_properties(node); });
  add_nonempty_rewrite(rw_profile, profile_tree[2], [](const xml_node& node) { return rewrite_activation(node); });
  add_nonempty_rewrite(rw_profile, profile_tree[3], [](const xml_node& node) { return rewrite_build(node); });

  return rw_profile;
}

xml_node
rewrite_profiles(const xml_node& node) {
  assert(!node.content);
  return rewrite_subnodes(node, [](const xml_node& node) { return rewrite_profile(node); });
}

xml_node
rewrite_active_profile(const xml_node& node) {
  assert(node.name == "activeProfile" && node.content && !node.tree());
  return xml_node{node};
}

xml_node
rewrite_active_profiles(const xml_node& node) {
  assert(!node.content && node.tree());
  return rewrite_sorted_subnodes(node, [](const xml_node& node) { return rewrite_active_profile(node); }, [](const xml_node& a, const xml_node& b) { return *a.content < *b.content; });
}

xml_node
rewrite_project(const xml_node& node) {
  assert(!node.content && node.tree() && node.tree()->node_cnt() <= 15);

  const vector<const xml_node*> project_tree{node.tree()->find_in({"modelVersion", "parent", "groupId", "artifactId", "version", "packaging", "properties", "scm", "distributionManagement", "dependencyManagement", "dependencies", "build", "modules", "profiles", "activeProfiles"})};
  assert(project_tree.size() == 15 && project_tree[0] && project_tree[3]);

  xml_node rw_project{node.lineno, node.level, node.name, node.comment.get()};
  rw_project.add_subnode(rewrite_model_version(*project_tree[0]));
  add_nonempty_rewrite(rw_project, project_tree[1], [](const xml_node& node) { return rewrite_parent(node); });
  add_nonempty_rewrite(rw_project, project_tree[2], [](const xml_node& node) { return rewrite_group_id(node); });
  rw_project.add_subnode(rewrite_artifact_id(*project_tree[3]));
  add_nonempty_rewrite(rw_project, project_tree[4], [](const xml_node& node) { return rewrite_version(node); });
  add_nonempty_rewrite(rw_project, project_tree[5], [](const xml_node& node) { return rewrite_packaging(node); });
  add_nonempty_rewrite(rw_project, project_tree[6], [](const xml_node& node) { return rewrite_project_properties(node); });
  add_nonempty_rewrite(rw_project, project_tree[7], [](const xml_node& node) { return rewrite_scm(node); });
  add_nonempty_rewrite(rw_project, project_tree[8], [](const xml_node& node) { return rewrite_distribution_management(node); });
  add_nonempty_rewrite(rw_project, project_tree[9], [](const xml_node& node) { return rewrite_dependency_management(node); });
  add_nonempty_rewrite(rw_project, project_tree[10], [](const xml_node& node) { return rewrite_dependencies(node); });
  add_nonempty_rewrite(rw_project, project_tree[11], [](const xml_node& node) { return rewrite_build(node); });
  add_nonempty_rewrite(rw_project, project_tree[12], [](const xml_node& node) { return rewrite_modules(node); });
  add_nonempty_rewrite(rw_project, project_tree[13], [](const xml_node& node) { return rewrite_profiles(node); });
  add_nonempty_rewrite(rw_project, project_tree[14], [](const xml_node& node) { return rewrite_active_profiles(node); });
  return rw_project;
}

xml_node
rewrite_pom(const xml_node* node) {
  assert(node);
  if (node->name != "project" || !node->tree())
    throw runtime_error{"root project node missing or empty"};
  return rewrite_project(*node);
}
}
