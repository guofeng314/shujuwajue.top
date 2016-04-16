#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#include <string.h>


int main( int argc, char* argv[] )
{
		CURL* pEasyHandle = InitCurl();
		CURLcode code;
		if(NULL == pEasyHandle)
		{
			//curl_global_cleanup();
			return false;
		}
		char* szpage = "www.baidu.com";
		curl_easy_setopt(pEasyHandle, CURLOPT_URL, szpage);
		//curl_easy_setopt(pEasyHandle, CURLOPT_POSTFIELDS, "XXXX");  //post访问网页
		curl_easy_setopt(pEasyHandle, CURLOPT_WRITEFUNCTION, &process_data);
		curl_easy_setopt(pEasyHandle, CURLOPT_TIMEOUT, 10);  //设置访问的超时
		
		curl_easy_setopt(pEasyHandle, CURLOPT_FORBID_REUSE, 1);   
		
		char szRet[1024] = {0};
		curl_easy_setopt(pEasyHandle, CURLOPT_WRITEDATA, szRet);
		code = curl_easy_perform(pEasyHandle);
		
		
		if(CURLE_OK != code)
		{
			CleanCurl(pEasyHandle);
			return false;
		}
}
