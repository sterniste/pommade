#ifndef POM_REWRITER_FNS_H
#define POM_REWRITER_FNS_H

#include <cstddef>
#include <functional>
#include <unordered_map>
namespace xml_graph {
class xml_node;
}

namespace pommade {

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

struct pom_xml_node;
class pom_rewriter;

struct pom_rewriter_fns {
  enum rw_keys { rw_parent = 0, rw_distribution_management, rw_exclusion, rw_exclusions, rw_dependency, rw_dependencies, rw_dependency_management, rw_properties, rw_activation, rw_configuration, rw_execution, rw_executions, rw_plugin, rw_plugins, rw_plugin_management, rw_resource, rw_resources, rw_build, rw_profile, rw_profiles, rw_active_profiles };
  std::unordered_map<rw_key, std::function<pom_xml_node(const xml_graph::xml_node&, bool)>> rws_fn_map;

  enum rw_with_flags { rw_with_flag_property = 0 };
  std::unordered_map<rw_with_flag_key, std::function<pom_xml_node(const xml_graph::xml_node&, bool)>, rw_with_flag_key::hasher> rws_with_flag_fn_map;

  enum lt_keys { lt_exclusion = 0, lt_dependency };
  std::unordered_map<lt_key, std::function<bool(const xml_graph::xml_node*, const xml_graph::xml_node*)>> lts_fn_map;

  const std::function<pom_xml_node(const xml_graph::xml_node&, bool)>& get_rw_fn(rw_key key, pom_rewriter* rewriter);
  const std::function<pom_xml_node(const xml_graph::xml_node&, bool)>& get_rw_with_flag_fn(rw_with_flag_key key, pom_rewriter* rewriter);
  const std::function<bool(const xml_graph::xml_node*, const xml_graph::xml_node*)>& get_lt_fn(lt_key key, pom_rewriter* rewriter);
};
}
#endif
