#ifndef RESIN_FILE_DIALOG_HPP
#define RESIN_FILE_DIALOG_HPP

#include <sys/types.h>

#include <cstdint>
#include <future>
#include <glm/ext/scalar_uint_sized.hpp>
#include <libresin/utils/logger.hpp>
#include <optional>
#include <span>
#include <string>
#include <thread>

namespace resin {

class FileDialog {
 public:
  FileDialog(FileDialog&&)            = delete;
  FileDialog(FileDialog&)             = delete;
  FileDialog& operator=(FileDialog&)  = delete;
  FileDialog& operator=(FileDialog&&) = delete;

  ~FileDialog();

  // As the UTF8 version of the nfd is used on all platforms, the
  // following struct can be reinterpereted to match the nfdu8filteritem_t.
  struct FilterItem {
    const char* name;
    const char* spec;
    FilterItem(const char* _name, const char* _spec) : name(_name), spec(_spec) {}
  };

  static FileDialog& instance() {
    static FileDialog instance;
    return instance;
  }

  inline bool is_active() const { return dialog_task_.has_value(); }

  inline void open_file(const std::function<void(const std::filesystem::path&)>& on_open_function,
                        std::optional<std::span<const FilterItem>> filters = std::nullopt) {
    on_finish_ = on_open_function;
    start_file_dialog(DialogType::OpenFile, std::move(filters));
  }

  inline void save_file(const std::function<void(const std::filesystem::path&)>& on_save_function,
                        std::optional<std::span<const FilterItem>> filters = std::nullopt,
                        std::optional<std::string> default_name            = std::nullopt) {
    on_finish_ = on_save_function;
    start_file_dialog(DialogType::SaveFile, std::move(filters), std::move(default_name));
  }

  inline void pick_folder(const std::function<void(const std::filesystem::path&)>& on_pick_function) {
    on_finish_ = on_pick_function;
    start_file_dialog(DialogType::PickFolder);
  }

  void update();

 private:
  enum class DialogType : uint8_t { OpenFile = 0, SaveFile, PickFolder };

  FileDialog() = default;

  void start_file_dialog(DialogType dialog_type, std::optional<std::span<const FilterItem>> filters = std::nullopt,
                         std::optional<std::string> default_name = std::nullopt);

 private:
  std::optional<std::future<std::optional<std::string>>> dialog_task_;
  std::thread dialog_thread_;
  std::optional<std::function<void(const std::filesystem::path&)>> on_finish_;
};

}  // namespace resin

#endif
