#ifndef _PARTNET_LOADER
#define _PARTNET_LOADER
#include "json.hpp"
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

void parserHelper(std::map<int, std::vector<std::string>> &out, const json &j) {
  if (j.contains("objs")) {
    out[j.at("id").get<int>()] = j.at("objs").get<std::vector<std::string>>();
  }
  if (j.contains("children")) {
    auto children = j.at("children").get<std::vector<json>>();
    for (auto &child : children) {
      parserHelper(out, child);
    }
  }
}

std::map<int, std::vector<std::string>> loadPartNet(const std::string &dir) {
  json j;
  std::ifstream i(dir + "/result.json");
  i >> j;
  std::map<int, std::vector<std::string>> objMap;
  std::vector<json> js = j.get<std::vector<json>>();

  for (auto &j : js) {
    parserHelper(objMap, j);
  }

  for (auto &p : objMap) {
    std::cout << p.first << ": " << p.second.size() << std::endl;
  }
  return objMap;
}


#endif
