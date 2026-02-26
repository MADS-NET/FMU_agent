#pragma once

// Windows MADS packages may ship a split toml++ tree without impl/*.inl.
// Fall back to the bundled monolithic header to keep agent.hpp buildable.
#include <toml.hpp>
