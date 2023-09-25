#pragma once

#include <filesystem>
#include <functional>
#include <optional>


using OptionalPathCRef = std::optional<const std::reference_wrapper<const std::filesystem::path>>;