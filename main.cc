#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/filesystem/operations.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>

#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/util/XMLException.hpp>

#include "rewrite_pom.h"
#include "xml_parser.h"

namespace {
using namespace std;
using namespace boost::filesystem;
using namespace boost::program_options;
using namespace xercesc_3_1;
using namespace pommade;
using namespace xml_parser;

vector<pom_artifact_matcher>
parse_pom_artifact_matchers(const vector<string>& pom_artifact_matcher_specs) {
  vector<pom_artifact_matcher> pom_artifacts;
  for (const auto& pom_artifact_matcher_spec : pom_artifact_matcher_specs)
    pom_artifacts.push_back(pom_artifact_matcher::parse(pom_artifact_matcher_spec));
  return pom_artifacts;
}
}

int
main(int argc, const char* argv[]) {
  // gather options
  ostringstream opt_headers_oss;
  const char* const usage = "usage: pommade [options] file";
  opt_headers_oss << "pommade" << endl << usage << endl << "Command-line options";
  options_description cmd_line_opts_desc(opt_headers_oss.str());
  cmd_line_opts_desc.add_options()("help,h", "this help message")("config-file,c", value<string>(), "configuration file");

  options_description config_file_opts_desc("Configuration options");
  config_file_opts_desc.add_options()("preferred-artifact,p", value<vector<string>>()->composing(), "groupId[:artifactId]");
  cmd_line_opts_desc.add(config_file_opts_desc);

  variables_map var_map;
  vector<string> unrecognized_opts;
  try {
    const parsed_options parsed{command_line_parser(argc, argv).options(cmd_line_opts_desc).run()};
    store(parsed, var_map);
    notify(var_map);
    unrecognized_opts = collect_unrecognized(parsed.options, include_positional);
  } catch (const exception& e) {
    cerr << "can't parse command line: " << e.what() << endl;
    return 1;
  }

  // help
  if (var_map.count("help")) {
    cout << cmd_line_opts_desc;
    return 0;
  }
  // configuration file
  if (var_map.count("config-file")) {
    const char* const config_file = var_map["config-file"].as<string>().c_str();
    if (!exists(config_file) || !is_regular_file(config_file)) {
      cerr << "can't find configuration file '" << config_file << '\'' << endl;
      return 1;
    }
    store(parse_config_file<char>(var_map["config-file"].as<string>().c_str(), config_file_opts_desc), var_map);
  }
  notify(var_map);

  // validate file
  if (unrecognized_opts.empty()) {
    cerr << "no file set" << endl;
    return 1;
  } else if (unrecognized_opts.size() != 1) {
    cerr << "unrecognized argument(s) after file '" << unrecognized_opts[0] << '\'' << endl;
    return 1;
  }
  const char* const file = unrecognized_opts[0].c_str();

  // option validation: preferred artifacts
  vector<pom_artifact_matcher> preferred_artifacts;
  if (var_map.count("preferred-artifact")) {
    try {
      preferred_artifacts = parse_pom_artifact_matchers(var_map["preferred-artifact"].as<vector<string>>());
    } catch (const invalid_argument& e) {
      cerr << "invalid preferred artifacts: " << e.what() << endl;
      return 1;
    }
  }

  try {
    default_xml_doc_handler doc_handler;
    cout << pom_rewriter{preferred_artifacts}.rewrite_pom(xml_doc_parser{doc_handler}.parse_doc(file).get());
  } catch (const XMLException& e) {
    cerr << "caught XMLException: " << xmlstring{e.getMessage()} << endl;
    return 1;
  } catch (const SAXParseException& e) {
    cerr << "caught SAXParseException: " << xmlstring{e.getMessage()} << endl;
    return 1;
  } catch (...) {
    cerr << "caught exception" << endl;
    return 1;
  }
}
