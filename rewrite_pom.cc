#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "rewrite_pom.h"
#include "xml_graph.h"

namespace pommade {
using namespace std;
using namespace xml_graph;

const function<pom_xml_node(const xml_node&, bool)> pom_xml_node::copy_node_fn{[](const xml_node& node, bool gap_before) { return pom_xml_node{node, gap_before}; }};

bool
pom_artifact::operator<(const pom_artifact& that) const {
  return group_id < that.group_id || (group_id == that.group_id && artifact_id < that.artifact_id);
}

bool
pom_artifact_matcher::match(const pom_artifact& that) const {
  if (group_id == that.group_id)
    return artifact_id.empty() || artifact_id == that.artifact_id;
  return false;
}

pom_xml_node::pom_xml_node(const xml_node& node, bool gap_before) : basic_xml_node<pom_xml_node>{node.lineno, node.level, node.name, node.comment.get(), node.get_content()}, gap_before{gap_before} {
  if (node.tree()) {
    for (auto cit = node.tree()->cbegin(); cit != node.tree()->cend(); ++cit)
      add_subnode(copy_node_fn(*cit, false));
  }
}

pom_artifact_matcher
pom_artifact_matcher::parse(const string& pom_artifact_matcher_spec) {
  const auto pos = pom_artifact_matcher_spec.find(':');
  if (pos == 0)
    throw invalid_argument{string{"empty groupId in pom-artifact spec '"} + pom_artifact_matcher_spec + '\''};
  if (pos == string::npos)
    return pom_artifact_matcher{pom_artifact_matcher_spec};
  return pom_artifact_matcher{pom_artifact_matcher_spec.substr(0, pos), pom_artifact_matcher_spec.substr(pos + 1)};
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
  pom_xml_node rw_node{node.lineno, node.level, node.name, node.comment.get(), node.get_content(), gap_before};
  if (node.tree()) {
    for (auto cit = node.tree()->cbegin(); cit != node.tree()->cend(); ++cit)
      rw_node.add_subnode(rw_fn(*cit, cit == node.tree()->cbegin() ? false : gap_before_subnodes));
  }
  return rw_node;
}

pom_xml_node
pom_rewriter::rewrite_sort_subnodes(const xml_node& node, bool gap_before, bool gap_before_subnodes, const function<pom_xml_node(const xml_node&, bool)>& rw_fn, const function<bool(const xml_node*, const xml_node*)>& lt_fn) {
  pom_xml_node rw_node{node.lineno, node.level, node.name, node.comment.get(), node.get_content(), gap_before};
  if (node.tree()) {
    vector<const xml_node*> subnodeps;
    for (auto cit = node.tree()->cbegin(); cit != node.tree()->cend(); ++cit)
      subnodeps.push_back(&*cit);
    sort(subnodeps.begin(), subnodeps.end(), lt_fn);
    vector<unique_ptr<const pom_xml_node>> pom_subnodes;
    bool gap_before_subnode{};
    for (auto nodep : subnodeps) {
      pom_subnodes.push_back(unique_ptr<const pom_xml_node>{new pom_xml_node{rw_fn(*nodep, gap_before_subnode)}});
      gap_before_subnode = gap_before_subnodes;
    }
    rw_node.add_subnodes(move(pom_subnodes));
  }
  return rw_node;
}

pom_xml_node
pom_rewriter::rewrite_leaf_node(const xml_node& node, bool gap_before) {
  assert(node.get_content() && !node.tree());
  return pom_xml_node{node.lineno, node.level, node.name, node.comment.get(), node.get_content(), gap_before};
}

pom_xml_node
pom_rewriter::rewrite_leaf_subnodes(const xml_node& node, bool gap_before) {
  assert(!node.get_content());
  return rewrite_subnodes(node, gap_before, false, rewrite_leaf_node);
}

pom_xml_node
pom_rewriter::rewrite_leaf_subnodes_by_name(const xml_node& node, bool gap_before) {
  assert(!node.get_content());
  return rewrite_sort_subnodes(node, gap_before, false, rewrite_leaf_node, [](const xml_node* a, const xml_node* b) { return a->name < b->name; });
}

pom_xml_node
pom_rewriter::rewrite_parent_node(const xml_node& node, bool gap_before) {
  assert(!node.get_content() && node.tree() && node.tree()->node_cnt() <= 4);

  const vector<const xml_node*> parent_tree{node.tree()->find_in({"groupId", "artifactId", "version", "relativePath"})};
  assert(parent_tree.size() == 4 && parent_tree[0] && parent_tree[1] && parent_tree[2]);

  pom_xml_node rw_parent{node.lineno, node.level, node.name, node.comment.get(), node.get_content(), gap_before};
  rw_parent.add_subnode(rewrite_leaf_node(*parent_tree[0], false));
  rw_parent.add_subnode(rewrite_leaf_node(*parent_tree[1], false));
  rw_parent.add_subnode(rewrite_leaf_node(*parent_tree[2], false));
  add_nonempty_rewrite_node(rw_parent, false, parent_tree[3], rewrite_leaf_node);

  return rw_parent;
}

pom_xml_node
pom_rewriter::rewrite_distribution_management_node(const xml_node& node, bool gap_before) {
  assert(!node.get_content() && node.tree() && node.tree()->node_cnt() <= 2);

  const vector<const xml_node*> distribution_management_tree{node.tree()->find_in(vector<const char*>{"repository", "snapshotRepository"})};
  assert(distribution_management_tree.size() == 2);

  pom_xml_node rw_distribution_management{node.lineno, node.level, node.name, node.comment.get(), node.get_content(), gap_before};
  add_nonempty_rewrite_node(rw_distribution_management, false, distribution_management_tree[0], rewrite_leaf_subnodes_by_name);
  add_nonempty_rewrite_node(rw_distribution_management, false, distribution_management_tree[1], rewrite_leaf_subnodes_by_name);

  return rw_distribution_management;
}

pom_xml_node
pom_rewriter::rewrite_exclusion_node(const xml_node& node, bool gap_before) {
  assert(node.name == "exclusion" && !node.get_content() && node.tree() && node.tree()->node_cnt() >= 1);

  const vector<const xml_node*> exclusion_tree{node.tree()->find_in(vector<const char*>{"groupId", "artifactId"})};
  assert(exclusion_tree.size() == 2 && exclusion_tree[0]);

  pom_xml_node rw_exclusion{node.lineno, node.level, node.name, node.comment.get(), node.get_content(), gap_before};
  rw_exclusion.add_subnode(rewrite_leaf_node(*exclusion_tree[0], false));
  add_nonempty_rewrite_node(rw_exclusion, false, exclusion_tree[1], rewrite_leaf_node);

  return rw_exclusion;
}

pom_xml_node
pom_rewriter::rewrite_exclusions_node(const xml_node& node, bool gap_before) {
  assert(node.name == "exclusions" && !node.get_content());
  return rewrite_sort_subnodes(node, gap_before, false, get_rw_fn(rw_exclusion), get_lt_fn(lt_artifact));
}

pom_xml_node
pom_rewriter::rewrite_dependency_node(const xml_node& node, bool gap_before) {
  assert(node.name == "dependency" && !node.get_content() && node.tree() && node.tree()->node_cnt() <= 6);

  const vector<const xml_node*> dependency_tree{node.tree()->find_in(vector<const char*>{"groupId", "artifactId", "version", "packaging", "scope", "exclusions"})};
  assert(dependency_tree.size() == 6 && dependency_tree[0] && dependency_tree[1]);

  pom_xml_node rw_dependency{node.lineno, node.level, node.name, node.comment.get(), node.get_content(), gap_before};
  rw_dependency.add_subnode(rewrite_leaf_node(*dependency_tree[0], false));
  rw_dependency.add_subnode(rewrite_leaf_node(*dependency_tree[1], false));
  add_nonempty_rewrite_node(rw_dependency, false, dependency_tree[2], rewrite_leaf_node);
  add_nonempty_rewrite_node(rw_dependency, false, dependency_tree[3], rewrite_leaf_node);
  add_nonempty_rewrite_node(rw_dependency, false, dependency_tree[4], rewrite_leaf_node);
  add_nonempty_rewrite_node(rw_dependency, false, dependency_tree[5], get_rw_fn(rw_exclusions));

  return rw_dependency;
}

pom_xml_node
pom_rewriter::rewrite_dependencies_node(const xml_node& node, bool gap_before) {
  assert(node.name == "dependencies" && !node.get_content());
  return rewrite_sort_subnodes(node, gap_before, true, get_rw_fn(rw_dependency), get_lt_fn(lt_artifact));
}

pom_xml_node
pom_rewriter::rewrite_dependency_management_node(const xml_node& node, bool gap_before) {
  assert(!node.get_content());

  pom_xml_node rw_dependency_management{node.lineno, node.level, node.name, node.comment.get(), node.get_content(), gap_before};
  if (!node.tree())
    return rw_dependency_management;

  rw_dependency_management.add_subnode(rewrite_dependencies_node(*node.tree()->cbegin(), false));

  return rw_dependency_management;
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

  pom_xml_node rw_property{node.lineno, node.level, node.name, node.comment.get(), node.get_content(), gap_before};
  rw_property.add_subnode(rewrite_leaf_node(*property_tree[0], false));
  if (!unvalued_ok)
    rw_property.add_subnode(rewrite_leaf_node(*property_tree[1], false));
  else
    add_nonempty_rewrite_node(rw_property, false, property_tree[1], rewrite_leaf_node);

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
pom_rewriter::rewrite_activation_node(const xml_node& node, bool gap_before) {
  assert(!node.get_content() && node.tree());

  const vector<const xml_node*> activation_tree{node.tree()->find_in(vector<const char*>{"activeByDefault", "property"})};
  assert(activation_tree.size() >= 2);

  pom_xml_node rw_activation{node.lineno, node.level, node.name, node.comment.get(), node.get_content(), gap_before};
  add_nonempty_rewrite_node(rw_activation, false, activation_tree[0], rewrite_leaf_node);
  if (activation_tree[1]) {
    for (auto i = 1U; i < activation_tree.size(); ++i)
      rw_activation.add_subnode(rewrite_property_node(*activation_tree[i], false));
  }
  return rw_activation;
}

pom_xml_node
pom_rewriter::rewrite_configuration_node(const xml_node& node, bool gap_before) {
  assert(!node.get_content() && node.tree());
  return rewrite_sort_subnodes(node, gap_before, false, (node.name == "properties" ? get_rw_fn(rw_properties) : pom_xml_node::copy_node_fn), [](const xml_node* a, const xml_node* b) { return a->name == "properties" || a->name < b->name; });
}

pom_xml_node
pom_rewriter::rewrite_execution_node(const xml_node& node, bool gap_before) {
  assert(node.name == "execution" && !node.get_content() && node.tree() && node.tree()->node_cnt() <= 4);

  const vector<const xml_node*> execution_tree{node.tree()->find_in(vector<const char*>{"id", "phase", "goals", "configuration"})};
  assert(execution_tree.size() == 4 && execution_tree[2]);

  pom_xml_node rw_execution{node.lineno, node.level, node.name, node.comment.get(), node.get_content(), gap_before};
  add_nonempty_rewrite_node(rw_execution, false, execution_tree[0], rewrite_leaf_node);
  add_nonempty_rewrite_node(rw_execution, false, execution_tree[1], rewrite_leaf_node);
  rw_execution.add_subnode(rewrite_leaf_subnodes(*execution_tree[2], false));
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

  pom_xml_node rw_plugin{node.lineno, node.level, node.name, node.comment.get(), node.get_content(), gap_before};
  add_nonempty_rewrite_node(rw_plugin, false, plugin_tree[0], rewrite_leaf_node);
  rw_plugin.add_subnode(rewrite_leaf_node(*plugin_tree[1], false));
  add_nonempty_rewrite_node(rw_plugin, false, plugin_tree[2], rewrite_leaf_node);
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
pom_rewriter::rewrite_resource_node(const xml_node& node, bool gap_before) {
  assert(node.name == "resource" && !node.get_content() && node.tree() && node.tree()->node_cnt() <= 4);

  const vector<const xml_node*> resource_tree{node.tree()->find_in(vector<const char*>{"directory", "filtering", "includes", "excludes"})};
  assert(resource_tree.size() == 4 && resource_tree[0]);

  pom_xml_node rw_resource{node.lineno, node.level, node.name, node.comment.get(), node.get_content(), gap_before};
  rw_resource.add_subnode(rewrite_leaf_node(*resource_tree[0], false));
  add_nonempty_rewrite_node(rw_resource, false, resource_tree[1], rewrite_leaf_node);
  add_nonempty_rewrite_node(rw_resource, false, resource_tree[2], rewrite_leaf_subnodes);
  add_nonempty_rewrite_node(rw_resource, false, resource_tree[3], rewrite_leaf_subnodes);

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

  pom_xml_node rw_build{node.lineno, node.level, node.name, node.comment.get(), node.get_content(), gap_before};
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

  pom_xml_node rw_profile{node.lineno, node.level, node.name, node.comment.get(), node.get_content(), gap_before};
  rw_profile.add_subnode(rewrite_leaf_node(*profile_tree[0], false));
  add_nonempty_rewrite_node(rw_profile, false, profile_tree[1], rewrite_leaf_subnodes_by_name);
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
pom_rewriter::rewrite_active_profiles_node(const xml_node& node, bool gap_before) {
  assert(!node.get_content() && node.tree());
  return rewrite_sort_subnodes(node, gap_before, false, rewrite_leaf_node, [](const xml_node* a, const xml_node* b) { return *a->get_content() < *b->get_content(); });
}

pom_xml_node
pom_rewriter::rewrite_project_node(const xml_node& node) {
  assert(!node.get_content() && node.tree() && node.tree()->node_cnt() <= 15);

  const vector<const xml_node*> project_tree{node.tree()->find_in({"modelVersion", "parent", "groupId", "artifactId", "version", "packaging", "properties", "scm", "distributionManagement", "dependencyManagement", "dependencies", "build", "modules", "profiles", "activeProfiles"})};
  assert(project_tree.size() == 15 && project_tree[0] && project_tree[3]);

  pom_xml_node rw_project{node.lineno, node.level, node.name, node.comment.get(), node.get_content(), false};
  rw_project.add_subnode(rewrite_leaf_node(*project_tree[0], false));
  has_parent = add_nonempty_rewrite_node(rw_project, true, project_tree[1], get_rw_fn(rw_parent));
  add_nonempty_rewrite_node(rw_project, false, project_tree[2], rewrite_leaf_node);
  rw_project.add_subnode(rewrite_leaf_node(*project_tree[3], false));
  add_nonempty_rewrite_node(rw_project, false, project_tree[4], rewrite_leaf_node);
  add_nonempty_rewrite_node(rw_project, false, project_tree[5], rewrite_leaf_node);
  add_nonempty_rewrite_node(rw_project, true, project_tree[6], rewrite_leaf_subnodes_by_name);
  add_nonempty_rewrite_node(rw_project, true, project_tree[7], rewrite_leaf_subnodes_by_name);
  add_nonempty_rewrite_node(rw_project, true, project_tree[8], get_rw_fn(rw_distribution_management));
  add_nonempty_rewrite_node(rw_project, true, project_tree[9], get_rw_fn(rw_dependency_management));
  add_nonempty_rewrite_node(rw_project, true, project_tree[10], get_rw_fn(rw_dependencies));
  add_nonempty_rewrite_node(rw_project, true, project_tree[11], get_rw_fn(rw_build));
  add_nonempty_rewrite_node(rw_project, true, project_tree[12], rewrite_leaf_subnodes);
  add_nonempty_rewrite_node(rw_project, true, project_tree[13], get_rw_fn(rw_profiles));
  add_nonempty_rewrite_node(rw_project, true, project_tree[14], get_rw_fn(rw_active_profiles));

  return rw_project;
}

pom_artifact
pom_rewriter::build_pom_artifact(const xml_node& node) {
  assert(node.tree());
  pom_artifact artifact{};
  const auto cend = node.tree()->cend();
  // can't assume that subnodes are sorted!
  for (auto cit = node.tree()->cbegin(); cit != cend; ++cit) {
    if (cit->name == "groupId" && cit->get_content()) {
      artifact.group_id = *cit->get_content();
      if (!artifact.artifact_id.empty())
        break;
    }
    if (cit->name == "artifactId" && cit->get_content()) {
      artifact.artifact_id = *cit->get_content();
      if (!artifact.group_id.empty())
        break;
    }
  }
  return artifact;
}

bool
pom_rewriter::lt_artifact_nodes(const xml_node* a, const xml_node* b) const {
  const pom_artifact a_artifact{build_pom_artifact(*a)}, b_artifact{build_pom_artifact(*b)};
  for (const auto& preferred_artifact : preferred_artifacts) {
    const bool a_match{preferred_artifact.match(a_artifact)}, b_match{preferred_artifact.match(b_artifact)};
    if (a_match && b_match)
      return a_artifact < b_artifact;
    if (a_match != b_match)
      return a_match;
  }
  return a_artifact < b_artifact;
}

pom_xml_node
pom_rewriter::rewrite_pom(const xml_node* node) {
  assert(node);
  if (node->name != "project" || !node->tree())
    throw runtime_error{"root project node missing or empty"};
  return rewrite_project_node(*node);
}
}
