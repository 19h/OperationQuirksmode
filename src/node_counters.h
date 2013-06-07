

#ifndef NODE_COUNTERS_H_
#define NODE_COUNTERS_H_

#include "node.h"
#include "v8.h"

namespace node {

void InitPerfCounters(v8::Handle<v8::Object> target);
void TermPerfCounters(v8::Handle<v8::Object> target);

}

#ifdef HAVE_PERFCTR
#include "node_win32_perfctr_provider.h"
#else
#define NODE_COUNTER_ENABLED() (false)
#define NODE_COUNT_HTTP_SERVER_REQUEST()
#define NODE_COUNT_HTTP_SERVER_RESPONSE()
#define NODE_COUNT_HTTP_CLIENT_REQUEST()
#define NODE_COUNT_HTTP_CLIENT_RESPONSE()
#define NODE_COUNT_SERVER_CONN_OPEN()
#define NODE_COUNT_SERVER_CONN_CLOSE()
#define NODE_COUNT_NET_BYTES_SENT(bytes)
#define NODE_COUNT_NET_BYTES_RECV(bytes)
#define NODE_COUNT_GET_GC_RAWTIME()
#define NODE_COUNT_GC_PERCENTTIME()
#define NODE_COUNT_PIPE_BYTES_SENT(bytes)
#define NODE_COUNT_PIPE_BYTES_RECV(bytes)
#endif

#endif
