# Narrow, fail-closed corrections to the checksum-pinned Moonbase 4.4.0 module.
# Patch a build-local copy, never a shared SDK checkout. Re-review on SDK updates.
set(RCL_MOONBASE_MODULE "${CMAKE_CURRENT_BINARY_DIR}/rcl-moonbase/moonbase_licensing")
file(COPY "${moonbase_cpp_SOURCE_DIR}/modules/moonbase_licensing"
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/rcl-moonbase")

function(rcl_moonbase_replace relative_path before after)
    set(path "${RCL_MOONBASE_MODULE}/${relative_path}")
    file(READ "${path}" text)
    string(FIND "${text}" "${before}" found)
    if(found EQUAL -1)
        message(FATAL_ERROR "Moonbase hardening context changed: ${relative_path}")
    endif()
    string(REPLACE "${before}" "${after}" text "${text}")
    file(WRITE "${path}" "${text}")
endfunction()

# JUCE emits UTF-8, while std::filesystem::path(string) uses the Windows codepage.
rcl_moonbase_replace("juce/ActivationController.cpp"
    "std::filesystem::path(file.getFullPathName().toStdString())"
    "std::filesystem::u8path(file.getFullPathName().toStdString())")

# A sibling host may be writing the shared cache. An unlocked read can see a
# partial JSON document and the SDK's recovery path then deletes a valid license.
# Release the read lock before validation, which takes its own update lock.
rcl_moonbase_replace("juce/ActivationController.cpp"
    "auto stored = licensing->store().load_local_license();"
    "std::optional<moonbase::license> stored;\n            {\n                auto readGuard = licensing->store().lock_for_update();\n                stored = licensing->store().load_local_license();\n            }")

# Bound corrupt-cache parsing; normal signed license caches are only a few KB.
rcl_moonbase_replace("moonbase/store.hpp"
    "errno = 0;\n        std::ifstream file(path_);"
    "if (std::filesystem::file_size(path_) > 1024 * 1024) {\n            return std::nullopt;\n        }\n\n        errno = 0;\n        std::ifstream file(path_);")

# A future validation date must not bypass checks or extend the offline grace.
rcl_moonbase_replace("moonbase/licensing.hpp"
    "if (age < options_.online_validation_min_interval"
    "if (age >= std::chrono::system_clock::duration::zero()\n            && age < options_.online_validation_min_interval")
rcl_moonbase_replace("moonbase/licensing.hpp"
    "if (age <= options_.online_validation_grace_period) {"
    "if (age >= std::chrono::system_clock::duration::zero()\n                && age <= options_.online_validation_grace_period) {")
