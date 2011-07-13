#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "zend_exceptions.h"

#include <stdio.h>
#include <stdint.h>
#include <signal.h>

#include "php_riak_pbc.h"
#include "riakclient.pb-c.h"

/* .h file included in php_riak_pbc.h */

static void php_riak_pbc_connect(INTERNAL_FUNCTION_PARAMETERS, int);
static void riak_pbc_exec_op(riak_pbc_conn_t *riak_pbc_connection, riak_pbc_op_t *op, riak_pbc_op_t *resp);

/*
#define TEST_LITTLE_ENDIAN 0
#define TEST_BIG_ENDIAN    1

int endian() {
    int i = 1;
    char *p = (char *)&i;

    if (p[0] == 1)
        return TEST_LITTLE_ENDIAN;
    else
        return TEST_BIG_ENDIAN;
} */

// destructor
void riak_pbc_dtor(void *object TSRMLS_DC)
{
	riak_pbc_conn_t *ob = (riak_pbc_conn_t*)object;

	//php_amqp_disconnect(ob);

	zend_object_std_dtor(&ob->zo TSRMLS_CC);

	efree(object);

}

// constructor
zend_object_value riak_pbc_ctor(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value new_value;
	riak_pbc_conn_t* obj = (riak_pbc_conn_t*)emalloc(sizeof(riak_pbc_conn_t));

	memset(obj, 0, sizeof(riak_pbc_conn_t));

	zend_object_std_init(&obj->zo, ce TSRMLS_CC);

	new_value.handle = zend_objects_store_put(obj, (zend_objects_store_dtor_t)zend_objects_destroy_object,
		(zend_objects_free_object_storage_t)riak_pbc_dtor, NULL TSRMLS_CC);
	new_value.handlers = zend_get_std_object_handlers();

	return new_value;
}

/*riak_pbc_t *riak_pbc_connection_new(riak_pbc_t *rpbc, char *host, int host_len, unsigned short port, int persistent) 
{

	// The pemalloc() family include a 'persistent' flag which allows them to behave like their non-persistent counterparts.
   	// For example: emalloc(1234) is the same as pemalloc(1234, 0)
   	
   	// switch to persistent later on
   	
	
	//riak_pbc_t *rpbc = pemalloc(sizeof(riak_pbc_t), persistent);
	rpbc = emalloc(sizeof(riak_pbc_t);
	memset(rpbc, 0, sizeof(*rpbc));
	
	//rpbc->host = pemalloc(host_len + 1, persistent);
	rpbc->host = emalloc(host_len + 1);
	memcpy(rpbc->host, host, host_len);
	rpbc->host[host_len] = '\0';
	
	rpbc->port = port;
	rpbc->persistent = persistent;
	
	return rpbc;

} */

