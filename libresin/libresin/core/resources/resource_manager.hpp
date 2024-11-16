#ifndef RESIN_RESOURCE_MANAGER_HPP
#define RESIN_RESOURCE_MANAGER_HPP

#include <filesystem>
#include <libresin/utils/logger.hpp>
#include <optional>
#include <unordered_map>

namespace resin {

template <typename T>
concept ResourceConcept = std::is_copy_constructible_v<T> && std::is_move_constructible_v<T>;

template <ResourceConcept Resource>
class ResourceManager {
 public:
  virtual ~ResourceManager() = default;

  virtual std::optional<std::shared_ptr<const Resource>> get_res(const std::filesystem::path& path) {
    auto elem = cache_.find(path);
    if (elem != cache_.end()) {
      return elem->second;
    }

    auto res = load_res(path);
    if (!res.has_value()) {
      resin::Logger::err("Could not load resource with path \"{}\".", path.string());
      return std::nullopt;
    }

    auto res_ptr = std::make_shared<const Resource>(res);

    cache_[path] = res_ptr;

    resin::Logger::info("Loaded and cached resource with path \"{}\".", path.string());

    return res_ptr;
  }

  virtual std::optional<Resource> load_res(const std::filesystem::path&) = 0;

 protected:
  std::unordered_map<std::filesystem::path, std::shared_ptr<const Resource>> cache_;
};

}  // namespace resin
#endif  // RESIN_RESOURCE_MANAGER_HPP
