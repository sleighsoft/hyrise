#pragma once

#include <memory>
#include <string>

#include "base_visualizer.hpp"
#include "dot_config.hpp"

namespace opossum {

class JoinGraph;

class JoinGraphVisualizer : public BaseVisualizer  {
 public:
  explicit JoinGraphVisualizer(const DotConfig& config = {});

  void visualize(const std::shared_ptr<JoinGraph>& join_graph, const std::string& output_prefix);

 private:
  const DotConfig _config;
};

}