void php_riak_pbc_connect(INTERNAL_FUNCTION_PARAMETERS, int persistent) // forget persistent connection for now
{

	zval *id;
	riak_pbc_conn_t *riak_pbc_connection;

	//zval *riak_pbc_object = getThis();
	
	int host_len, err=0;
	char *host;
	char *hostname = NULL;
	
	struct timeval tv;
	char *hash_key = NULL, *errstr = NULL;
	
	
	long port, timeout = 10, timeoutms = 10000;
	
	/* get connection object from parameters */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O", &id, riak_pbc_connection_class_entry) == FAILURE) {
		RETURN_FALSE;
	}
	
	/* Get the connection object out of the store */
	riak_pbc_connection = (riak_pbc_conn_t *)zend_object_store_get_object(id TSRMLS_CC);
	
	if(timeoutms < 1) {
		timeoutms = 30000;
	}
	
	// timeout of 10 seconds for now, until we allow configurable
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	// make socket connection 
	
	//riak_pbc = riak_pbc_server_new(host, host_len, port, 0);
	
	host_len = spprintf(&hostname, 0, "%s:%d", riak_pbc_connection->host, riak_pbc_connection->port);
	//host_len = riak_pbc_connection->host_len;
	
	riak_pbc_connection->stream = php_stream_xport_create( hostname, host_len,
		ENFORCE_SAFE_MODE | REPORT_ERRORS,
		STREAM_XPORT_CLIENT | STREAM_XPORT_CONNECT,
		hash_key, &tv, NULL, &errstr, &err);
		
	if(!riak_pbc_connection->stream) {
		//RIAK_PBC_DEBUG((printf("could not connect socket: %s", errstr))); // make a debug function
		char buff[100];
		RIAK_PBC_DEBUG((riak_pbc_connection->host));
		sprintf(buff,"Error opening stream %s", errstr);
		RIAK_PBC_DEBUG((buff));
		RETURN_FALSE;
	}
	
	php_stream_auto_cleanup(riak_pbc_connection->stream);
	php_stream_set_option(riak_pbc_connection->stream, PHP_STREAM_OPTION_READ_TIMEOUT, 0, &tv);
	php_stream_set_option(riak_pbc_connection->stream, PHP_STREAM_OPTION_WRITE_BUFFER, PHP_STREAM_BUFFER_NONE, NULL);
	php_stream_set_chunk_size(riak_pbc_connection->stream, 8192);
	
	/*if(!riak_pbc_object) { // if getThis() is null, we need to initialize the pointer entry?
		object_init_ex(return_value, riak_pbc_ce_ptr);
		add_property_zval(return_value, "riak_pbc_conn", riak_pbc_connection);
	} else { // before we should check if it already exists, and handle that
		add_property_zval(riak_pbc_object, "riak_pbc_conn", riak_pbc_connection);
		RETURN_TRUE;
	}*/
	
	RETURN_TRUE;
	
}


/**
 * Defined PHP_METHODs
 *
 **/

PHP_METHOD(riak_pbc_connection_class, __construct) 
{

	zval *id;
	riak_pbc_conn_t *riak_pbc_connection;
	
	char *host;
	int host_len;
	long port;
	
	/* Parse out the method parameters */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Osl", &id, riak_pbc_connection_class_entry, &host, &host_len, &port) == FAILURE) {
		zend_throw_exception(zend_exception_get_default(TSRMLS_C), "parse parameter error", 0 TSRMLS_CC);
		return;
	}
	
	riak_pbc_connection = (riak_pbc_conn_t *)zend_object_store_get_object(id TSRMLS_CC);
	
	// do we need to do some malloc here?
	riak_pbc_connection->host = emalloc(host_len + 1);
	memcpy(riak_pbc_connection->host, host, host_len);
	riak_pbc_connection->host[host_len] = '\0';
	riak_pbc_connection->host_len = host_len;
	riak_pbc_connection->port = port;
	
}

void riak_pbc_exec_op(riak_pbc_conn_t *riak_pbc_connection, riak_pbc_op_t *op, riak_pbc_op_t *resp) {
	// need to write the op codes, etc to heah
	// <length:32>  <msg_code:8> <pbmsg>
	__uint32_t *length;
	__uint8_t *cmdcode;
	
	RpbGetReq *msg;
	
	unsigned len;
	
	length = htonl(op->len + 1);
	
	// length of our message to send, and pbc 
	//length = htonl(len + 1);
	//cmdcode = 9;
	
	/* write request */
	if(php_stream_write(riak_pbc_connection->stream, &length, 4) != 4) {
		RIAK_PBC_DEBUG(("did not write length bytes correctly"));
		return;
	}
	if(php_stream_write(riak_pbc_connection->stream, &op->code, 1) != 1) {
		RIAK_PBC_DEBUG(("did not write cmdcode bytes correctly"));
		return;
	}
	if(php_stream_write(riak_pbc_connection->stream, op->msg, op->len) != op->len) {
		RIAK_PBC_DEBUG(("did not write all msg bytes"));
		return;
	}
	char respSize[4];
	
	if(php_stream_read(riak_pbc_connection->stream, respSize, 4) != 4){
		RIAK_PBC_DEBUG(("did not read the correct bytes"));
		return;
	}
	
	/* read response back */
	int i=0, msg_size=0;
	for(i=0;i<4;i++)
		msg_size =(msg_size<<8) | (respSize[i]);
	
	printf("msg length %u\n", msg_size);
	
	char respCode[1];
	if(php_stream_read(riak_pbc_connection->stream, respCode, 1) != 1){
		RIAK_PBC_DEBUG(("did not read correct bytes for msg code"));
		return;
	}
	
	int msg_code=0;
	for(i = 0; i < 1; i++)
		msg_code =(msg_code<<8) | (respCode[i]);
		
	printf("msg code %u\n", msg_code);
	
	char *msgData = malloc(msg_size - 1);
	
	//char msgData[msg_size - 1];
	
	if(php_stream_read(riak_pbc_connection->stream, msgData, msg_size - 1) != msg_size - 1){
		RIAK_PBC_DEBUG(("did not read correct bytes for msg code"));
		return;
	}
	size_t resp_len = msg_size - 1;
	printf("response code %i\n", msg_code);
	resp->len = resp_len;
	resp->code = msg_code;
	resp->msg = (void *)msgData;
	
	//rpb_get_resp__unpack(NULL, resp->len, resp->msg);
	
}


