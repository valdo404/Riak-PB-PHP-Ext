
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "zend_exceptions.h"

#include <stdint.h>
#include <signal.h>
#include <protobuf-c.h>

#include "php_riak_pbc.h"
#include "riak_pbc_connection.h"

#include <unistd.h>

// object handlers
//zend_object_handlers riak_pbc_connection_handlers;

zend_class_entry *riak_pbc_connection_class_entry;

/* host, port */
ZEND_BEGIN_ARG_INFO_EX(arginfo_riak_pbc_connection_class__construct, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 2)
	ZEND_ARG_INFO(0, host)
	ZEND_ARG_INFO(0, port)
ZEND_END_ARG_INFO()

/* connect takes no parameters */
ZEND_BEGIN_ARG_INFO_EX(arginfo_riak_pbc_connection_class_connect, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

/* pconnect takes no parameters */
ZEND_BEGIN_ARG_INFO_EX(arginfo_riak_pbc_connection_class_pconnect, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
ZEND_END_ARG_INFO()

/* get takes at least two parameters, bucket and key, with a third options parameter */
ZEND_BEGIN_ARG_INFO_EX(arginfo_riak_pbc_connection_class_get, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 2)
	ZEND_ARG_INFO(0, bucket)
	ZEND_ARG_INFO(0, host)
	ZEND_ARG_ARRAY_INFO(0, options, 0)
ZEND_END_ARG_INFO()

/*
 * riak_pbc functions
 */
zend_function_entry riak_pbc_connection_class_functions[] = {
	PHP_ME(riak_pbc_connection_class, __construct, arginfo_riak_pbc_connection_class__construct, ZEND_ACC_PUBLIC)
	PHP_ME(riak_pbc_connection_class, connect, arginfo_riak_pbc_connection_class_connect, ZEND_ACC_PUBLIC)
	PHP_ME(riak_pbc_connection_class, pconnect, arginfo_riak_pbc_connection_class_pconnect, ZEND_ACC_PUBLIC)
	PHP_ME(riak_pbc_connection_class, get, arginfo_riak_pbc_connection_class_get, ZEND_ACC_PUBLIC)
	
	{NULL, NULL, NULL}	/* Must be the last line in riak_pbc_connection_class_functions[] */
};

zend_function_entry riak_pbc_functions[] = {
	{NULL, NULL, NULL}	/* Must be the last line in riak_pbc_functions[] */
};

// module init
zend_module_entry riak_pbc_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "riak_pbc",
	riak_pbc_functions,
	PHP_MINIT(riak_pbc),
	PHP_MSHUTDOWN(riak_pbc),
	NULL,
	NULL,
	PHP_MINFO(riak_pbc),
#if ZEND_MODULE_API_NO >= 20010901
    RIAK_PBC_WORLD_EXTVER,
#endif
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_RIAK_PBC
	ZEND_GET_MODULE(riak_pbc)
#endif

void riak_pbc_debug(const char *format, ...) /* {{{ */
{
	TSRMLS_FETCH();
//	if (RIAK_PBC_G(debug_mode)) { // globals
		char buffer[1024];
		va_list args;

		va_start(args, format);
		vsnprintf(buffer, sizeof(buffer)-1, format, args);
		va_end(args);
		buffer[sizeof(buffer)-1] = '\0';
		php_printf("%s\n", buffer);
//	}
}

PHP_INI_BEGIN()
	PHP_INI_ENTRY("riak_pbc.host", DEFAULT_HOST, PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("riak_pbc.port", DEFAULT_PORT, PHP_INI_ALL, NULL)
PHP_INI_END()

// extension init funtion
PHP_MINIT_FUNCTION(riak_pbc)
{
	zend_class_entry ce;
	
	INIT_CLASS_ENTRY(ce, "RiakPbcConnection", riak_pbc_connection_class_functions);
	ce.create_object = riak_pbc_ctor;
	riak_pbc_connection_class_entry = zend_register_internal_class(&ce TSRMLS_CC);
	
	REGISTER_INI_ENTRIES();
	
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(riak_pbc)
{
	UNREGISTER_INI_ENTRIES();
	
	return SUCCESS;
}

PHP_MINFO_FUNCTION(riak_pbc)
{
	/* Build date time from compiler macros */
	char datetime[32];
	char **pstr = (char **)&datetime;
	spprintf(pstr, 0, "%s @ %s", __DATE__, __TIME__);
	
	php_info_print_table_start();
	php_info_print_table_header(2, "Version", 					"$0.1 $");
	php_info_print_table_header(2, "Compiled",					*pstr);
	php_info_print_table_header(2, "Riak Pbc version", 			"1.1");
	php_info_print_table_header(2, "Default host",				DEFAULT_HOST);
	php_info_print_table_header(2, "Default port",				DEFAULT_PORT);
	php_info_print_table_end();
}

/*
*Local variables:
*tab-width: 4
*c-basic-offset: 4
*End:
*vim600: noet sw=4 ts=4 fdm=marker
*vim<600: noet sw=4 ts=4
*/

