/**
 * @file parse.hpp
 * @brief Parse declarations — creates Moment objects from string input.
 *
 * Provides factory functions for creating Moment instances from various string
 * formats (ISO 8601, RFC 2822, custom format tokens), from timestamps, and from
 * individual date/time components. Also provides aggregation (min/max) functions.
 *
 * All parse factory functions are declared in moment.hpp alongside the Moment class;
 * this header is provided for organizational consistency and re-exports those
 * declarations. The actual implementations are in detail/parse.hpp.
 *
 * @see https://momentjs.com/docs/#/parsing/
 * @since 0.2.0
 */
#pragma once

#include <polycpp/moment/moment.hpp>

// All parse factory function declarations are in moment.hpp.
// This header exists for organizational parity with the design document.
