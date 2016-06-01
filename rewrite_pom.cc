#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "rewrite_pom.h"
#include "xml_graph.h"
#include "xml_parser.h"

namespace pommade {
using namespace std;
using namespace std::placeholders;
using namespace xml_graph;

function<xml_node(pom_rewriter*, const xml_node&)>
pom_rewriter::rewrite_for(xml_node (pom_rewriter::*rewrite)(const xml_node&)) {
  return bind(rewrite, _1, _2);
}

function<xml_node(pom_rewriter*, const xml_node&)>
pom_rewriter::rewrite_with_flag_for(xml_node (pom_rewriter::*rewrite_with_flag)(const xml_node&, bool flag), bool flag) {
  return bind(rewrite_with_flag, _1, _2, flag);
}

void
pom_rewriter::add_nonempty_rewrite(xml_node& node, const xml_node* subnode, xml_node (*rewrite)(pom_rewriter* self, const xml_node&)) {
  if (subnode) {
    xml_node rw_subnode{rewrite(this, *subnode)};
    if (rw_subnode.content || rw_subnode.tree())
      node.add_subnode(move(rw_subnode));
  }
}

xml_node
pom_rewriter::rewrite_subnodes(const xml_node& node, xml_node (*rewrite)(pom_rewriter* self, const xml_node&)) {
  xml_node rw_node{node.lineno, node.level, node.name, node.comment.get()};
  if (node.tree()) {
    for (auto cit = node.tree()->cbegin(); cit != node.tree()->cend(); ++cit)
      rw_node.add_subnode(rewrite(this, *cit));
  }
  return rw_node;
}

xml_node
pom_rewriter::rewrite_sort_subnodes(const xml_node& node, xml_node (*rewrite)(pom_rewriter* self, const xml_node&), bool (*compare)(const xml_node& a, const xml_node& b)) {
  xml_node rw_node{node.lineno, node.level, node.name, node.comment.get()};
  if (node.tree()) {
    vector<xml_node> subnodes;
    for (auto cit = node.tree()->cbegin(); cit != node.tree()->cend(); ++cit)
      subnodes.push_back(rewrite(this, *cit));
    sort(subnodes.begin(), subnodes.end(), compare);
    for (auto&& subnode : subnodes)
      rw_node.add_subnode(move(subnode));
  }
  return rw_node;
}

xml_node
pom_rewriter::rewrite_model_version(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
pom_rewriter::rewrite_group_id(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
pom_rewriter::rewrite_artifact_id(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
pom_rewriter::rewrite_parent_version(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
pom_rewriter::rewrite_relative_path(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
pom_rewriter::rewrite_parent(const xml_node& node) {
  assert(!node.content && node.tree() && node.tree()->node_cnt() <= 4);

  const vector<const xml_node*> parent_tree{node.tree()->find_in({"groupId", "artifactId", "version", "relativePath"})};
  assert(parent_tree.size() == 4 && parent_tree[0] && parent_tree[1] && parent_tree[2]);

  xml_node rw_parent{node.lineno, node.level, node.name, node.comment.get()};
  rw_parent.add_subnode(rewrite_group_id(*parent_tree[0]));
  rw_parent.add_subnode(rewrite_artifact_id(*parent_tree[1]));
  rw_parent.add_subnode(rewrite_parent_version(*parent_tree[2]));
  add_nonempty_rewrite(rw_parent, parent_tree[3], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_relative_path)(self, node); });

  return rw_parent;
}

xml_node
pom_rewriter::rewrite_version(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
pom_rewriter::rewrite_packaging(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
pom_rewriter::rewrite_project_property(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
pom_rewriter::rewrite_project_properties(const xml_node& node) {
  assert(!node.content);
  return rewrite_sort_subnodes(node, [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_project_property)(self, node); }, [](const xml_node& a, const xml_node& b) { return a.name < b.name; });
}

xml_node
pom_rewriter::rewrite_scm_element(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
pom_rewriter::rewrite_scm(const xml_node& node) {
  assert(!node.content);
  return rewrite_sort_subnodes(node, [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_scm_element)(self, node); }, [](const xml_node& a, const xml_node& b) { return a.name < b.name; });
}

xml_node
pom_rewriter::rewrite_distribution_management_element(const xml_node& node) {
  assert(!node.content && node.tree());
  return xml_node{node};
}

xml_node
pom_rewriter::rewrite_distribution_management(const xml_node& node) {
  assert(!node.content);
  return rewrite_sort_subnodes(node, [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_distribution_management_element)(self, node); }, [](const xml_node& a, const xml_node& b) { return a.name < b.name; });
}

xml_node
pom_rewriter::rewrite_scope(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
pom_rewriter::rewrite_dependency(const xml_node& node) {
  assert(node.name == "dependency" && !node.content && node.tree() && node.tree()->node_cnt() <= 5);

  const vector<const xml_node*> dependency_tree{node.tree()->find_in(vector<const char*>{"groupId", "artifactId", "version", "packaging", "scope"})};
  assert(dependency_tree.size() == 5 && dependency_tree[0] && dependency_tree[1]);

  xml_node rw_dependency{node.lineno, node.level, node.name, node.comment.get()};
  rw_dependency.add_subnode(rewrite_group_id(*dependency_tree[0]));
  rw_dependency.add_subnode(rewrite_artifact_id(*dependency_tree[1]));
  add_nonempty_rewrite(rw_dependency, dependency_tree[2], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_version)(self, node); });
  add_nonempty_rewrite(rw_dependency, dependency_tree[3], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_packaging)(self, node); });
  add_nonempty_rewrite(rw_dependency, dependency_tree[4], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_scope)(self, node); });

