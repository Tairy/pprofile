--TEST--
Check if pprofile is loaded
--SKIPIF--
<?php
if (!extension_loaded('pprofile')) {
	echo 'skip';
}
?>
--FILE--
<?php
echo 'The extension "pprofile" is available';
?>
--EXPECT--
The extension "pprofile" is available
