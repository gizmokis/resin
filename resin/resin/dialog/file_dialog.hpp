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

  // According to the nfd.h char definitions:
  //
  // typedef char nfdu8char_t;
  // #ifdef _WIN32
  // /** @typedef UTF-16 character */
  // typedef wchar_t nfdnchar_t;
  // #else
  // /** @typedef UTF-8 character */
  // typedef nfdu8char_t nfdnchar_t;
  // #endif  // _WIN32
  //
  // And filter item strucuture definition:
  //
  // typedef struct {
  //     const nfdnchar_t* name;
  //     const nfdnchar_t* spec;
  // } nfdnfilteritem_t;
  //
  // The wchar_t is used on windows and char on linux, so
  // the struct defined below can be reinterpreted as nfdnfilteritem_t.

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
