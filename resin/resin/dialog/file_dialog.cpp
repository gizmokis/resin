#include <resin/dialog/file_dialog.hpp>

namespace resin {

FileDialog::~FileDialog() {
  if (is_active()) {
    // https://stackoverflow.com/a/58222149
    // The nfd is a blocking api so there is no way to tell it to stop working.
    // An alternative would be to freeze ui and wait for the dialog exit by calling .join() instead.
    this->dialog_thread_.detach();
  }
}

void FileDialog::update() {
  if (!dialog_task_.has_value()) {
    return;
  }

  if (dialog_task_->wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
    return;
  }

  // the task is ready
  auto result = dialog_task_->get();
  if (result.has_value()) {
    if (on_save_function_.has_value()) {
      (*on_save_function_)(std::filesystem::path(*result));
    } else {
      Logger::warn(R"(Obtained save path target, but no "on save" function is set)");
    }
  } else {
    Logger::info("Save dialog cancelled");
  }

  dialog_task_ = std::nullopt;
  dialog_thread_.join();
}

void FileDialog::start_file_dialog(std::vector<nfdfilteritem_t> filters, bool is_open) {
  if (dialog_task_.has_value()) {
    Logger::warn("Detected attempt to open second file dialog");
    return;
  }

  std::promise<std::optional<std::string>> curr_promise;
  dialog_task_   = curr_promise.get_future();
  dialog_thread_ = std::thread(
      [is_open](std::promise<std::optional<std::string>> promise, std::vector<nfdfilteritem_t> dialog_filters) {
        NFD::Guard nfd_guard;
        NFD::UniquePath out_path;
        nfdresult_t result = NFD_ERROR;

        if (is_open) {
          result =
              NFD::OpenDialog(out_path, dialog_filters.data(), static_cast<nfdfiltersize_t>(dialog_filters.size()));
        } else {
          result =
              NFD::SaveDialog(out_path, dialog_filters.data(), static_cast<nfdfiltersize_t>(dialog_filters.size()));
        }

        if (result == NFD_OKAY) {
          promise.set_value(std::string(out_path.get()));
          out_path.release();  // NOLINT
        } else {
          promise.set_value(std::nullopt);
        }
      },
      std::move(curr_promise), std::move(filters));
}

}  // namespace resin
