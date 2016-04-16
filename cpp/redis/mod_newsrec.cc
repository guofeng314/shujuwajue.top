#include <math.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <map>
#include <tr1/functional>
#include <unistd.h>
#include <sys/stat.h>
#include <tr1/unordered_map>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#include "segmenter.h"
#include "/home/guofeng-pd/code/cpp/360seg/qmodule/segment/include/config.h"
#include "string.h"
#include "transcode.h"
//#include "segbase.h"

#include <vector>
#include <string>
#include "logging.h"
#include "module.h"
#include "downstream/memcached/server.h"
#include "downstream/memcached/request.h"
#include "downstream/memcached/response.h"
#include "downstream/http/server.h"
#include "downstream/http/request.h"
#include "downstream/http/response.h"
#include "boost/shared_ptr.hpp"
#include "hiredis/hiredis.h"
#include "json/json.h"
#include <stdlib.h>
#include <sys/time.h>



LOGGING_INIT("mod_newsrec");

using std::cout;
using std::endl;
using std::vector;
using std::string;

using namespace cloud_ucs;
using namespace std::tr1::placeholders;
using namespace std;




unsigned char fromHex(const unsigned char &x)  
{  
	return isdigit(x) ? x-'0' : x-'A'+10;  
}  
std::string URLDecode(const std::string &sIn)  
{  
	std::string sOut;  
	for( size_t ix = 0; ix < sIn.size(); ix++ )  
	{  
		unsigned char ch = 0;  
		if(sIn[ix]=='%')  
		{  
			ch = (fromHex(sIn[ix+1])<<4);  
			ch |= fromHex(sIn[ix+2]);  
			ix += 2;  
		}  
		else if(sIn[ix] == '+')  
		{  
			ch = ' ';  
		}  
		else  
		{  
			ch = sIn[ix];  
		}  
		sOut += (char)ch;  
	}  
	return sOut;  
}
std::vector<std::string> split_msp(const std::string &line, std::string sps)
{
        std::vector<std::string> vec;
        std::set<char> chs;
        unsigned int size = sps.size();
        for(unsigned int i = 0; i < size; i++)
        {
            chs.insert(sps[i]);
        }
        size = line.size(); 
        int begin = 0;
        int pos = 0;
        for(unsigned int i = 0; i < size; i++)
        {
            char ch = line[i];
            if(chs.find(ch) != chs.end())
            {
                pos = i;
                if(pos == begin)
                {
                    vec.push_back("");
                }
                else
                {
                    vec.push_back(line.substr(begin,pos - begin));
                }
                begin = pos + 1;
            }

        }
        vec.push_back(line.substr(begin, line.size() -begin));
        return vec;
}
std::string strim(const std::string & line_in)
{
	    unsigned int begin = 0;
	    unsigned int end = line_in.size() - 1;
	    unsigned int i = 0;
	    for(i = begin; i >= 0 && i < line_in.size() && (ispunct(line_in[i]) == true); i++)
	    {
	        begin++;
	    }
	                        
	    for(i = end; i >= 0 && i < line_in.size() && (ispunct(line_in[i]) == true); i--)
	    {
	        end--;
	    }
	    if(end <= begin)
	    {
	        return "";
	    }
	    else
	    {
	        return line_in.substr(begin, end - begin + 1);
	    }
}
std::string mytrim(const std::string & line_in)
{
    unsigned int begin = 0;
    unsigned int end = line_in.size() - 1;
    unsigned int i = 0;
    for(i = begin; i >= 0 && i < line_in.size() && (line_in[i] == ' ' || line_in[i] == '\t' || line_in[i] == '\r' || line_in[i] == '\n'); i++)
    {
        begin++;
    }
                        
    for(i = end; i >= 0 && i < line_in.size() && (line_in[i] == ' ' || line_in[i] == '\t' || line_in[i] == '\r' || line_in[i] == '\n'); i--)
    {
        end--;
    }
    if(end <= begin)
    {
        return "";
    }
    else
    {
        return line_in.substr(begin, end - begin + 1);
    }
}


std::vector<std::string> split(const std::string & line_in, std::string dem)
{
        std::vector<std::string> vec;
        size_t pos;
        size_t begin = 0;
        while((pos = line_in.find(dem, begin)) != std::string::npos )
        {
            if(pos != begin)
            {
                 vec.push_back(line_in.substr(begin, pos - begin));
            }
            begin = pos + 1;
        }
        vec.push_back(line_in.substr(begin, line_in.size() - begin));
        return vec;
}

	
std::map<std::string,std::string> LoadMap(std::string file, std::string seg)
{
	    std::map<std::string,std::string> dataset;
	    ifstream inFile;
	    std::string in_line;
	    inFile.open(file.c_str(), ios::in);
	    while(getline(inFile, in_line))
	    {
	        //dataset.insert(in_line);
	        std::vector<std::string> ss = split(in_line,seg);
	        if(ss.size() == 2)
	        {
	            dataset.insert(make_pair(ss[0],ss[1]));
	        }
	    }
	    inFile.close();
	    return dataset;
}

