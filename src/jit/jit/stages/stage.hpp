#pragma once

#include <json.hpp>

template <typename... Args>
class Stage {
 public:
  virtual ~Stage() = default;

  void run(Args&... args, nlohmann::json& json) {
    json["type"] = "stage";
    json["name"] = name();
    json["options"] = options();

    auto start = std::chrono::steady_clock::now();
    run_impl(args..., json);
    auto end = std::chrono::steady_clock::now();
    json["runtime"] = std::round(std::chrono::duration<double, std::milli>(end - start).count());
  }

 private:
  virtual std::string name() const = 0;
  virtual nlohmann::json options() const { return {}; }
  virtual void run_impl(Args&... args, nlohmann::json& json) const = 0;
};
