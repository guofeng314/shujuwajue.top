#include <math.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <map>
#include <set>
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

#include <string>
#include "hiredis.h"
#include "GFRedis.h"

using namespace std;
GFRedis::GFRedis(string h, int p)
{
	struct timeval timeout = {1, 500000};
	context = redisConnectWithTimeout(h.c_str(), p, timeout);	
	if(context == NULL)
	{
		printf("\nconnection can't setup!");
	}
	else
	{
		printf("\nconnection setup!");
	}
}

GFRedis::~GFRedis()
{
	redisFree(context);
}


vector<string> GFRedis::LRange(string key)
{
	vector<string> res;
	string cmds ="LRANGE " + key + " 0 -1";
	cout<<cmds<<endl;
	redisReply *reply= (redisReply*)redisCommand(context, cmds.c_str());
	if(reply != NULL && reply->type == REDIS_REPLY_ARRAY)
	{
		for (unsigned i = 0; i < reply->elements; ++i) 
		{
			redisReply* childReply = reply->element[i];
			if (childReply->type == REDIS_REPLY_STRING)
			{
				res.push_back(childReply->str);	
			}
			else
			{
			  res.push_back("");
			}
		}
		freeReplyObject(reply); 
	}
	return res;
}



bool GFRedis::HMSet(string key, map<string, string> kv)
{
	map<string, string>::iterator it;
	string cmds ="HMSET " + key;
	for(it = kv.begin(); it != kv.end(); it++)
	{
		cmds = cmds  + " " + (it->first + " " + it->second);
	}
	cout<<cmds<<endl;
	redisReply *reply= (redisReply*)redisCommand(context, cmds.c_str());
	if(reply != NULL && reply->type == REDIS_REPLY_STRING)
	{
		if(strcasecmp("ok", reply->str) == 0)
		{
			freeReplyObject(reply); 
			return true;
		}
	}
	return false;
}


map<string, string> GFRedis::HGetAll(string key)
{
	map<string, string> res;
	string cmds ="HGETALL " + key;
	cout<<cmds<<endl;
	redisReply *reply= (redisReply*)redisCommand(context, cmds.c_str());

	if(reply != NULL && reply->type == REDIS_REPLY_ARRAY )
	{
		unsigned i = 0;
		for (i = 0; i < reply->elements; i = i + 2 )
		{
			string k = reply->element[i]->str;
			string v = reply->element[i + 1]->str;
			res[k] = v;
		}
		freeReplyObject(reply); 
	}
	return res;
}

bool GFRedis::LPush(string k, string v)
{
	redisReply *reply;    
	string cmds = "LPUSH " + k + " " + v;
	reply= (redisReply*)redisCommand(context, cmds.c_str());  
	cout<<cmds<<endl;     
	if (reply != NULL)       
	{
		freeReplyObject(reply);           
		return false;
	}
	return true;
}

bool GFRedis::Set(string k, string v)
{
	redisReply *reply;    
	reply= (redisReply*)redisCommand(context, "SET %s %s", k.c_str(), v.c_str());       
	if (reply != NULL)       
	{
		freeReplyObject(reply);           
		return false;
	}
	return true;
}

string GFRedis::Get(string k)
{
	redisReply *reply;
	reply= (redisReply*)redisCommand(context, "GET %s", k.c_str());
	if(reply != NULL)
	{
		if(reply->type == REDIS_REPLY_STRING)
		{
			string v = reply->str;
			freeReplyObject(reply);        
			return v;	
		}
	}
	
	return "";
}



bool GFRedis::MSet(map<string, string> kv)
{
	map<string, string>::iterator it;
	string cmds ="MSET";
	for(it = kv.begin(); it != kv.end(); it++)
	{
		cmds = cmds  + " " + (it->first + " " + it->second);
	}
	cout<<cmds<<endl;
	redisReply *reply= (redisReply*)redisCommand(context, cmds.c_str());
	if(reply != NULL && reply->type == REDIS_REPLY_STRING)
	{
		if(strcasecmp("ok", reply->str) == 0)
		{
			return true;
		}
	}
	return false;
}


