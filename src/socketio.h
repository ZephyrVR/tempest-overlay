#pragma once

#undef slot
#undef slots
#undef signal
#undef signals
#undef emit
#include <sio_client.h>
#define slot Q_SLOT
#define slots Q_SLOTS
#define signal Q_SIGNAL
#define signals Q_SIGNALS
#define emit Q_EMIT