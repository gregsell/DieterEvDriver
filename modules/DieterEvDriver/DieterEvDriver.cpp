// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest
#include "DieterEvDriver.hpp"

namespace module {

void DieterEvDriver::init() {
    // try to open serial port
    invoke_init(*p_board_support);
}

void DieterEvDriver::ready() {
    // possible GPIO reset
    invoke_ready(*p_board_support);
}

} // namespace module
