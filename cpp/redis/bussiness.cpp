﻿#include <math.h>
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
using namespace std;



string GetAdData(string user, string product, string pagetype, int n)
{
	map<string,string> dupmap = bussiness_redis.HGetAll("dup_" + user);
	vector<string> list = bussiness_redis.LRange(product + "_" + pagetype, 0, -1);
	map<string, string> map;
	int i = 0;
	Json::Value res_json;
  for (unsigned int i = 0; i < list.size(); ++i)
  {
  		string v = list[i];
      Json::Value root;
      if (!reader.parse(v.c_str(), root, false))
      {
           continue;
      }
      string url =   root["url"].asString();  
			string times = root["times"].asString();
			map<string, string>::iterator it;
			string occ = "";
			it = dupmap.find(url);
			if(it == dupmap.end())
			{
				 occ = "1";
			}
			else if(it->second.size() < times.size())
			{
				if(map.get(url) == null)
				{
					map.put(url, occ);
					res_json["result"].append(root);
				}
				occ += "1";
			}
			else
			{
				 continue;
			}
			if(++i >= n)
			{
				break;
			}		
  }
  
	if(map.size() > 0)
	{
		bussiness_redis.jedis.HMSet("dup_" + user, map);
	}
	return res_json;
}


