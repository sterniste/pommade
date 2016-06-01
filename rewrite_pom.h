#ifndef REWRITE_POM_H
#define REWRITE_POM_H

#include <functional>

namespace xml_graph {
class xml_node;
}

namespace pommade {

class pom_rewriter {
  bool has_parent;

  static std::function<xml_graph::xml_node(pom_rewriter*, const xml_graph::xml_node&)> rewrite_for(xml_graph::xml_node (pom_rewriter::*rewrite)(const xml_graph::xml_node &));
  static std::function<xml_graph::xml_node(pom_rewriter*, const xml_graph::xml_node&)> rewrite_with_flag_for(xml_graph::xml_node (pom_rewriter::*rewrite_with_flag)(const xml_graph::xml_node&, bool flag), bool flag);

  void add_nonempty_rewrite(xml_graph::xml_node& node, const xml_graph::xml_node* subnode, xml_graph::xml_node (*rewrite)(pom_rewriter* self, const xml_graph::xml_node&));
  xml_graph::xml_node rewrite_subnodes(const xml_graph::xml_node& node, xml_graph::xml_node (*rewrite)(pom_rewriter* self, const xml_graph::xml_node&));
  xml_graph::xml_node rewrite_sort_subnodes(const xml_graph::xml_node& node, xml_graph::xml_node (*rewrite)(pom_rewriter* self, const xml_graph::xml_node&), bool (*compare)(const xml_graph::xml_node& a, const xml_graph::xml_node& b));

  xml_graph::xml_node rewrite_model_version(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_group_id(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_artifact_id(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_parent_version(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_relative_path(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_parent(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_version(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_packaging(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_project_property(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_project_properties(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_scm_element(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_scm(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_distribution_management_element(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_distribution_management(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_scope(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_dependency(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_dependencies(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_dependency_management(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_module(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_modules(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_id(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_name(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_value(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_property(const xml_graph::xml_node& node, bool unvalued_ok = false);
  xml_graph::xml_node rewrite_properties(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_activation(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_configuration(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_phase(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_goal(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_goals(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_execution(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_executions(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_plugin(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_plugins(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_plugin_management(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_filtering(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_include(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_includes(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_exclude(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_excludes(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_resource(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_resources(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_build(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_profile(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_profiles(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_active_profile(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_active_profiles(const xml_graph::xml_node& node);
  xml_graph::xml_node rewrite_project(const xml_graph::xml_node& node);

 public:
  pom_rewriter() : has_parent{} {}

  xml_graph::xml_node rewrite_pom(const xml_graph::xml_node* node);
};
}

#endif
