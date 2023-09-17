#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <string>


using OptionalPathCRef   = std::optional<const std::reference_wrapper<const std::filesystem::path>>;
using OptionalStringCRef = std::optional<const std::reference_wrapper<const std::string>>;