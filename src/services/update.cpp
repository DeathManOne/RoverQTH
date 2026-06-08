#include "services/update.h"

namespace services::update {
    namespace {
        bool _UPDATING = false;
        int _PROGRESS = 0;
    }

    bool begin() {
        _UPDATING = false;
        _PROGRESS = 0;
        return true;
    }

    bool checkUpdate() {
        // TODO
        return false;
    }

    bool startUpdate() {
        // TODO
        _UPDATING = true;
        _PROGRESS = 0;
        return false;
    }

    bool isUpdating() {
        return _UPDATING;
    }

    int progress() {
        return _PROGRESS;
    }
}