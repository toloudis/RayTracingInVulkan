#pragma once

#include <string>
#include <functional>

class AssetDownloader {

public:
    AssetDownloader();
    ~AssetDownloader();

    void download(const std::string& url, const std::string& path, const std::function<void(bool)>& callback);
};
