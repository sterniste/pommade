#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "rewrite_pom.h"
#include "xml_graph.h"

namespace pommade {
using namespace std;
using namespace std::placeholders;
using namespace xml_graph;

const function<pom_xml_node(const xml_node&, bool)>&
pom_rewriter::get_rw_fn(rw_key key) {
  const auto cit = rws_fn_map.find(key);
  if (cit != rws_fn_map.cend())
    return cit->second;
  pom_xml_node (pom_rewriter::*rw_f)(const xml_node&, bool){};
  switch (key) {
  case rw_model_version:
    rw_f = &pom_rewriter::rewrite_model_version_node;
    break;
  case rw_group_id:
    rw_f = &pom_rewriter::rewrite_group_id_node;
    break;
  case rw_artifact_id:
    rw_f = &pom_rewriter::rewrite_artifact_id_node;
    break;
  case rw_parent_version:
    rw_f = &pom_rewriter::rewrite_parent_version_node;
    break;
  case rw_relative_path:
    rw_f = &pom_rewriter::rewrite_relative_path_node;
    break;
  case rw_parent:
    rw_f = &pom_rewriter::rewrite_parent_node;
    break;
  case rw_version:
    rw_f = &pom_rewriter::rewrite_version_node;
    break;
  case rw_packaging:
    rw_f = &pom_rewriter::rewrite_packaging_node;
    break;
  case rw_project_property:
    rw_f = &pom_rewriter::rewrite_project_property_node;
    break;
  case rw_project_properties:
    rw_f = &pom_rewriter::rewrite_project_properties_node;
    break;
  case rw_scm_element:
    rw_f = &pom_rewriter::rewrite_scm_element_node;
    break;
  case rw_scm:
    rw_f = &pom_rewriter::rewrite_scm_node;
    break;
  case rw_distribution_management_element:
    rw_f = &pom_rewriter::rewrite_distribution_management_element_node;
    break;
  case rw_distribution_management:
    rw_f = &pom_rewriter::rewrite_distribution_management_node;
    break;
  case rw_exclusion:
    rw_f = &pom_rewriter::rewrite_exclusion_node;
    break;
  case rw_exclusions:
    rw_f = &pom_rewriter::rewrite_exclusions_node;
    break;
  case rw_scope:
    rw_f = &pom_rewriter::rewrite_scope_node;
    break;
  case rw_dependency:
    rw_f = &pom_rewriter::rewrite_dependency_node;
    break;
  case rw_dependencies:
    rw_f = &pom_rewriter::rewrite_dependencies_node;
    break;
  case rw_dependency_management:
    rw_f = &pom_rewriter::rewrite_dependency_management_node;
    break;
  case rw_module:
    rw_f = &pom_rewriter::rewrite_module_node;
    break;
  case rw_modules:
    rw_f = &pom_rewriter::rewrite_modules_node;
    break;
  case rw_id:
    rw_f = &pom_rewriter::rewrite_id_node;
    break;
  case rw_name:
    rw_f = &pom_rewriter::rewrite_name_node;
    break;
  case rw_value:
    rw_f = &pom_rewriter::rewrite_value_node;
    break;
  case rw_properties:
    rw_f = &pom_rewriter::rewrite_properties_node;
    break;
  case rw_active_by_default:
    rw_f = &pom_rewriter::rewrite_active_by_default_node;
    break;
  case rw_activation:
    rw_f = &pom_rewriter::rewrite_activation_node;
    break;
  case rw_configuration:
    rw_f = &pom_rewriter::rewrite_configuration_node;
    break;
  case rw_phase:
    rw_f = &pom_rewriter::rewrite_phase_node;
    break;
  case rw_goal:
    rw_f = &pom_rewriter::rewrite_goal_node;
    break;
  case rw_goals:
    rw_f = &pom_rewriter::rewrite_goals_node;
    break;
  case rw_execution:
    rw_f = &pom_rewriter::rewrite_execution_node;
    break;
  case rw_executions:
    rw_f = &pom_rewriter::rewrite_executions_node;
    break;
  case rw_plugin:
    rw_f = &pom_rewriter::rewrite_plugin_node;
    break;
  case rw_plugins:
    rw_f = &pom_rewriter::rewrite_plugins_node;
    break;
  case rw_plugin_management:
    rw_f = &pom_rewriter::rewrite_plugin_management_node;
    break;
  case rw_filtering:
    rw_f = &pom_rewriter::rewrite_filtering_node;
    break;
  case rw_include:
    rw_f = &pom_rewriter::rewrite_include_node;
    break;
  case rw_includes:
    rw_f = &pom_rewriter::rewrite_includes_node;
    break;
  case rw_exclude:
    rw_f = &pom_rewriter::rewrite_exclude_node;
    break;
  case rw_excludes:
    rw_f = &pom_rewriter::rewrite_excludes_node;
    break;
  case rw_resource:
    rw_f = &pom_rewriter::rewrite_resource_node;
    break;
  case rw_resources:
    rw_f = &pom_rewriter::rewrite_resources_node;
    break;
  case rw_build:
    rw_f = &pom_rewriter::rewrite_build_node;
    break;
  case rw_profile:
    rw_f = &pom_rewriter::rewrite_profile_node;
    break;
  case rw_profiles:
    rw_f = &pom_rewriter::rewrite_profiles_node;
    break;
  case rw_active_profile:
    rw_f = &pom_rewriter::rewrite_active_profile_node;
    break;
  case rw_active_profiles:
    rw_f = &pom_rewriter::rewrite_active_profiles_node;
    break;
  }
  assert(rw_f);
  auto insert{rws_fn_map.insert(pair<rw_key, function<pom_xml_node(const xml_node&, bool)>>{key, bind(rw_f, this, _1, _2)})};
  assert(insert.second);
  return insert.first->second;
}

const function<pom_xml_node(const xml_node&, bool)>&
pom_rewriter::get_rw_with_flag_fn(rw_with_flag_key key) {
  const auto cit = rws_with_flag_fn_map.find(key);
  if (cit != rws_with_flag_fn_map.cend())
    return cit->second;
  pom_xml_node (pom_rewriter::*rw_with_flag_f)(const xml_node&, bool, bool){};
  switch (key.i) {
  case rw_with_flag_property:
    rw_with_flag_f = &pom_rewriter::rewrite_property_node;
    break;
  }
  assert(rw_with_flag_f);
  auto insert{rws_with_flag_fn_map.insert(pair<rw_with_flag_key, function<pom_xml_node(const xml_node&, bool)>>{key, bind(rw_with_flag_f, this, _1, _2, key.flag)})};
  assert(insert.second);
  return insert.first->second;
}

const function<bool(const xml_node*, const xml_node*)>&
pom_rewriter::get_lt_fn(lt_key key) {
  const auto cit = lts_fn_map.find(key);
  if (cit != lts_fn_map.cend())
    return cit->second;
  bool (pom_rewriter::*lt_f)(const xml_node*, const xml_node*) const{};
  switch (key) {
  case lt_exclusion:
    lt_f = &pom_rewriter::lt_exclusion_nodes;
    break;
  case lt_dependency:
    lt_f = &pom_rewriter::lt_dependency_nodes;
    break;
  }
  assert(lt_f);
  auto insert{lts_fn_map.insert(pair<lt_key, function<bool(const xml_node*, const xml_node*)>>{key, bind(lt_f, this, _1, _2)})};
  assert(insert.second);
  return insert.first->second;
}

bool
pom_rewriter::add_nonempty_rewrite_node(pom_xml_node& node, bool gap_before, const xml_node* subnode, const function<pom_xml_node(const xml_node&, bool)>& rw_fn) {
  if (subnode) {
    pom_xml_node rw_subnode{rw_fn(*subnode, gap_before)};
    if (rw_subnode.get_content() || rw_subnode.tree()) {
      node.add_subnode(move(rw_subnode));
      return true;
    }
  }
  return false;
}

pom_xml_node
pom_rewriter::rewrite_subnodes(const xml_node& node, bool gap_before, bool gap_before_subnodes, const function<pom_xml_node(const xml_node&, bool)>& rw_fn) {
  pom_xml_node rw_node{node.lineno, node.level, node.name, node.comment.get(), gap_before};
  if (node.tree()) {
    for (auto cit = node.tree()->cbegin(); cit != node.tree()->cend(); ++cit)
      rw_node.add_subnode(rw_fn(*cit, cit == node.tree()->cbegin() ? false : gap_before_subnodes));
  }
  return rw_node;
}

pom_xml_node
pom_rewriter::rewrite_sort_subnodes(const xml_node& node, bool gap_before, bool gap_before_subnodes, const function<pom_xml_node(const xml_node&, bool)>& rw_fn, const function<bool(const xml_node*, const xml_node*)>& lt_fn) {
  pom_xml_node rw_node{node.lineno, node.level, node.name, node.comment.get(), gap_before};
  if (node.tree()) {
    vector<const xml_node*> subnodes;
    for (auto cit = node.tree()->cbegin(); cit != node.tree()->cend(); ++cit)
      subnodes.push_back(&*cit);
    sort(subnodes.begin(), subnodes.end(), lt_fn);
    vector<unique_ptr<const pom_xml_node>> pom_subnodes;
    bool gap_before_subnode{};
    for (auto nodep : subnodes) {
      pom_subnodes.push_back(unique_ptr<const pom_xml_node>{new pom_xml_node{rw_fn(*nodep, gap_before_subnode)}});
      gap_before_subnode = gap_before_subnodes;
    }
    rw_node.add_subnodes(move(pom_subnodes));
  }
  return rw_node;
}

pom_xml_node
pom_rewriter::rewrite_model_version_node(const xml_node& node, bool gap_before) {
  assert(node.get_content() && !node.tree());
  return pom_xml_node{node, gap_before};
}

pom_xml_node
pom_rewriter::rewrite_group_id_node(const xml_node& node, bool gap_before) {
  assert(node.get_content() && !node.tree());
  return pom_xml_node{node, gap_before};
}

pom_xml_node
pom_rewriter::rewrite_artifact_id_node(const xml_node& node, bool gap_before) {
  assert(node.get_content() && !node.tree());
  return pom_xml_node{node, gap_before};
}

pom_xml_node
pom_rewriter::rewrite_parent_version_node(const xml_node& node, bool gap_before) {
  assert(node.get_content() && !node.tree());
  return pom_xml_node{node, gap_before};
}

pom_xml_node
pom_rewriter::rewrite_relative_path_node(const xml_node& node, bool gap_before) {
  assert(node.get_content() && !node.tree());
  return pom_xml_node{node, gap_before};
}

pom_xml_node
pom_rewriter::rewrite_parent_node(const xml_node& node, bool gap_before) {
  assert(!node.get_content() && node.tree() && node.tree()->node_cnt() <= 4);

  const vector<const xml_node*> parent_tree{node.tree()->find_in({"groupId", "artifactId", "version", "relativePath"})};
  assert(parent_tree.size() == 4 && parent_tree[0] && parent_tree[1] && parent_tree[2]);

  pom_xml_node rw_parent{node.lineno, node.level, node.name, node.comment.get(), gap_before};
  rw_parent.add_subnode(rewrite_group_id_node(*parent_tree[0], false));
  rw_parent.add_subnode(rewrite_artifact_id_node(*parent_tree[1], false));
  rw_parent.add_subnode(rewrite_parent_version_node(*parent_tree[2], false));
  add_nonempty_rewrite_node(rw_parent, false, parent_tree[3], get_rw_fn(rw_relative_path));

  return rw_parent;
}

pom_xml_node
pom_rewriter::rewrite_version_node(const xml_node& node, bool gap_before) {
  assert(node.get_content() && !node.tree());
  return pom_xml_node{node, gap_before};
}

pom_xml_node
pom_rewriter::rewrite_packaging_node(const xml_node& node, bool gap_before) {
  assert(node.get_content() && !node.tree());
  return pom_xml_node{node, gap_before};
}

pom_xml_node
pom_rewriter::rewrite_project_property_node(const xml_node& node, bool gap_before) {
  assert(node.get_content() && !node.tree());
  return pom_xml_node{node, gap_before};
}

pom_xml_node
pom_rewriter::rewrite_project_properties_node(const xml_node& node, bool gap_before) {
  assert(!node.get_content());
  return rewrite_sort_subnodes(node, gap_before, false, get_rw_fn(rw_project_property), [](const xml_node* a, const xml_node* b) { return a->name < b->name; });
}

pom_xml_node
pom_rewriter::rewrite_scm_element_node(const xml_node& node, bool gap_before) {
  assert(node.get_content() && !node.tree());
  return pom_xml_node{node, gap_before};
}

pom_xml_node
pom_rewriter::rewrite_scm_node(const xml_node& node, bool gap_before) {
  assert(!node.get_content());
  return rewrite_sort_subnodes(node, gap_before, false, get_rw_fn(rw_scm_element), [](const xml_node* a, const xml_node* b) { return a->name < b->name; });
}

pom_xml_node
pom_rewriter::rewrite_distribution_management_element_node(const xml_node& node, bool gap_before) {
  assert(!node.get_content() && node.tree());
  return pom_xml_node{node, gap_before};
}

pom_xml_node
pom_rewriter::rewrite_distribution_management_node(const xml_node& node, bool gap_before) {
  assert(!node.get_content());
  return rewrite_sort_subnodes(node, gap_before, false, get_rw_fn(rw_distribution_management_element), [](const xml_node* a, const xml_node* b) { return a->name < b->name; });
}

pom_xml_node
pom_rewriter::rewrite_scope_node(const xml_node& node, bool gap_before) {
  assert(node.get_content() && !node.tree());
  return pom_xml_node{node, gap_before};
}
  
pom_xml_node
pom_rewriter::rewrite_exclusion_node(const xml_node& node, bool gap_before) {
  assert(node.name == "exclusion" && !node.get_content() && node.tree() && node.tree()->node_cnt() == 2);

  const vector<const xml_node*> exclusion_tree{node.tree()->find_in(vector<const char*>{"groupId", "artifactId"})};
  assert(exclusion_tree.size() == 2 && exclusion_tree[0] && exclusion_tree[1]);

  pom_xml_node rw_exclusion{node.lineno, node.level, node.name, node.comment.get(), gap_before};
  rw_exclusion.add_subnode(rewrite_group_id_node(*exclusion_tree[0], false));
  rw_exclusion.add_subnode(rewrite_artifact_id_node(*exclusion_tree[1], false));

  return rw_exclusion;
}

pom_xml_node
pom_rewriter::rewrite_exclusions_node(const xml_node& node, bool gap_before) {
  assert(node.name == "exclusions" && !node.get_content());
  return rewrite_sort_subnodes(node, gap_before, false, get_rw_fn(rw_exclusion), get_lt_fn(lt_exclusion));
}
  
pom_xml_node
pom_rewriter::rewrite_dependency_node(const xml_node& node, bool gap_before) {
  assert(node.name == "dependency" && !node.get_content() && node.tree() && node.tree()->node_cnt() <= 6);

  const vector<const xml_node*> dependency_tree{node.tree()->find_in(vector<const char*>{"groupId", "artifactId", "version", "packaging", "scope", "exclusions"})};
  assert(dependency_tree.size() == 6 && dependency_tree[0] && dependency_tree[1]);

  pom_xml_node rw_dependency{node.lineno, node.level, node.name, node.comment.get(), gap_before};
  rw_dependency.add_subnode(rewrite_group_id_node(*dependency_tree[0], false));
  rw_dependency.add_subnode(rewrite_artifact_id_node(*dependency_tree[1], false));
  add_nonempty_rewrite_node(rw_dependency, false, dependency_tree[2], get_rw_fn(rw_version));
  add_nonempty_rewrite_node(rw_dependency, false, dependency_tree[3], get_rw_fn(rw_packaging));
  add_nonempty_rewrite_node(rw_dependency, false, dependency_tree[4], get_rw_fn(rw_scope));
  add_nonempty_rewrite_node(rw_dependency, false, dependency_tree[5], get_rw_fn(rw_exclusions));

  return rw_dependency;
}

pom_xml_node
pom_rewriter::rewrite_dependencies_node(const xml_node& node, bool gap_before) {
  assert(node.name == "dependencies" && !node.get_content());
  return rewrite_sort_subnodes(node, gap_before, true, get_rw_fn(rw_dependency), get_lt_fn(lt_dependency));
}

pom_xml_node
pom_rewriter::rewrite_dependency_management_node(const xml_node& node, bool gap_before) {
  assert(!node.get_content());

  pom_xml_node rw_dependency_management{node.lineno, node.level, node.name, node.comment.get(), gap_before};
  if (!node.tree())
    return rw_dependency_management;

  rw_dependency_management.add_subnode(rewrite_dependencies_node(*node.tree()->cbegin(), false));

  return rw_dependency_management;
}

pom_xml_node
pom_rewriter::rewrite_module_node(const xml_node& node, bool gap_before) {
  assert(node.name == "module" && node.get_content());
  return pom_xml_node{node, gap_before};
}

pom_xml_node
pom_rewriter::rewrite_modules_node(const xml_node& node, bool gap_before) {
  assert(!node.get_content() && node.tree());
  return rewrite_subnodes(node, gap_before, false, get_rw_fn(rw_module));
}

pom_xml_node
pom_rewriter::rewrite_id_node(const xml_node& node, bool gap_before) {
  assert(node.get_content() && !node.tree());
  return pom_xml_node{node, gap_before};
}

pom_xml_node
pom_rewriter::rewrite_name_node(const xml_node& node, bool gap_before) {
  assert(node.get_content() && !node.tree());
  return pom_xml_node{node, gap_before};
}

pom_xml_node
pom_rewriter::rewrite_value_node(const xml_node& node, bool gap_before) {
  assert(node.get_content() && !node.tree());
  return pom_xml_node{node, gap_before};
}

pom_xml_node
pom_rewriter::rewrite_property_node(const xml_node& node, bool gap_before, bool unvalued_ok) {
  assert(node.name == "property" && !node.get_content() && node.tree());
  if (unvalued_ok)
    assert(node.tree()->node_cnt() <= 2);
  else
    assert(node.tree()->node_cnt() == 2);

  const vector<const xml_node*> property_tree{node.tree()->find_in(vector<const char*>{"name", "value"})};
  assert(property_tree.size() == 2 && property_tree[0]);
  if (!unvalued_ok)
    assert(property_tree[1]);

  pom_xml_node rw_property{node.lineno, node.level, node.name, node.comment.get(), gap_before};
  rw_property.add_subnode(rewrite_name_node(*property_tree[0], false));
  if (!unvalued_ok)
    rw_property.add_subnode(rewrite_value_node(*property_tree[1], false));
  else
    add_nonempty_rewrite_node(rw_property, false, property_tree[1], get_rw_fn(rw_value));

  return rw_property;
}

pom_xml_node
pom_rewriter::rewrite_properties_node(const xml_node& node, bool gap_before) {
  assert(!node.get_content() && node.tree());
  return rewrite_sort_subnodes(node, gap_before, false, get_rw_with_flag_fn(rw_with_flag_key{rw_with_flag_property, false}), [](const xml_node* a, const xml_node* b) {
    auto a_cit = a->tree()->cbegin(), b_cit = b->tree()->cbegin();
    return *a_cit->get_content() < *b_cit->get_content();
  });
}

pom_xml_node
pom_rewriter::rewrite_active_by_default_node(const xml_node& node, bool gap_before) {
  assert(node.get_content() && !node.tree());
  return pom_xml_node{node, gap_before};
}

pom_xml_node
pom_rewriter::rewrite_activation_node(const xml_node& node, bool gap_before) {
  assert(!node.get_content() && node.tree());

  const vector<const xml_node*> activation_tree{node.tree()->find_in(vector<const char*>{"activeByDefault", "property"})};
  assert(activation_tree.size() >= 2 && activation_tree[1]);

  pom_xml_node rw_activation{node.lineno, node.level, node.name, node.comment.get(), gap_before};
  add_nonempty_rewrite_node(rw_activation, false, activation_tree[0], get_rw_fn(rw_active_by_default));
  for (auto i = 1U; i < activation_tree.size(); ++i)
    rw_activation.add_subnode(rewrite_property_node(*activation_tree[i], false));

  return rw_activation;
}

pom_xml_node
pom_rewriter::rewrite_configuration_node(const xml_node& node, bool gap_before) {
  assert(!node.get_content() && node.tree());
  return rewrite_sort_subnodes(node, gap_before, false, (node.name == "properties" ? get_rw_fn(rw_properties) : copy_node_fn), [](const xml_node* a, const xml_node* b) { return a->name == "properties" || a->name < b->name; });
}

pom_xml_node
pom_rewriter::rewrite_phase_node(const xml_node& node, bool gap_before) {
  assert(node.get_content() && !node.tree());
  return pom_xml_node{node, gap_before};
}

pom_xml_node
pom_rewriter::rewrite_goal_node(const xml_node& node, bool gap_before) {
  assert(node.name == "goal" && node.get_content() && !node.tree());
  return pom_xml_node{node, gap_before};
}

pom_xml_node
pom_rewriter::rewrite_goals_node(const xml_node& node, bool gap_before) {
  assert(!node.get_content() && node.tree());
  return rewrite_subnodes(node, gap_before, false, get_rw_fn(rw_goal));
}

pom_xml_node
pom_rewriter::rewrite_execution_node(const xml_node& node, bool gap_before) {
  assert(node.name == "execution" && !node.get_content() && node.tree() && node.tree()->node_cnt() <= 4);

  const vector<const xml_node*> execution_tree{node.tree()->find_in(vector<const char*>{"id", "phase", "goals", "configuration"})};
  assert(execution_tree.size() == 4 && execution_tree[2]);

  pom_xml_node rw_execution{node.lineno, node.level, node.name, node.comment.get(), gap_before};
  add_nonempty_rewrite_node(rw_execution, false, execution_tree[0], get_rw_fn(rw_id));
  add_nonempty_rewrite_node(rw_execution, false, execution_tree[1], get_rw_fn(rw_phase));
  rw_execution.add_subnode(rewrite_goals_node(*execution_tree[2], false));
  add_nonempty_rewrite_node(rw_execution, false, execution_tree[3], get_rw_fn(rw_configuration));

  return rw_execution;
}

pom_xml_node
pom_rewriter::rewrite_executions_node(const xml_node& node, bool gap_before) {
  assert(!node.get_content() && node.tree());
  return rewrite_subnodes(node, gap_before, false, get_rw_fn(rw_execution));
}

pom_xml_node
pom_rewriter::rewrite_plugin_node(const xml_node& node, bool gap_before) {
  assert(node.name == "plugin" && !node.get_content() && node.tree() && node.tree()->node_cnt() <= 5);

  const vector<const xml_node*> plugin_tree{node.tree()->find_in(vector<const char*>{"groupId", "artifactId", "version", "configuration", "executions"})};
  assert(plugin_tree.size() == 5 && plugin_tree[1]);

  pom_xml_node rw_plugin{node.lineno, node.level, node.name, node.comment.get(), gap_before};
  add_nonempty_rewrite_node(rw_plugin, false, plugin_tree[0], get_rw_fn(rw_group_id));
  rw_plugin.add_subnode(rewrite_artifact_id_node(*plugin_tree[1], false));
  add_nonempty_rewrite_node(rw_plugin, false, plugin_tree[2], get_rw_fn(rw_version));
  add_nonempty_rewrite_node(rw_plugin, false, plugin_tree[3], get_rw_fn(rw_configuration));
  add_nonempty_rewrite_node(rw_plugin, false, plugin_tree[4], get_rw_fn(rw_executions));

  return rw_plugin;
}

pom_xml_node
pom_rewriter::rewrite_plugins_node(const xml_node& node, bool gap_before) {
  assert(node.name == "plugins" && !node.get_content());
  return rewrite_subnodes(node, gap_before, true, get_rw_fn(rw_plugin));
}

pom_xml_node
pom_rewriter::rewrite_plugin_management_node(const xml_node& node, bool gap_before) {
  assert(!node.get_content() && node.tree() && node.tree()->node_cnt() == 1);
  return rewrite_subnodes(node, gap_before, true, get_rw_fn(rw_plugins));
}

pom_xml_node
pom_rewriter::rewrite_filtering_node(const xml_node& node, bool gap_before) {
  assert(node.get_content() && !node.tree());
  return pom_xml_node{node, gap_before};
}

pom_xml_node
pom_rewriter::rewrite_include_node(const xml_node& node, bool gap_before) {
  assert(node.name == "include" && node.get_content() && !node.tree());
  return pom_xml_node{node, gap_before};
}

pom_xml_node
pom_rewriter::rewrite_includes_node(const xml_node& node, bool gap_before) {
  assert(!node.get_content());
  return rewrite_subnodes(node, gap_before, false, get_rw_fn(rw_include));
}

pom_xml_node
pom_rewriter::rewrite_exclude_node(const xml_node& node, bool gap_before) {
  assert(node.name == "exclude" && node.get_content() && !node.tree());
  return pom_xml_node{node, gap_before};
}

pom_xml_node
pom_rewriter::rewrite_excludes_node(const xml_node& node, bool gap_before) {
  assert(!node.get_content());
  return rewrite_subnodes(node, gap_before, false, get_rw_fn(rw_exclude));
}

pom_xml_node
pom_rewriter::rewrite_resource_node(const xml_node& node, bool gap_before) {
  assert(node.name == "resource" && !node.get_content() && node.tree() && node.tree()->node_cnt() <= 4);

  const vector<const xml_node*> resource_tree{node.tree()->find_in(vector<const char*>{"directory", "filtering", "includes", "excludes"})};
  assert(resource_tree.size() == 4 && resource_tree[0]);

  pom_xml_node rw_resource{node.lineno, node.level, node.name, node.comment.get(), gap_before};
  rw_resource.add_subnode(rewrite_artifact_id_node(*resource_tree[0], false));
  add_nonempty_rewrite_node(rw_resource, false, resource_tree[1], get_rw_fn(rw_filtering));
  add_nonempty_rewrite_node(rw_resource, false, resource_tree[2], get_rw_fn(rw_includes));
  add_nonempty_rewrite_node(rw_resource, false, resource_tree[3], get_rw_fn(rw_excludes));

  return rw_resource;
}

pom_xml_node
pom_rewriter::rewrite_resources_node(const xml_node& node, bool gap_before) {
  assert(!node.get_content() && node.tree());
  return rewrite_subnodes(node, gap_before, false, get_rw_fn(rw_resource));
}

pom_xml_node
pom_rewriter::rewrite_build_node(const xml_node& node, bool gap_before) {
  assert(node.name == "build" && !node.get_content() && node.tree() && node.tree()->node_cnt() <= 3);

  const vector<const xml_node*> build_tree{node.tree()->find_in({"pluginManagement", "plugins", "resources"})};
  assert(build_tree.size() == 3);

  pom_xml_node rw_build{node.lineno, node.level, node.name, node.comment.get(), gap_before};
  const bool has_plugin_management = add_nonempty_rewrite_node(rw_build, false, build_tree[0], get_rw_fn(rw_plugin_management));
  add_nonempty_rewrite_node(rw_build, has_plugin_management, build_tree[1], get_rw_fn(rw_plugins));
  add_nonempty_rewrite_node(rw_build, true, build_tree[2], get_rw_fn(rw_resources));

  return rw_build;
}

pom_xml_node
pom_rewriter::rewrite_profile_node(const xml_node& node, bool gap_before) {
  assert(node.name == "profile" && !node.get_content() && node.tree() && node.tree()->node_cnt() <= 4);

  const vector<const xml_node*> profile_tree{node.tree()->find_in({"id", "properties", "activation", "build"})};
  assert(profile_tree.size() == 4 && profile_tree[0]);

  pom_xml_node rw_profile{node.lineno, node.level, node.name, node.comment.get(), gap_before};
  rw_profile.add_subnode(rewrite_id_node(*profile_tree[0], false));
  add_nonempty_rewrite_node(rw_profile, false, profile_tree[1], get_rw_fn(rw_project_properties));
  add_nonempty_rewrite_node(rw_profile, false, profile_tree[2], get_rw_fn(rw_activation));
  add_nonempty_rewrite_node(rw_profile, false, profile_tree[3], get_rw_fn(rw_build));

  return rw_profile;
}

pom_xml_node
pom_rewriter::rewrite_profiles_node(const xml_node& node, bool gap_before) {
  assert(!node.get_content());
  return rewrite_subnodes(node, gap_before, true, get_rw_fn(rw_profile));
}

pom_xml_node
pom_rewriter::rewrite_active_profile_node(const xml_node& node, bool gap_before) {
  assert(node.name == "activeProfile" && node.get_content() && !node.tree());
  return pom_xml_node{node, gap_before};
}

pom_xml_node
pom_rewriter::rewrite_active_profiles_node(const xml_node& node, bool gap_before) {
  assert(!node.get_content() && node.tree());
  return rewrite_sort_subnodes(node, gap_before, false, get_rw_fn(rw_active_profile), [](const xml_node* a, const xml_node* b) { return *a->get_content() < *b->get_content(); });
}

pom_xml_node
pom_rewriter::rewrite_project_node(const xml_node& node) {
  assert(!node.get_content() && node.tree() && node.tree()->node_cnt() <= 15);

  const vector<const xml_node*> project_tree{node.tree()->find_in({"modelVersion", "parent", "groupId", "artifactId", "version", "packaging", "properties", "scm", "distributionManagement", "dependencyManagement", "dependencies", "build", "modules", "profiles", "activeProfiles"})};
  assert(project_tree.size() == 15 && project_tree[0] && project_tree[3]);

  pom_xml_node rw_project{node.lineno, node.level, node.name, node.comment.get(), false};
  rw_project.add_subnode(rewrite_model_version_node(*project_tree[0], false));
  has_parent = add_nonempty_rewrite_node(rw_project, true, project_tree[1], get_rw_fn(rw_parent));
  add_nonempty_rewrite_node(rw_project, false, project_tree[2], get_rw_fn(rw_group_id));
  rw_project.add_subnode(rewrite_artifact_id_node(*project_tree[3], false));
  add_nonempty_rewrite_node(rw_project, false, project_tree[4], get_rw_fn(rw_version));
  add_nonempty_rewrite_node(rw_project, false, project_tree[5], get_rw_fn(rw_packaging));
  add_nonempty_rewrite_node(rw_project, true, project_tree[6], get_rw_fn(rw_project_properties));
  add_nonempty_rewrite_node(rw_project, true, project_tree[7], get_rw_fn(rw_scm));
  add_nonempty_rewrite_node(rw_project, true, project_tree[8], get_rw_fn(rw_distribution_management));
  add_nonempty_rewrite_node(rw_project, true, project_tree[9], get_rw_fn(rw_dependency_management));
  add_nonempty_rewrite_node(rw_project, true, project_tree[10], get_rw_fn(rw_dependencies));
  add_nonempty_rewrite_node(rw_project, true, project_tree[11], get_rw_fn(rw_build));
  add_nonempty_rewrite_node(rw_project, true, project_tree[12], get_rw_fn(rw_modules));
  add_nonempty_rewrite_node(rw_project, true, project_tree[13], get_rw_fn(rw_profiles));
  add_nonempty_rewrite_node(rw_project, true, project_tree[14], get_rw_fn(rw_active_profiles));

  return rw_project;
}

bool
pom_rewriter::lt_exclusion_nodes(const xml_node* a, const xml_node* b) const {
  auto a_cit = a->tree()->cbegin(), b_cit = b->tree()->cbegin();
  if (*a_cit->get_content() < *b_cit->get_content())
    return true;
  if (*a_cit->get_content() == *b_cit->get_content())
    return *(++a_cit)->get_content() < *(++b_cit)->get_content();
  return false;
}

bool
pom_rewriter::lt_dependency_nodes(const xml_node* a, const xml_node* b) const {
  auto a_cit = a->tree()->cbegin(), b_cit = b->tree()->cbegin();
  if (*a_cit->get_content() < *b_cit->get_content())
    return true;
  if (*a_cit->get_content() == *b_cit->get_content())
    return *(++a_cit)->get_content() < *(++b_cit)->get_content();
  return false;
}

pom_xml_node
pom_rewriter::rewrite_pom(const xml_node* node) {
  assert(node);
  if (node->name != "project" || !node->tree())
    throw runtime_error{"root project node missing or empty"};
  return rewrite_project_node(*node);
}
}