struct NEWS_WEIGHT
{
    Json::Value news;
    float weight;
};
void SortTop(NEWS_WEIGHT heap[], int n)
{
    NEWS_WEIGHT tmp;
    int max_p = 0;
    for(int i = 0; i < n - 1; i++)
    {
        max_p = i;
        for(int j = i+ 1; j < n ; j++)
        {
            if(heap[max_p].weight < heap[j].weight)
            {
                max_p = j;
            }
        }
        tmp = heap[i];
        heap[i] = heap[max_p];
        heap[max_p] = tmp;    
    }
}
void Adjust(NEWS_WEIGHT heap[], int n, NEWS_WEIGHT a)
{
    if(heap[0].weight >= a.weight)
    {
        return;
    }
    int p = 0;
    int sign = 0;
    heap[p] = a;
    while(sign < 2)
    {
        if(2 * p + 1 >= n)
        {
           break;
        }
        if(2 * p + 2 >=n)
        {
            int q = 2 * p + 1;
            if(q >= n || heap[q].weight >= heap[p].weight)
            {
                 sign++;
            }
            else
            {
                NEWS_WEIGHT tmp = heap[q];
                heap[q] =  heap[p];
                heap[p] = tmp;
                p = q;
                sign = 0;
            }
        }
        else
        {
            int min_child = heap[2 * p + 2].weight <=  heap[2 * p + 1].weight ? 2 * p + 2 : 2 * p + 1;
            int max_child = heap[2 * p + 2].weight >   heap[2 * p + 1].weight ? 2 * p + 2 : 2 * p + 1;
            int q = min_child;
            if(q >= n || heap[q].weight >= heap[p].weight)
            {
                sign++;
            }
            else
            {
                NEWS_WEIGHT tmp = heap[q];
                heap[q] =  heap[p];
                heap[p] = tmp;
                p = q;
                sign = 0;
            }
            q = max_child;
            if(q >= n || heap[q].weight >= heap[p].weight)
            {
                sign++;
            }
            else
            {
                NEWS_WEIGHT tmp = heap[q];
                heap[q] =  heap[p];
                heap[p] = tmp;
                p = q;
                sign = 0;
            }

        }
    }
}


class Mod_NewsRecModule : public cloud_ucs::Module
{
    public:
        Mod_NewsRecModule();
        ~Mod_NewsRecModule();

        bool Init(const std::string& conf, int *thread_num);

        void Segment(const std::map<std::string, std::string>& params, const cloud_ucs::InvokeCompleteHandler& cb);
        void GetResult(const std::map<std::string, std::string>& params, const cloud_ucs::InvokeCompleteHandler& cb);
        void Status(const map<string, string>& params, const InvokeCompleteHandler& cb);

        void HandleGetResult(cloud_ucs::downstream::ErrCode ec, 
                boost::shared_ptr<cloud_ucs::downstream::http::GetResponse> response, 
                const cloud_ucs::InvokeCompleteHandler& complete_handler, 
                boost::shared_ptr<cloud_ucs::downstream::http::GetRequest> request);

        void HandleStatus(downstream::ErrCode ec, 
                boost::shared_ptr<downstream::http::GetResponse> response, 
                const InvokeCompleteHandler& complete_handler, 
                boost::shared_ptr<downstream::http::GetRequest> request);
				void HandleResponse(downstream::ErrCode ec,
                boost::shared_ptr<downstream::http::GetResponse> response,
                    const InvokeCompleteHandler& complete_handler,
                boost::shared_ptr<downstream::http::PostRequest> request);
    private:
    cloud_ucs::downstream::http::Server* baidu_server_;
	boost::shared_ptr<cloud_ucs::downstream::http::Server> sptr_baidu_server_;
	//DTreeClass<word_index_t> *ptree;
    
    string news_host;
    int news_port;
    string news_pass;

    redisContext* news_redis_context_;
    qss::segmenter::Segmenter *segmenter;
};


Mod_NewsRecModule::Mod_NewsRecModule()
{
    news_redis_context_ = NULL;
}

Mod_NewsRecModule::~Mod_NewsRecModule()
{
//    delete baidu_server_;
    
/*    if (ptree != NULL) {
        delete ptree;
        ptree = NULL;
    }
    if (parray != NULL) {
        free(parray);
        parray = NULL;
    }
*/
}


