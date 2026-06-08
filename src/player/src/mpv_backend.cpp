#include "player/mpv_backend.h"

#include <mpv/client.h>

namespace yapcr::player {

unsigned long mpvClientApiVersion() {
    return mpv_client_api_version();
}

}  // namespace yapcr::player
