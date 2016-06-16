#include <cassert>
#include <functional>
#include <utility>

#include "pom_rewriter_fns.h"
#include "rewrite_pom.h"
namespace xml_graph {
class xml_node;
}

namespace pommade {
using namespace std;
using namespace std::placeholders;
using namespace xml_graph;

const function<pom_xml_node(const xml_node&, bool)>&
pom_rewriter_fns::get_rw_fn(rw_key key, pom_rewriter* rewriter) {
  const auto cit = rws_fn_map.find(key);
  if (cit != rws_fn_map.cend())
    return cit->second;
  pom_xml_node (pom_rewriter::*rw_f)(const xml_node&, bool){};
  switch (key) {
  case rw_parent:
    rw_f = &pom_rewriter::rewrite_parent_node;
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
  case rw_dependency:
    rw_f = &pom_rewriter::rewrite_dependency_node;
    break;
  case rw_dependencies:
    rw_f = &pom_rewriter::rewrite_dependencies_node;
    break;
  case rw_dependency_management:
    rw_f = &pom_rewriter::rewrite_dependency_management_node;
    break;
  case rw_properties:
    rw_f = &pom_rewriter::rewrite_properties_node;
    break;
  case rw_activation:
    rw_f = &pom_rewriter::rewrite_activation_node;
    break;
  case rw_configuration:
    rw_f = &pom_rewriter::rewrite_configuration_node;
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
  case rw_active_profiles:
    rw_f = &pom_rewriter::rewrite_active_profiles_node;
    break;
  }
  assert(rw_f);
  auto insert = rws_fn_map.insert(pair<rw_key, function<pom_xml_node(const xml_node&, bool)>>{key, bind(rw_f, rewriter, _1, _2)});
  assert(insert.second);
  return insert.first->second;
}

const function<pom_xml_node(const xml_node&, bool)>&
pom_rewriter_fns::get_rw_with_flag_fn(rw_with_flag_key key, pom_rewriter* rewriter) {
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
  auto insert = rws_with_flag_fn_map.insert(pair<rw_with_flag_key, function<pom_xml_node(const xml_node&, bool)>>{key, bind(rw_with_flag_f, rewriter, _1, _2, key.flag)});
  assert(insert.second);
  return insert.first->second;
}

const function<bool(const xml_node*, const xml_node*)>&
pom_rewriter_fns::get_lt_fn(lt_key key, pom_rewriter* rewriter) {
  const auto cit = lts_fn_map.find(key);
  if (cit != lts_fn_map.cend())
    return cit->second;
  bool (pom_rewriter::*lt_f)(const xml_node*, const xml_node*) const {};
  switch (key) {
  case lt_exclusion:
    lt_f = &pom_rewriter::lt_exclusion_nodes;
    break;
  case lt_dependency:
    lt_f = &pom_rewriter::lt_dependency_nodes;
    break;
  }
  assert(lt_f);
  auto insert = lts_fn_map.insert(pair<lt_key, function<bool(const xml_node*, const xml_node*)>>{key, bind(lt_f, rewriter, _1, _2)});
  assert(insert.second);
  return insert.first->second;
}
}
