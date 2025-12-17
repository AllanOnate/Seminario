#pragma once

// Centralized Cadmium v2 includes.
//
// There are (at least) two common Cadmium v2 include layouts in the wild:
//
//   A) include/cadmium/core/modeling/...   (as in the official wiki examples)
//   B) include/cadmium/modeling/devs/...   (as in some forks / packaged layouts)
//
// Your local screenshot shows layout (B):
//   ~/cadmium_v2/include/cadmium/modeling/devs/atomic.hpp
//   ~/cadmium_v2/include/cadmium/modeling/devs/coupled.hpp
//
// We support both via __has_include checks.

// --- Atomic / Coupled / Ports ------------------------------------------------

#if __has_include(<cadmium/core/modeling/atomic.hpp>)
  // Layout A
  #include <cadmium/core/modeling/atomic.hpp>
  #include <cadmium/core/modeling/coupled.hpp>
  // Port headers may be transitively included, but include explicitly when present.
  #if __has_include(<cadmium/core/modeling/port.hpp>)
    #include <cadmium/core/modeling/port.hpp>
  #endif
  #if __has_include(<cadmium/core/modeling/component.hpp>)
    #include <cadmium/core/modeling/component.hpp>
  #endif

#elif __has_include(<cadmium/modeling/devs/atomic.hpp>)
  // Layout B
  #include <cadmium/modeling/devs/atomic.hpp>
  #include <cadmium/modeling/devs/coupled.hpp>
  #include <cadmium/modeling/devs/port.hpp>
  #include <cadmium/modeling/devs/component.hpp>

#else
  #error "Cadmium v2 headers not found. Your -I path must point to the folder that contains the 'cadmium' directory (e.g., ~/cadmium_v2/include)."
#endif
