#pragma once

// This header centralizes Cadmium v2 includes and provides small compatibility
// shims for minor header-name differences (e.g., coupled vs coupled.hpp).

#if __has_include(<cadmium/core/modeling/atomic.hpp>)
  #include <cadmium/core/modeling/atomic.hpp>
#else
  #error "Cadmium v2 not found: expected <cadmium/core/modeling/atomic.hpp>. Check your include path (CADMIUM_V2_INCLUDE)."
#endif

#if __has_include(<cadmium/core/modeling/coupled.hpp>)
  #include <cadmium/core/modeling/coupled.hpp>
#elif __has_include(<cadmium/core/modeling/coupled>)
  #include <cadmium/core/modeling/coupled>
#else
  #error "Cadmium v2 not found: expected coupled model header."
#endif