// we will make another class called riakpbcobject. we will shove the results from here into that object. boom
PHP_METHOD(riak_pbc_connection_class, get) 
{
	zval *id;
	riak_pbc_conn_t *riak_pbc_connection;
	php_stream *s;

	char *bucket;
	char *key;
	int bucket_len;
	int key_len;
	zval *optArr = NULL;
	
	// pb vars
	void *buf;
	unsigned len;
	
//	printf("endianness %i\n", endian());
	
	/* get connection object from parameters */
	if (zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Oss|a", &id, riak_pbc_connection_class_entry, &bucket, &bucket_len, &key, &key_len, &optArr) == FAILURE) {
		zend_throw_exception(zend_exception_get_default(TSRMLS_C), "parse parameter error", 0 TSRMLS_CC);
		return;
	}
	
	riak_pbc_connection = (riak_pbc_conn_t *)zend_object_store_get_object(id TSRMLS_CC);
	
	RpbGetReq msg = RPB_GET_REQ__INIT;
	
	ProtobufCBinaryData bucket_struct = { bucket_len, (uint8_t*) bucket };
	ProtobufCBinaryData key_struct = { key_len, (uint8_t*) key };
	
	msg.bucket = bucket_struct;
	msg.key = key_struct;
	
	len = rpb_get_req__get_packed_size(&msg);
	buf = malloc(len);
	
	rpb_get_req__pack(&msg, buf);
	
	riak_pbc_op_t op, result;
	
	op.code = RPB_GET_REQ;
	op.len = len;
	op.msg = buf;
	
	riak_pbc_exec_op(riak_pbc_connection, &op, &result);
	
	//free(buf);
	
	RpbGetResp *respMsg;
	//size_t resp_len = msg_size - 1;
	
	printf("return code %i\n", result.code);
	
	respMsg = rpb_get_resp__unpack(NULL, result.len, result.msg);
	
	// format is this n_content -> number of contents
	// RpbContent *content = respMsg->content[n_content-1] (n_content is id)
	// TODO: number of contents should be n_content
	
	if(respMsg->n_content > 0) {
	
		RpbContent *content = respMsg->content[0];

		ProtobufCBinaryData val;
	
		val = content->value;
		printf("val: %s\n", val.data);
		size_t valuelen;
	
		char *valstr;
		int retval;
	
	} else {
		printf("not found\n");
	}
	rpb_get_resp__free_unpacked(respMsg, NULL);
	
	
	
	//free(valstr);
	
	//RIAK_PBC_DEBUG((respSize));
	
	//RIAK_PBC_DEBUG((bucket));
	//RIAK_PBC_DEBUG((key));
	
	//free(msg.bucket);
	//free(msg.key);
	//free(buf);
	
	return;
}

PHP_METHOD(riak_pbc_connection_class, connect)
{
	php_riak_pbc_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}

PHP_METHOD(riak_pbc_connection_class, pconnect)
{
	php_riak_pbc_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}