  return rw_dependency;
}

xml_node
pom_rewriter::rewrite_dependencies(const xml_node& node) {
  assert(node.name == "dependencies" && !node.content);
  return rewrite_sort_subnodes(node, [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_dependency)(self, node); },
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
pom_rewriter::rewrite_dependency_management(const xml_node& node) {
  assert(!node.content);

  xml_node rw_dependency_management{node.lineno, node.level, node.name, node.comment.get()};
  if (!node.tree())
    return rw_dependency_management;

  rw_dependency_management.add_subnode(rewrite_dependencies(*node.tree()->cbegin()));

  return rw_dependency_management;
}

xml_node
pom_rewriter::rewrite_module(const xml_node& node) {
  assert(node.name == "module" && node.content);
  return xml_node{node};
}

xml_node
pom_rewriter::rewrite_modules(const xml_node& node) {
  assert(!node.content && node.tree());
  return rewrite_subnodes(node, [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_module)(self, node); });
}

xml_node
pom_rewriter::rewrite_id(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
pom_rewriter::rewrite_name(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
pom_rewriter::rewrite_value(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
pom_rewriter::rewrite_property(const xml_node& node, bool unvalued_ok) {
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
    add_nonempty_rewrite(rw_property, property_tree[1], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_value)(self, node); });

  return rw_property;
}

xml_node
pom_rewriter::rewrite_properties(const xml_node& node) {
  assert(!node.content && node.tree());
  return rewrite_sort_subnodes(node, [](pom_rewriter* self, const xml_node& node) { return rewrite_with_flag_for(&pom_rewriter::rewrite_property, false)(self, node); },
                               [](const xml_node& a, const xml_node& b) {
                                 auto a_cit = a.tree()->cbegin(), b_cit = b.tree()->cbegin();
                                 return *a_cit->content < *b_cit->content;
                               });
}

xml_node
pom_rewriter::rewrite_activation(const xml_node& node) {
  assert(!node.content && node.tree());
  return rewrite_sort_subnodes(node, [](pom_rewriter* self, const xml_node& node) { return node.name == "property" ? rewrite_with_flag_for(&pom_rewriter::rewrite_property, true)(self, node) : xml_node{node}; }, [](const xml_node& a, const xml_node& b) { return a.name < b.name; });
}

xml_node
pom_rewriter::rewrite_configuration(const xml_node& node) {
  assert(!node.content && node.tree());
  return rewrite_sort_subnodes(node, [](pom_rewriter* self, const xml_node& node) { return node.name == "properties" ? rewrite_for(&pom_rewriter::rewrite_properties)(self, node) : xml_node{node}; }, [](const xml_node& a, const xml_node& b) { return a.name == "properties" || a.name < b.name; });
}

xml_node
pom_rewriter::rewrite_phase(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
pom_rewriter::rewrite_goal(const xml_node& node) {
  assert(node.name == "goal" && node.content && !node.tree());
  return xml_node{node};
}

xml_node
pom_rewriter::rewrite_goals(const xml_node& node) {
  assert(!node.content && node.tree());
  return rewrite_subnodes(node, [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_goal)(self, node); });
}

xml_node
pom_rewriter::rewrite_execution(const xml_node& node) {
  assert(node.name == "execution" && !node.content && node.tree() && node.tree()->node_cnt() <= 4);

  const vector<const xml_node*> execution_tree{node.tree()->find_in(vector<const char*>{"id", "phase", "goals", "configuration"})};
  assert(execution_tree.size() == 4 && execution_tree[2]);

  xml_node rw_execution{node.lineno, node.level, node.name, node.comment.get()};
  add_nonempty_rewrite(rw_execution, execution_tree[0], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_id)(self, node); });
  add_nonempty_rewrite(rw_execution, execution_tree[1], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_phase)(self, node); });
  rw_execution.add_subnode(rewrite_goals(*execution_tree[2]));
  add_nonempty_rewrite(rw_execution, execution_tree[3], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_configuration)(self, node); });

  return rw_execution;
}

