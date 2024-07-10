#include "utilities.h"

bool Utilities::isHandleValid(HANDLE h) {
    return h != 0 && h != INVALID_HANDLE_VALUE;
}
