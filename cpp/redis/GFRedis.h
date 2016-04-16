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
#include "json/json.h"

using namespace std;
class GFRedis
{
	public:
	GFRedis(string h, int port);
	~GFRedis();
	Json::Value GetAdData(string user, string product, string pagetype, unsigned n);
	bool Set(string k, string v);
	string Get(string k);
	bool LPush(string k, string v);
	map<string, string> HGetAll(string k);
	bool HMSet(string key, map<string, string> kv);
	vector<string> LRange(string key);
	vector<string> MGet(vector<string> ks);
	bool MSet(map<string, string> kv);
	int dupReptids(map<std::string,std::string> reptidMap,std::string uid, int num,set<std::string> &repidSet);
	int writeReptids(map<std::string,std::string> reptidMap,std::string uid);
	private:
	redisContext* context;
	string host;    
	int    port;    
	string pass;
};
