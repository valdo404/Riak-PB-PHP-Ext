#!/usr/bin/php

<?php

$rpbc = new RiakPbcConnection("192.168.1.80", 8087);
$rpbc->connect();
$rpbc->get("test", "2");

?>
