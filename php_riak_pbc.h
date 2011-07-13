#ifndef PHP_RIAK_PBC_H
#define PHP_RIAK_PBC_H

/* Add pseudo refcount macros for PHP version < 5.3 */
#ifndef Z_REFCOUNT_PP

#define Z_REFCOUNT_PP(ppz)				Z_REFCOUNT_P(*(ppz))
#define Z_SET_REFCOUNT_PP(ppz, rc)		Z_SET_REFCOUNT_P(*(ppz), rc)
#define Z_ADDREF_PP(ppz)				Z_ADDREF_P(*(ppz))
#define Z_DELREF_PP(ppz)				Z_DELREF_P(*(ppz))
#define Z_ISREF_PP(ppz)					Z_ISREF_P(*(ppz))
#define Z_SET_ISREF_PP(ppz)				Z_SET_ISREF_P(*(ppz))
#define Z_UNSET_ISREF_PP(ppz)			Z_UNSET_ISREF_P(*(ppz))
#define Z_SET_ISREF_TO_PP(ppz, isref)	Z_SET_ISREF_TO_P(*(ppz), isref)

#define Z_REFCOUNT_P(pz)				zval_refcount_p(pz)
#define Z_SET_REFCOUNT_P(pz, rc)		zval_set_refcount_p(pz, rc)
#define Z_ADDREF_P(pz)					zval_addref_p(pz)
#define Z_DELREF_P(pz)					zval_delref_p(pz)
#define Z_ISREF_P(pz)					zval_isref_p(pz)
#define Z_SET_ISREF_P(pz)				zval_set_isref_p(pz)
#define Z_UNSET_ISREF_P(pz)				zval_unset_isref_p(pz)
#define Z_SET_ISREF_TO_P(pz, isref)		zval_set_isref_to_p(pz, isref)

#define Z_REFCOUNT(z)					Z_REFCOUNT_P(&(z))
#define Z_SET_REFCOUNT(z, rc)			Z_SET_REFCOUNT_P(&(z), rc)
#define Z_ADDREF(z)						Z_ADDREF_P(&(z))
#define Z_DELREF(z)						Z_DELREF_P(&(z))
#define Z_ISREF(z)						Z_ISREF_P(&(z))
#define Z_SET_ISREF(z)					Z_SET_ISREF_P(&(z))
#define Z_UNSET_ISREF(z)				Z_UNSET_ISREF_P(&(z))
#define Z_SET_ISREF_TO(z, isref)		Z_SET_ISREF_TO_P(&(z), isref)

#if defined(__GNUC__)
#define zend_always_inline inline __attribute__((always_inline))
#elif defined(_MSC_VER)
#define zend_always_inline __forceinline
#else
#define zend_always_inline inline
#endif

static zend_always_inline zend_uint zval_refcount_p(zval* pz) {
	return pz->refcount;
}

static zend_always_inline zend_uint zval_set_refcount_p(zval* pz, zend_uint rc) {
	return pz->refcount = rc;
}

static zend_always_inline zend_uint zval_addref_p(zval* pz) {
	return ++pz->refcount;
}

static zend_always_inline zend_uint zval_delref_p(zval* pz) {
	return --pz->refcount;
}

static zend_always_inline zend_bool zval_isref_p(zval* pz) {
	return pz->is_ref;
}

static zend_always_inline zend_bool zval_set_isref_p(zval* pz) {
	return pz->is_ref = 1;
}

static zend_always_inline zend_bool zval_unset_isref_p(zval* pz) {
	return pz->is_ref = 0;
}

static zend_always_inline zend_bool zval_set_isref_to_p(zval* pz, zend_bool isref) {
	return pz->is_ref = isref;
}

#else

#define PHP_ATLEAST_5_3   true

#endif

extern zend_module_entry riak_pbc_module_entry;
#define phpext_riak_pbc_ptr &riak_pbc_module_entry;

#ifdef PHP_WIN32
#define PHP_RIAK_PBC_API __declspec(dllexport)
#else
#define PHP_RIAK_PBC_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#define RIAK_PBC_WORLD_EXTVER "0.1"
#define RIAK_PBC_WORLD_EXTNAME "riak_pbc"

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 8087

#define RPB_ERROR_RESP  			0
#define RPB_PING_REQ 				1
#define RPB_PING_RESP  				2 
#define RPB_GET_CLIEN_ID_REQ 		3
#define RPB_GET_CLIENT_ID_RESP 		4
#define RPB_SET_CLIENT_ID_REQ 		5
#define RPB_SET_CLIENT_ID_RESP 		6
#define RPB_GET_SERVER_INFO_REQ 	7
#define RPB_GET_SERVER_INFO_RESP 	8
#define RPB_GET_REQ 				9
#define RPB_GET_RESP 				10
#define RPB_PUT_REQ 				11
#define RPB_PUT_RESP 				12
#define RPB_DEL_REQ 				13
#define RPB_DEL_RESP 				14
#define RPB_LIST_BUCKETS_REQ 		15
#define RPB_LIST_BUCKETS_RESP 		16
#define RPB_LIST_KEYS_REQ 			17
#define RPB_LIST_KEUS_RESP 			18
#define RPB_GET_BUCKET_REQ 			19
#define RPB_GET_BUCKET_RESP 		20
#define RPB_SET_BUCKET_REQ 			21
#define RPB_SET_BUCKET_RESP 		22
#define RPB_MAP_RED_REQ 			23
#define RPB_MAP_RED_RESP 			24



PHP_MINIT_FUNCTION(riak_pbc);
PHP_MSHUTDOWN_FUNCTION(riak_pbc);
PHP_MINFO_FUNCTION(riak_pbc);

void riak_pbc_debug(const char *format, ...);

// add class entries
extern zend_class_entry *riak_pbc_connection_class_entry;

// we will need to have a pool of connection identifiers for the persistent connections later
typedef struct riak_pbc_conn {

	zend_object zo;
	/** php stream **/
	php_stream *stream;
	/** riak host **/
	char * host;
	/** host length **/
	int host_len;
	/** riak port **/
	int port;
	/** Socket descriptor for Protocol Buffers connection **/
	int socket;
	/** Error code of last operation. Codes can be found in riakerrors.h **/
	int last_error;
	/** Riak internal error message. Only some operations return this message. Format: "(err code in hex): err msg" **/
	char * error_msg;
	int connect_timeout;
	int connect_timeout_ms;
	/** persistent **/
	int persistent;
	
} riak_pbc_conn_t;

typedef struct riak_pbc_op {
	/** Length of command. Equals length of msg + 1 (1 byte for msg code). */
	__uint32_t len;
	/** Message code. Defined in Riak API, also respective defines are in riakcodes.h. */
	__uint8_t code;
	/** Additional message data. Should be NULL if request doesn't pass any additional data. */
	void * msg;
} riak_pbc_op_t;

#ifdef ZTS
#define RIAK_PBC_G(v) TSRMG(riak_pbc_globals_id, zend_riak_pbc_globals *, v)
#else
#define RIAK_PBV_G(v) (riak_pbc_globals.v)
#endif
	
#define RIAK_PBC_DEBUG(info) \
{\
	riak_pbc_debug info; \
}\

#endif /* PHP_RIAK_PBC_H */