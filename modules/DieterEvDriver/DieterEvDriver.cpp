// SPDX-License-Identifier: Apache-2.0
// Copyright Pionix GmbH and Contributors to EVerest
#include "DieterEvDriver.hpp"

namespace module {

void DieterEvDriver::init() {
    invoke_init(*p_board_support);
}

void DieterEvDriver::ready() {
    invoke_ready(*p_board_support);
}

} // namespace module
