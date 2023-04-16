#include "AssetDownloader.hpp"

#include <curl/curl.h>

#include <iostream>

namespace
{
    size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
    {
		FILE* f = (FILE*)userp;
		fwrite(contents, size, nmemb, f);
        return size * nmemb;
    }

#if 0
    int progress_callback(
        void *clientp,
        curl_off_t dltotal,
        curl_off_t dlnow,
        curl_off_t ultotal,
        curl_off_t ulnow) {

        CURL* curl_handle = (CURL*)clientp;

        curl_off_t dlt = 0;
        curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &dlt);

        // TOOD check for abort?
        printf("Downloaded %lld of %lld bytes\r\n", (long long int)dlnow, (long long int)dlt);
        return CURL_PROGRESSFUNC_CONTINUE;
    }

#endif
}

AssetDownloader::AssetDownloader()
{
    curl_global_init(CURL_GLOBAL_ALL);
}
AssetDownloader::~AssetDownloader()
{
    curl_global_cleanup();
}

// TODO make a thread function that can be interrupted midway through the download
// and can signal when it's done
bool AssetDownloader::download(const std::string& url, const std::string& path, curl_xferinfo_callback progressCallback)
{
 /* init the curl session */
  CURL* curl_handle = curl_easy_init();
  if (curl_handle) {
      /* set URL to get here */
      curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());

      /* Switch on full protocol/debug output while testing */
      //curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);


      /* open the file */
      FILE* pagefile = fopen(path.c_str(), "wb");
      if (pagefile) {
        /* disable progress meter, set to 0L to enable it, 1L to disable */
        curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_XFERINFODATA, curl_handle);
        curl_easy_setopt(curl_handle, CURLOPT_XFERINFOFUNCTION,
                            progressCallback);

        /* send all data to this function  */
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteCallback);

          /* write the page body to this file handle */
          curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);

          /* get it! */
          curl_easy_perform(curl_handle);

          /* close the header file */
          fclose(pagefile);
      }

      /* cleanup curl stuff */
      curl_easy_cleanup(curl_handle);

      return true;

  }
  else {
	  std::cout << "Curl init failed" << std::endl;
  }
  return false;
 }