vector<string> GFRedis::MGet(vector<string> ks)
{
	vector<string> res;
  redisReply *reply;
	string bs;
	for(unsigned i = 0; i < ks.size(); i++)
	{
		if(i == 0)
		{
			bs = ks[0];
		}
		else
		{
			bs += " " + ks[i];
		}
	}
	cout << "MGET " + bs <<endl;
  //reply= (redisReply*)redisCommand(context, "MGET %s", bs.c_str());
  string cmd = string("MGET ") + bs;
  reply= (redisReply*)redisCommand(context, cmd.c_str());
  if(reply != NULL && reply->type == REDIS_REPLY_ARRAY)
  {
		for (unsigned i = 0; i < reply->elements; ++i) 
		{
			redisReply* childReply = reply->element[i];
			if (childReply->type == REDIS_REPLY_STRING)
			{
				res.push_back(childReply->str);	
			}
			else
			{
			  res.push_back("");
			}
		}
  }
  return res;
}


int GFRedis::dupReptids(map<std::string,std::string> reptidMap,std::string uid, int num,set<std::string> &repidSet)
{
	map<string,string>::iterator it;
	string cmds ="MGET";
	unsigned try_num = 50;
	unsigned res_num = 0;
	unsigned elem_num = reptidMap.size();
	unsigned ii = 0;
	cout<<"try_num:"<<try_num<<"res_num:"<<res_num<<"elem_num:"<<elem_num<<endl;
	redisReply *reply;
	map<string,string>::iterator last_it = reptidMap.begin(); 
	while(res_num < (unsigned)num)
	{
		cmds ="MGET";
		vector<string> ks;
		for(it = last_it; it != reptidMap.end() && ii < try_num && ii <  elem_num; ii++,it++)
		{
			cmds = cmds  + " " + (uid + it->first);
			ks.push_back(it->first);
		}
		cout<<cmds<<endl;
		reply= (redisReply*)redisCommand(context, cmds.c_str());
		if(reply != NULL && reply->type == REDIS_REPLY_ARRAY)
	        {
        	        for (unsigned i = 0; i < reply->elements; ++i)
                	{
				cout<<i<<endl;
                        	redisReply* childReply = reply->element[i];
	                        if (childReply == NULL || childReply->type == REDIS_REPLY_NIL) // childReply->type == REDIS_REPLY_STRING)
        	                {
                	        	repidSet.insert(ks[i]);
					res_num++;
					if(res_num >= (unsigned)num)
                			{
                        			break;
               				}
	                        }
        	                else
                	        {
                        		
	                        }
        	        }
	        }
		cout<<"de"<< res_num << " " << num <<endl;
		if(res_num >= (unsigned)num)
		{
			break;
		}
		if(try_num >= elem_num)
		{
			break;
		}
		try_num += 50;
		last_it = it;
	}
	
	return res_num;	
}

int GFRedis::writeReptids(map<std::string,std::string> reptidMap,std::string uid)
{
	map<string,string>::iterator it;
	string cmds ="MSET";
	for(it = reptidMap.begin(); it != reptidMap.end(); it++)
	{
		cmds = cmds  + " " + (uid + it->first + " " + it->second);
	}
	cout<<cmds<<endl;
	redisReply *reply= (redisReply*)redisCommand(context, cmds.c_str());
  if(reply != NULL && reply->type == REDIS_REPLY_STRING)
  {
      if(strcasecmp("ok", reply->str) == 0)
      {
              return true;
      }
  }
  return false;
}

Json::Value GFRedis::GetAdData(string user, string product, string pagetype, unsigned n)
{
	map<string,string> dupmap = HGetAll("dup_" + user);
	vector<string> list = LRange(product + "_" + pagetype);
	map<string, string> map;
	unsigned num = 0;
	Json::Value res_json;
	for (unsigned int i = 0; i < list.size(); ++i)
  	{
  	      string v = list[i];
	      cout<<i <<":"<<v<<endl;
	      Json::Value root;
	      Json::Reader reader;
	      if (!reader.parse(v.c_str(), root, false))
	      {
		   cout<<"parse error"<<endl;
        	   continue;
	      }
	      string url =   root["url"].asString();  
	      string times = root["times"].asString();
	      string occ = "";
	      std::map<string,string>::iterator it = dupmap.find(url);
	      if(it == dupmap.end())
	      {
		 occ = "1";
		 map[url] = occ;
                 res_json["result"].append(root);
	      }
	      else if(it->second.size() < times.size())
	      {
		   if(map.find(url)==map.end())
		   {
			occ = it->second + "1";
			map[url] = occ;
			res_json["result"].append(root);
		   }
	      }
	      else
	      {
		   continue;
	      }
	      if(++num >= n)
	      {
        	  break;
	      }		
  	}
	if(map.size() > 0)
	{
	     HMSet("dup_" + user, map);
	}
	return res_json;
}


