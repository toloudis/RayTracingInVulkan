#pragma once

#include <curl/curl.h>

#include <string>
#include <filesystem>
#include <functional>
#include <mutex>
#include <condition_variable>

class AssetDownloader {

public:
    AssetDownloader();
    ~AssetDownloader();

    bool download(const std::string& url, const std::string& path, curl_xferinfo_callback progressCallback);
};

struct AssetEntry {
    enum eState { NOT_DOWNLOADED, DOWNLOADING, DOWNLOADED };
    std::string name;
    std::string url;
    std::string localPath;
    eState state; // 0 = not downloaded, 1 = downloading, 2 = downloaded
    // callback should broadcast to whole scene that this asset is ready
    //std::function<void(std::string)> callback;
    std::thread* downloadThread;
};

class AssetCache {
    std::filesystem::path mLocalCacheDir;
    AssetDownloader mDownloader;
    std::unordered_map<std::string, std::shared_ptr<AssetEntry>> mCache;
    std::mutex mMutex;
    std::condition_variable mCv;
    std::function<void(std::shared_ptr<AssetEntry>)> mOnAssetReady;
    public:

    AssetCache(std::string localCacheDir = ".") {
        mLocalCacheDir = std::filesystem::absolute(localCacheDir);
        std::filesystem::create_directories(mLocalCacheDir);
    }
    ~AssetCache() {
        std::lock_guard<std::mutex> lock(mMutex);

        // TODO: signal to cancel all threads currently downloading
        // and drop all shared_ptrs to AssetEntry
        mCv.notify_all();
        for (auto& entry : mCache) {
            auto asset = entry.second;
            if (asset->state == AssetEntry::DOWNLOADING) {
                if (asset->downloadThread && asset->downloadThread->joinable()) {
					// TODO CANT JOIN BECAUSE THIS WILL TRY TO LOCK MUTEX AGAIN
                    // asset->downloadThread->join();
                    delete asset->downloadThread;
					asset->downloadThread = nullptr;
                }
            }
            entry.second.reset();
        }
    }
    std::shared_ptr<AssetEntry> get(std::string s) {
        std::lock_guard<std::mutex> lock(mMutex);
        if (mCache.find(s) != mCache.end()) {
            return mCache[s];
        }
        // create new entry and start download thread
        auto e = std::make_shared<AssetEntry>();
        e->name = s;
        e->url = s;
        e->localPath = (mLocalCacheDir / "test").string();
        e->state = AssetEntry::DOWNLOADING;
        mCache[s] = e;
        e->downloadThread = new std::thread([this](AssetDownloader& d, std::shared_ptr<AssetEntry> e) {
            // todo make this a loop that is cancellable during progress checks
            bool success = d.download(e->url, e->localPath, [](void *clientp,
                curl_off_t dltotal,
                curl_off_t dlnow,
                curl_off_t ultotal,
                curl_off_t ulnow)->int {
                // TODO check for abort?
                printf("Downloaded %lld of %lld bytes\r\n", dlnow, dltotal);
                return CURL_PROGRESSFUNC_CONTINUE;
                });

//            if (mCv.)
            {
                std::lock_guard<std::mutex> lock(mMutex);

                if (success) {
                    e->state = AssetEntry::DOWNLOADED;
					// TODO delete/discard downloadThread somehow???
                }
                else {
                    e->state = AssetEntry::NOT_DOWNLOADED;
                }
            }
            // broadcast this asset to rest of app
            if (mOnAssetReady) {
                mOnAssetReady(e);
            }
        }, std::ref(mDownloader), e);
        return e;
    }
};