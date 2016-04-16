BINARYS = test_redis
all : $(BINARYS)

#m_path="/home/guofeng-pd/code/cpp/360seg"
m_path=/home/guofeng-pd/code/cpp/redis/



INCLUDES = -I. 


LIBS = $(m_path)/libhiredis.a $(m_path)/libjson.a

CMD ?= -O
CFLAGS = $(CMD) -Wall -fno-strict-aliasing  -Werror -fPIC -g


test_redis: test_redis.cpp GFRedis.cpp 
	g++ $(CFLAGS) $(ALIBDIR) -o $@ $^ $(INCLUDES) $(LIBS)


release:
	make clean;make CMD=-O5;
	make CMD=-O5

clean:
	rm -f *.o $(BINARYS)
