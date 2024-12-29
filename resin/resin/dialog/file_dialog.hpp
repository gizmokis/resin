#ifndef RESIN_FILE_DIALOG_HPP
#define RESIN_FILE_DIALOG_HPP

#include <nfd/nfd.h>

#include <future>
#include <libresin/utils/logger.hpp>
#include <nfd/nfd.hpp>
#include <optional>
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

  inline void open(std::vector<nfdfilteritem_t> filters,
                   const std::function<void(const std::filesystem::path&)>& on_open_function) {
    on_finish_ = on_open_function;
    start_file_dialog(std::move(filters), true);
  }

  inline void save(std::vector<nfdfilteritem_t> filters,
                   const std::function<void(const std::filesystem::path&)>& on_save_function) {
    on_finish_ = on_save_function;
    start_file_dialog(std::move(filters), false);
  }

  void update();

 private:
  FileDialog() = default;
  void start_file_dialog(std::vector<nfdfilteritem_t> filters, bool is_open);

 private:
  std::optional<std::future<std::optional<std::string>>> dialog_task_;
  std::thread dialog_thread_;
  std::optional<std::function<void(const std::filesystem::path&)>> on_finish_;
};

}  // namespace resin

#endif
