#pragma once

#include "types.hpp"

namespace opossum {

/**
 * Turns a scan_type so the expressions if the operands change sides as well, e.g., <= becomes >= and == remains ==
 *
 */
ScanType flip_scan_type(ScanType scan_type);

}