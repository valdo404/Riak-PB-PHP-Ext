

PHP_ARG_WITH(riak_pbc,  
        [Whether to enable the "riak_pbc" extension],  
        [  --enable-riak-pbc  Enable "riak protocol buffer client" extension support])  
if test $PHP_RIAK_PBC != "no"; then  

		#dnl # --enable-riak-pbc -> we need the protobuf-c libraries
		#	SEARCH_PATH = "/usr/local"
		
		PHP_ADD_INCLUDE(/usr/local/include/google/protobuf)
		PHP_ADD_INCLUDE(/usr/local/include/google/protobuf-c)
		
		# change to a search for this library
		PHP_ADD_LIBRARY(protobuf-c, /usr/local/lib, RIAK_PBC_SHARED_LIBADD)
		PHP_ADD_LIBRARY(protobuf, /usr/local/lib, RIAK_PBC_SHARED_LIBADD)
			
        PHP_SUBST(RIAK_PBC_SHARED_LIBADD)  
		# PHP_ADD_LIBRARY(stdc++, 1, RIAK_PBC_SHARED_LIBADD)
        PHP_NEW_EXTENSION(riak_pbc, riak_pbc.c riak_pbc_connection.c riakclient.pb-c.c, $ext_shared)
fi