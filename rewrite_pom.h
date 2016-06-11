#ifndef REWRITE_POM_H
#define REWRITE_POM_H

#include <cstddef>
#include <functional>
#include <ostream>
#include <string>
#include <unordered_map>

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

using rw_key = unsigned short int;

using rw_with_flag = unsigned short int;

struct rw_with_flag_key {
  struct hasher {
    std::size_t operator()(const rw_with_flag_key& key) const { return std::hash<unsigned int>()(key.i + static_cast<unsigned int>(key.flag << 16)); }
  };

  const rw_with_flag i;
  const bool flag;

  rw_with_flag_key(rw_with_flag i, bool flag) : i{i}, flag{flag} {}

  bool operator==(const rw_with_flag_key& that) const { return i == that.i && flag == that.flag; }
};

using lt_key = unsigned short int;

class pom_rewriter; 

struct pom_rewriter_fns {
  enum rw_keys { rw_parent = 0, rw_project_properties, rw_scm, rw_repository, rw_snapshot_repository, rw_distribution_management, rw_exclusion, rw_exclusions, rw_dependency, rw_dependencies, rw_dependency_management, rw_modules, rw_properties, rw_activation, rw_configuration, rw_goals, rw_execution, rw_executions, rw_plugin, rw_plugins, rw_plugin_management, rw_includes, rw_excludes, rw_resource, rw_resources, rw_build, rw_profile, rw_profiles, rw_active_profiles };
  std::unordered_map<rw_key, std::function<pom_xml_node(const xml_graph::xml_node&, bool)>> rws_fn_map;

  enum rw_with_flags { rw_with_flag_property = 0 };
  std::unordered_map<rw_with_flag_key, std::function<pom_xml_node(const xml_graph::xml_node&, bool)>, rw_with_flag_key::hasher> rws_with_flag_fn_map;

  enum lt_keys { lt_exclusion = 0, lt_dependency };
  std::unordered_map<lt_key, std::function<bool(const xml_graph::xml_node*, const xml_graph::xml_node*)>> lts_fn_map;

  const std::function<pom_xml_node(const xml_graph::xml_node&, bool)>& get_rw_fn(rw_key key, pom_rewriter* rewriter);
  const std::function<pom_xml_node(const xml_graph::xml_node&, bool)>& get_rw_with_flag_fn(rw_with_flag_key key, pom_rewriter* rewriter);
  const std::function<bool(const xml_graph::xml_node*, const xml_graph::xml_node*)>& get_lt_fn(lt_key key, pom_rewriter* rewriter);
};

class pom_rewriter : private pom_rewriter_fns {
  friend struct pom_rewriter_fns;
  
  bool has_parent;
  
  const std::function<pom_xml_node(const xml_graph::xml_node&, bool)>& get_rw_fn(rw_key key) { return pom_rewriter_fns::get_rw_fn(key, this); }
  const std::function<pom_xml_node(const xml_graph::xml_node&, bool)>& get_rw_with_flag_fn(rw_with_flag_key key) { return pom_rewriter_fns::get_rw_with_flag_fn(key, this); }
  const std::function<bool(const xml_graph::xml_node*, const xml_graph::xml_node*)>& get_lt_fn(lt_key key) { return pom_rewriter_fns::get_lt_fn(key, this); }

  bool add_nonempty_rewrite_node(pom_xml_node& node, bool gap_before, const xml_graph::xml_node* subnode, const std::function<pom_xml_node(const xml_graph::xml_node&, bool)>& rw_fn);
  pom_xml_node rewrite_subnodes(const xml_graph::xml_node& node, bool gap_before, bool gap_before_subnodes, const std::function<pom_xml_node(const xml_graph::xml_node&, bool)>& rw_fn);
  pom_xml_node rewrite_sort_subnodes(const xml_graph::xml_node& node, bool gap_before, bool gap_before_subnodes, const std::function<pom_xml_node(const xml_graph::xml_node&, bool)>& rw_fn, const std::function<bool(const xml_graph::xml_node*, const xml_graph::xml_node*)>& lt_fn);

  static pom_xml_node rewrite_leaf_node(const xml_graph::xml_node& node, bool gap_before);
  
  pom_xml_node rewrite_parent_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_project_properties_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_repository_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_snapshot_repository_node(const xml_graph::xml_node& node, bool gap_before);
  pom_xml_node rewrite_scm_node(const xml_graph::xml_node& node, bool gap_before);
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
  pom_rewriter() : has_parent{} {}

  pom_xml_node rewrite_pom(const xml_graph::xml_node* node);
};
}

#endif