xml_node
pom_rewriter::rewrite_executions(const xml_node& node) {
  assert(!node.content && node.tree());
  return rewrite_subnodes(node, [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_execution)(self, node); });
}

xml_node
pom_rewriter::rewrite_plugin(const xml_node& node) {
  assert(node.name == "plugin" && !node.content && node.tree() && node.tree()->node_cnt() <= 5);

  const vector<const xml_node*> plugin_tree{node.tree()->find_in(vector<const char*>{"groupId", "artifactId", "version", "configuration", "executions"})};
  assert(plugin_tree.size() == 5 && plugin_tree[1]);

  xml_node rw_plugin{node.lineno, node.level, node.name, node.comment.get()};
  add_nonempty_rewrite(rw_plugin, plugin_tree[0], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_group_id)(self, node); });
  rw_plugin.add_subnode(rewrite_artifact_id(*plugin_tree[1]));
  add_nonempty_rewrite(rw_plugin, plugin_tree[2], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_version)(self, node); });
  add_nonempty_rewrite(rw_plugin, plugin_tree[3], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_configuration)(self, node); });
  add_nonempty_rewrite(rw_plugin, plugin_tree[4], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_executions)(self, node); });

  return rw_plugin;
}

xml_node
pom_rewriter::rewrite_plugins(const xml_node& node) {
  assert(node.name == "plugins" && !node.content);
  return rewrite_subnodes(node, [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_plugin)(self, node); });
}

xml_node
pom_rewriter::rewrite_plugin_management(const xml_node& node) {
  assert(!node.content && node.tree() && node.tree()->node_cnt() == 1);
  return rewrite_subnodes(node, [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_plugins)(self, node); });
}

xml_node
pom_rewriter::rewrite_filtering(const xml_node& node) {
  assert(node.content && !node.tree());
  return xml_node{node};
}

xml_node
pom_rewriter::rewrite_include(const xml_node& node) {
  assert(node.name == "include" && node.content && !node.tree());
  return xml_node{node};
}

xml_node
pom_rewriter::rewrite_includes(const xml_node& node) {
  assert(!node.content);
  return rewrite_subnodes(node, [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_include)(self, node); });
}

xml_node
pom_rewriter::rewrite_exclude(const xml_node& node) {
  assert(node.name == "exclude" && node.content && !node.tree());
  return xml_node{node};
}

xml_node
pom_rewriter::rewrite_excludes(const xml_node& node) {
  assert(!node.content);
  return rewrite_subnodes(node, [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_exclude)(self, node); });
}

xml_node
pom_rewriter::rewrite_resource(const xml_node& node) {
  assert(node.name == "resource" && !node.content && node.tree() && node.tree()->node_cnt() <= 4);

  const vector<const xml_node*> resource_tree{node.tree()->find_in(vector<const char*>{"directory", "filtering", "includes", "excludes"})};
  assert(resource_tree.size() == 4 && resource_tree[0]);

  xml_node rw_resource{node.lineno, node.level, node.name, node.comment.get()};
  rw_resource.add_subnode(rewrite_artifact_id(*resource_tree[0]));
  add_nonempty_rewrite(rw_resource, resource_tree[1], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_filtering)(self, node); });
  add_nonempty_rewrite(rw_resource, resource_tree[2], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_includes)(self, node); });
  add_nonempty_rewrite(rw_resource, resource_tree[3], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_excludes)(self, node); });

  return rw_resource;
}

xml_node
pom_rewriter::rewrite_resources(const xml_node& node) {
  assert(!node.content && node.tree());
  return rewrite_subnodes(node, [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_resource)(self, node); });
}

