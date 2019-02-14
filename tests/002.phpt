--TEST--
pprofile_test1() Basic test
--SKIPIF--
<?php
if (!extension_loaded('pprofile')) {
	echo 'skip';
}
?>
--FILE--
<?php
$ret = pprofile_test1();

var_dump($ret);
?>
--EXPECT--
The extension pprofile is loaded and working!
NULL
