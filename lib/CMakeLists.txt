# -------------------- spdlog --------------------
if (NOT TARGET spdlog)
    # Build spdlog as a static library to keep compile times down
    set(SPDLOG_COMPILED_LIB ON)
    add_subdirectory(spdlog)
endif()
