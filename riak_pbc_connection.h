void riak_pbc_dtor(void *object TSRMLS_DC);
zend_object_value riak_pbc_ctor(zend_class_entry *ce TSRMLS_DC);

//void php_riak_pbc_connect(riak_pbc_connection_object *riak_pbc_connection);
//void php_riak_pbc_disconnect(riak_pbc_connection_object *riak_pbc_connection);

PHP_METHOD(riak_pbc_connection_class, __construct);
PHP_METHOD(riak_pbc_connection_class, connect);
PHP_METHOD(riak_pbc_connection_class, pconnect);
PHP_METHOD(riak_pbc_connection_class, get);