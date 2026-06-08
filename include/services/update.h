#pragma once

namespace services::update {
    bool begin();

    bool checkUpdate();
    bool startUpdate();

    bool isUpdating();

    int progress();
}