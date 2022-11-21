#include "AssetDownloader.hpp"

#include <curl/curl.h>

#include <iostream>

namespace
{
    size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
    {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
}

AssetDownloader::AssetDownloader()
{
    curl_global_init(CURL_GLOBAL_ALL);
}
AssetDownloader::~AssetDownloader()
{
    curl_global_cleanup();
}

void AssetDownloader::download(const std::string& url, const std::string& path, const std::function<void(bool)>& callback)
{
 /* init the curl session */
  CURL* curl_handle = curl_easy_init();

  /* set URL to get here */
  curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());

  /* Switch on full protocol/debug output while testing */
  curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);

  /* disable progress meter, set to 0L to enable it */
  curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);

  /* send all data to this function  */
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteCallback);

  /* open the file */
  FILE* pagefile = fopen(path.c_str(), "wb");
  if(pagefile) {

    /* write the page body to this file handle */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);

    /* get it! */
    curl_easy_perform(curl_handle);

    /* close the header file */
    fclose(pagefile);
  }

  /* cleanup curl stuff */
  curl_easy_cleanup(curl_handle);
 }