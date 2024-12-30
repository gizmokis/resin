#ifndef RESIN_FILE_DIALOG_HPP
#define RESIN_FILE_DIALOG_HPP

#include <sys/types.h>

#include <future>
#include <glm/ext/scalar_uint_sized.hpp>
#include <libresin/utils/logger.hpp>
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

  inline void open_file(
      const std::function<void(const std::filesystem::path&)>& on_open_function,
      std::optional<std::vector<std::pair<const std::string_view, const std::string_view>>> filters = std::nullopt) {
    on_finish_ = on_open_function;
    start_file_dialog(DialogType::OpenFile, std::move(filters));
  }

  inline void save_file(
      const std::function<void(const std::filesystem::path&)>& on_save_function,
      std::optional<std::vector<std::pair<const std::string_view, const std::string_view>>> filters = std::nullopt,
      std::optional<std::string> default_name                                                       = std::nullopt) {
    on_finish_ = on_save_function;
    start_file_dialog(DialogType::SaveFile, std::move(filters), std::move(default_name));
  }

  inline void pick_folder(const std::function<void(const std::filesystem::path&)>& on_picked_function) {
    on_finish_ = on_picked_function;
    start_file_dialog(DialogType::PickFolder);
  }

  void update();

 private:
  enum class DialogType : u_int8_t { OpenFile = 0, SaveFile, PickFolder };

  FileDialog() = default;

  void start_file_dialog(
      DialogType dialog_type,
      std::optional<std::vector<std::pair<const std::string_view, const std::string_view>>> filters = std::nullopt,
      std::optional<std::string> default_name                                                       = std::nullopt);

 private:
  std::optional<std::future<std::optional<std::string>>> dialog_task_;
  std::thread dialog_thread_;
  std::optional<std::function<void(const std::filesystem::path&)>> on_finish_;
};

}  // namespace resin

#endif