xml_node
pom_rewriter::rewrite_build(const xml_node& node) {
  assert(node.name == "build" && !node.content && node.tree() && node.tree()->node_cnt() <= 3);

  const vector<const xml_node*> build_tree{node.tree()->find_in({"pluginManagement", "plugins", "resources"})};
  assert(build_tree.size() == 3);

  xml_node rw_build{node.lineno, node.level, node.name, node.comment.get()};
  add_nonempty_rewrite(rw_build, build_tree[0], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_plugin_management)(self, node); });
  add_nonempty_rewrite(rw_build, build_tree[1], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_plugins)(self, node); });
  add_nonempty_rewrite(rw_build, build_tree[2], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_resources)(self, node); });

  return rw_build;
}

xml_node
pom_rewriter::rewrite_profile(const xml_node& node) {
  assert(node.name == "profile" && !node.content && node.tree() && node.tree()->node_cnt() <= 4);

  const vector<const xml_node*> profile_tree{node.tree()->find_in({"id", "properties", "activation", "build"})};
  assert(profile_tree.size() == 4 && profile_tree[0]);

  xml_node rw_profile{node.lineno, node.level, node.name, node.comment.get()};
  rw_profile.add_subnode(rewrite_id(*profile_tree[0]));
  add_nonempty_rewrite(rw_profile, profile_tree[1], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_project_properties)(self, node); });
  add_nonempty_rewrite(rw_profile, profile_tree[2], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_activation)(self, node); });
  add_nonempty_rewrite(rw_profile, profile_tree[3], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_build)(self, node); });

  return rw_profile;
}

xml_node
pom_rewriter::rewrite_profiles(const xml_node& node) {
  assert(!node.content);
  return rewrite_subnodes(node, [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_profile)(self, node); });
}

xml_node
pom_rewriter::rewrite_active_profile(const xml_node& node) {
  assert(node.name == "activeProfile" && node.content && !node.tree());
  return xml_node{node};
}

xml_node
pom_rewriter::rewrite_active_profiles(const xml_node& node) {
  assert(!node.content && node.tree());
  return rewrite_sort_subnodes(node, [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_active_profile)(self, node); }, [](const xml_node& a, const xml_node& b) { return *a.content < *b.content; });
}

xml_node
pom_rewriter::rewrite_project(const xml_node& node) {
  assert(!node.content && node.tree() && node.tree()->node_cnt() <= 15);

  const vector<const xml_node*> project_tree{node.tree()->find_in({"modelVersion", "parent", "groupId", "artifactId", "version", "packaging", "properties", "scm", "distributionManagement", "dependencyManagement", "dependencies", "build", "modules", "profiles", "activeProfiles"})};
  assert(project_tree.size() == 15 && project_tree[0] && project_tree[3]);

  xml_node rw_project{node.lineno, node.level, node.name, node.comment.get()};
  rw_project.add_subnode(rewrite_model_version(*project_tree[0]));
  add_nonempty_rewrite(rw_project, project_tree[1], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_parent)(self, node); });
  has_parent = (project_tree[1] != nullptr);
  add_nonempty_rewrite(rw_project, project_tree[2], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_group_id)(self, node); });
  rw_project.add_subnode(rewrite_artifact_id(*project_tree[3]));
  add_nonempty_rewrite(rw_project, project_tree[4], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_version)(self, node); });
  add_nonempty_rewrite(rw_project, project_tree[5], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_packaging)(self, node); });
  add_nonempty_rewrite(rw_project, project_tree[6], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_project_properties)(self, node); });
  add_nonempty_rewrite(rw_project, project_tree[7], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_scm)(self, node); });
  add_nonempty_rewrite(rw_project, project_tree[8], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_distribution_management)(self, node); });
  add_nonempty_rewrite(rw_project, project_tree[9], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_dependency_management)(self, node); });
  add_nonempty_rewrite(rw_project, project_tree[10], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_dependencies)(self, node); });
  add_nonempty_rewrite(rw_project, project_tree[11], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_build)(self, node); });
  add_nonempty_rewrite(rw_project, project_tree[12], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_modules)(self, node); });
  add_nonempty_rewrite(rw_project, project_tree[13], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_profiles)(self, node); });
  add_nonempty_rewrite(rw_project, project_tree[14], [](pom_rewriter* self, const xml_node& node) { return rewrite_for(&pom_rewriter::rewrite_active_profiles)(self, node); });

  return rw_project;
}

xml_node
pom_rewriter::rewrite_pom(const xml_node* node) {
  assert(node);
  if (node->name != "project" || !node->tree())
    throw runtime_error{"root project node missing or empty"};
  return rewrite_project(*node);
}
}