bool Mod_NewsRecModule::Init(const std::string& conf, int* thread_num)
{
    *thread_num = 1;
    
    ///
    LOG(INFO)<<"load config from "<<conf<<"\n";
    ifstream in_file(conf.c_str());
    string in_line;
    map<string, string> map_config;
    while(getline(in_file, in_line))
    {
        if(in_line[0] != '#')
        {
          vector<string> vec_pair = split(in_line, "=");
          if(vec_pair.size() == 2)
          {
             map_config.insert(make_pair(vec_pair[0], vec_pair[1]));
          }
        }
    }
	  
   in_file.close();
   LOG(DEBUG) << "TESTModule::Init conf: end " << conf;
   Register("Segment", std::tr1::bind(&Mod_NewsRecModule::Segment, this, _1, _2));
   Register("Status", std::tr1::bind(&Mod_NewsRecModule::Status, this, _1, _2));
   LOG(INFO) << "Init succeed" << "\n" ;
   string qsegconf = "/home/guofeng-pd/code/cpp/360seg/conf/qsegconf.ini";
   std::map<std::string, std::string>::const_iterator iter = map_config.find("qsegconf");
   if(iter != map_config.end())
   {
   	qsegconf = iter->second;
   }

   qss::segmenter::Config::get_instance()->init(qsegconf.c_str());
   segmenter = qss::segmenter::CreateSegmenter();   
   return true;
}

void Mod_NewsRecModule::Segment(const std::map<std::string, std::string>& params, const cloud_ucs::InvokeCompleteHandler& cb)
{
    LOG(INFO) <<"news_rec begin"<<"\n";
    std::map<std::string, std::string> res;
	  cloud_ucs::InvokeResult result;
    string mid = "";
    std::map<std::string, std::string>::const_iterator iter = params.find("mid");
    if ((iter == params.end()))
    {
        res["result"] = string("{\"result\":[]}");
	result.__set_results(res);
	cb(result);
	return;
    }
    int json = 0;
    std::map<std::string, std::string>::const_iterator iter_j = params.find("json");
    if ((iter_j != params.end()))
    {
       json = 1;
    }
    char *buf = new char[10240];
    char *obuf = new char[327680];
    uint16_t *wbuf = new uint16_t[10240];
    uint16_t *tbuf = new uint16_t[32768];
    
    strcpy(buf, iter->second.c_str());
    int n = strlen(buf);
    while(n > 0 && isspace(buf[n-1])) { buf[--n] = '\0'; }
    int len = convToucs2(buf, n, wbuf, 10240, qsrch_code_utf8);
    wbuf[len] = 0;
    len = segmenter->segmentUnicode(wbuf, len, tbuf, 327680);
    int l = convFromucs2(tbuf, len, obuf, 327680, qsrch_code_utf8);
    while(l > 0 && isspace(obuf[l-1])) { obuf[--l] = '\0'; }
    obuf[l] = '\0';
    if(obuf[l-1] == '\\')
    {
	obuf[l-1] = '\0';
    }
    //string segres = obuf;
    //string resstr = "{\"res\":\"" + segres +"\"}";
    if(json == 0)
    {
    	res["result"] = obuf;	
    }
    else
    {
        Json::Value res_json;
        res_json["seg"]= obuf;
        Json::FastWriter fast_writer;
	res["result"] = fast_writer.write(res_json);
    }
    result.__set_results(res);
    cb(result);
}

void Mod_NewsRecModule::HandleResponse(downstream::ErrCode ec,
         boost::shared_ptr<downstream::http::GetResponse> response,
        const InvokeCompleteHandler& complete_handler,
        boost::shared_ptr<downstream::http::PostRequest> request) {
    LOG(TRACE) << "HandleResponse";
     cloud_ucs::InvokeResult result;
     std::map<std::string, std::string> res;
     string resultstr;

     if (ec != downstream::OK || response->code() != 200) {
		 LOG(DEBUG)<<"FILTER FAILED:"<<resultstr;
        res["result"] = "";
        result.__set_results(res);
         complete_handler(result);
        return ;
     }
     istringstream stream(response->body());
     string line;
     int i = 0;
     while (getline(stream, line)) 
     {
         size_t pos = line.find("\t");
        if (pos == string::npos) {
             continue;
         }
         string query = line.substr(0, pos);
         int type = atoi(line.substr(pos+1).c_str());
         if (type == 0 or type == 100) {
            if (i == 0)
                 resultstr += query;
             else
                 resultstr += "\n" + query;
             i++;
         }
     }
 
     LOG(DEBUG)<<"AFTER FILTER:"<<resultstr;
     res["result"] = resultstr;
     result.__set_results(res);
     complete_handler(result);
 }

void Mod_NewsRecModule::Status(const map<string, string>& params, const InvokeCompleteHandler& cb)
{
	LOG(TRACE) << "Mod_NewsRecModule::Status()";
  	cloud_ucs::InvokeResult result; 
        std::map<std::string, std::string> res;
        res["result"] = string("ok");   
        result.__set_results(res);    
        cb(result);

	/*
	boost::shared_ptr<downstream::http::GetRequest> req_engine(
			new downstream::http::GetRequest("/status"));
	req_engine->AddHeader("Host", "baidu.com");
	req_engine->Execute(sptr_baidu_server_.get(), cb,
			std::tr1::bind(&Mod_NewsRecModule::HandleStatus, this,
				_1, _2, _3,
				req_engine));*/

}


EXPORT_MODULE(Mod_NewsRecModule)
