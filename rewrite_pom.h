#ifndef REWRITE_POM_H
#define REWRITE_POM_H

#include <functional>
#include <ostream>
#include <string>
#include <vector>

#include "pom_rewriter_fns.h"
#include "xml_graph.h"

namespace pommade {

struct pom_xml_node : public xml_graph::basic_xml_node<pom_xml_node> {
  static const std::function<pom_xml_node(const xml_graph::xml_node&, bool)> copy_node_fn;
  
  const bool gap_before;

  pom_xml_node(unsigned short lineno, unsigned short level, const std::string& name, const std::string* comment, const std::string* content, bool gap_before) : xml_graph::basic_xml_node<pom_xml_node>{lineno, level, name, comment, content}, gap_before{gap_before} {}
  pom_xml_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node(const pom_xml_node& that) : xml_graph::basic_xml_node<pom_xml_node>{that}, gap_before{that.gap_before} {}

  friend std::ostream& operator<<(std::ostream& os, const pom_xml_node& node) {
    if (node.gap_before)
      os << std::endl;
    os << static_cast<const xml_graph::basic_xml_node<pom_xml_node>&>(node);
    return os;
  }
};

struct pom_artifact_matcher {
  std::string group_id;
  std::string artifact_id;

  static pom_artifact_matcher parse(const std::string& pom_artifact_matcher_spec);

private:  
  pom_artifact_matcher(const std::string& group_id, const std::string& artifact_id = "") : group_id{group_id}, artifact_id{artifact_id} {}
};

class pom_rewriter : private pom_rewriter_fns {
  friend struct pom_rewriter_fns;
  
  bool has_parent;
  const std::vector<pom_artifact_matcher>& preferred_artifacts;
  
  const std::function<pom_xml_node(const xml_graph::xml_node&, bool)>& get_rw_fn(rw_key key) { return pom_rewriter_fns::get_rw_fn(key, this); }
  const std::function<pom_xml_node(const xml_graph::xml_node&, bool)>& get_rw_with_flag_fn(rw_with_flag_key key) { return pom_rewriter_fns::get_rw_with_flag_fn(key, this); }
  const std::function<bool(const xml_graph::xml_node*, const xml_graph::xml_node*)>& get_lt_fn(lt_key key) { return pom_rewriter_fns::get_lt_fn(key, this); }

  static bool add_nonempty_rewrite_node(pom_xml_node& node, bool gap_before, const xml_graph::xml_node* subnode, const std::function<pom_xml_node(const xml_graph::xml_node&, bool)>& rw_fn);
  static pom_xml_node rewrite_subnodes(const xml_graph::xml_node& node, bool gap_before, bool gap_before_subnodes, const std::function<pom_xml_node(const xml_graph::xml_node&, bool)>& rw_fn);
  static pom_xml_node rewrite_sort_subnodes(const xml_graph::xml_node& node, bool gap_before, bool gap_before_subnodes, const std::function<pom_xml_node(const xml_graph::xml_node&, bool)>& rw_fn, const std::function<bool(const xml_graph::xml_node*, const xml_graph::xml_node*)>& lt_fn);

  static pom_xml_node rewrite_leaf_node(const xml_graph::xml_node& node, bool gap_before);
  static pom_xml_node rewrite_leaf_subnodes(const xml_graph::xml_node& node, bool gap_before);
  static pom_xml_node rewrite_leaf_subnodes_by_name(const xml_graph::xml_node& node, bool gap_before);
  
  pom_xml_node rewrite_parent_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_distribution_management_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_exclusion_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_exclusions_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_dependency_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_dependencies_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_dependency_management_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_modules_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_property_node(const xml_graph::xml_node& node, bool gap_before, bool unvalued_ok = false);
  pom_xml_node rewrite_properties_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_activation_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_configuration_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_phase_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_goals_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_execution_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_executions_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_plugin_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_plugins_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_plugin_management_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_includes_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_excludes_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_resource_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_resources_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_build_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_profile_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_profiles_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_active_profiles_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_project_node(const xml_graph::xml_node& node);

  bool lt_exclusion_nodes(const xml_graph::xml_node* a, const xml_graph::xml_node* b) const;
  bool lt_dependency_nodes(const xml_graph::xml_node* a, const xml_graph::xml_node* b) const;

 public:
  pom_rewriter(const std::vector<pom_artifact_matcher>& preferred_artifacts) : has_parent{}, preferred_artifacts{preferred_artifacts} {}

  pom_xml_node rewrite_pom(const xml_graph::xml_node* node);
};
}

#endif
