/**
 * @file HCUrl.h
 *
 * @brief
 * The CCUrl class header file.

 *
 * @author zieckey
 *
 * @date Jun. 3 2009
 *
 * @version 0.0.1
 *
 * Revision History
 * @if CR/PR ID Author    Date    Major Change @endif
 * @bug \n
****************************************************************************/

#ifndef __CCUrl_H__
#define __CCUrl_H__

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#include <string.h>

namespace OSLib
{
    //const long CLIENTTIMEOUT = 10;


    /*! @class CCUrl CCUrl.h "CCUrl.h"
    * @brief This is the CCUrl class, a wrapper of libcurl.
    *
    */
    class CCUrl
    {
    public:
        CCUrl();
        virtual ~CCUrl();
    
        /**
        * @brief Summary:
        *    Initiate this CURL session
        * @return    0, if success. 

        *            -1, if failed.
        *******************************************************************************/
        int    init();
    
    protected:
        /**
        * @brief Summary:
        *        This function is virtual function, which will be called by automatic, 
        *    as soon as there is data received that needs to be processed.

        *        The size of the data pointed to by 'data' is 'size' multiplied with 'nmemb', 
        *    it will not be zero terminated. 

        * @note    You should override this function to do you own busyness. 
        *        You save the data to a file, or process it anyway.
        * @param[in] data, point to the data
        * @param[in] size, the size        
        * @param[in] nmemb, the unit in bytes        
        * @return 
        *****************************************************************************/
        virtual int process( void* data, size_t size, size_t nmemb );

    public:
        /**
        * @brief Summary:
        *    Post 'data' to 'url', and the server response data is restored in resultData.
        *    The process function is 'pfun' 
        * @param[in]    url
        * @param[in]    data, the data to be sent to server. 
        *                    eg. "id=admin&passwd=123456"
        * @param[in]    timeout, the limit time in seconds to do this process.
        *                    default value 10
        * @param[in]    withheader, 
        *                    1 indicate the server response data including header
        *                    0 indicate the server response data excluding header
        *                    default value 0
        * @return    0, if success. 

        *            -1, if failed.
        *******************************************************************************/
        int doHttpPost( const char* url, const char* data, long timeout = 10, long withheader = 0 );

        /**
        * @brief Summary:
        *    Get 'url' using 'data' to be set in the header, 
        *    and the server response data is restored in resultData.
        *    The process function is 'pfun' 
        * @param[in]    url
        * @param[in]    data, the data to be sent to server. 
        *                    eg. "id=admin&passwd=123456"
        * @param[in]    timeout, the limit time in seconds to do this process.
        *                    default value 10
        * @param[in]    withheader, 
        *                    1 indicate the server response data including header
        *                    0 indicate the server response data excluding header
        *                    default value 0
        * @return    0, if success. 

        *            -1, if failed.
        *******************************************************************************/
        int doHttpGet( const char* url, const char* data, long timeout = 10, long withheader = 0 );


    public:
        //CURLcode setOpt( CURLoption option ); 


        /**
        * @brief Summary:
        *    Reset all options of a libcurl session handle
        * @return void, no return
        *******************************************************************************/
        void resetOpt(); 
        
    private:
        /**
        * @brief Summary:
        *        This function gets called by libcurl as soon as there is data received
        *    that needs to be saved.

        *        The size of the data pointed to by ptr is size multiplied with nmemb, 
        *    it will not be zero terminated. 

        * @param[in] 
        * @param[in]    
        * @param[in]    
        * @param[in] usrptr, the user ptr, 
        *            set by option CURLOPT_READDATA or CURLOPT_WRITEDATA...
        * @return    the number of bytes actually taken care of. 

        *        If that amount differs from the amount passed to your function,
        *    it'll signal an error to the library and it will abort the transfer 
        *    and return CURLE_WRITE_ERROR. 
        *****************************************************************************/
        static size_t processFunc( void* ptr, size_t size, size_t nmemb, void *usrptr );

    private:
        CURL * m_curl;
    };    
};

#endif