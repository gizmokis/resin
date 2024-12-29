#ifndef RESIN_FILE_DIALOG_HPP
#define RESIN_FILE_DIALOG_HPP

#include <future>
#include <libresin/utils/logger.hpp>
#include <nfd/nfd.hpp>
#include <optional>
#include <string>
#include <thread>
#include <vector>

namespace resin {

class FileDialog {
 public:
  FileDialog(FileDialog&&)            = delete;
  FileDialog(FileDialog&)             = delete;
  FileDialog& operator=(FileDialog&)  = delete;
  FileDialog& operator=(FileDialog&&) = delete;

  ~FileDialog();

  static FileDialog& instance() {
    static FileDialog instance;
    return instance;
  }

  inline bool is_active() const { return dialog_task_.has_value(); }

  inline void open(
      const std::function<void(const std::filesystem::path&)>& on_open_function,
      std::optional<std::vector<std::pair<const std::string_view, const std::string_view>>> filters = std::nullopt) {
    on_finish_ = on_open_function;
    start_file_dialog(true, std::move(filters));
  }

  inline void save(
      const std::function<void(const std::filesystem::path&)>& on_save_function,
      std::optional<std::vector<std::pair<const std::string_view, const std::string_view>>> filters = std::nullopt,
      std::optional<std::string> default_name                                                       = std::nullopt) {
    on_finish_ = on_save_function;
    start_file_dialog(false, std::move(filters), std::move(default_name));
  }

  void update();

 private:
  FileDialog() = default;
  void start_file_dialog(
      bool is_open,
      std::optional<std::vector<std::pair<const std::string_view, const std::string_view>>> filters = std::nullopt,
      std::optional<std::string> default_name                                                       = std::nullopt);

 private:
  std::optional<std::future<std::optional<std::string>>> dialog_task_;
  std::thread dialog_thread_;
  std::optional<std::function<void(const std::filesystem::path&)>> on_finish_;
};

}  // namespace resin

#endif
