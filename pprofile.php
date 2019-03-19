<?php

function test1() {
    return "ssss\n";
}

function test() {
   echo test1();
}
function main() {
    //pprofile_start();

    for( $i = 0; $i < 1000; ++$i ) {
        //test();
        $a = pprofile_get_uuid();
        echo $a["result"] . "\n";
    }

    //pprofile_end();
    //echo "===";
    //echo "\n";
    //echo json_encode(pprofile_end());
    //echo "\n";
    //echo json_encode(pprofile_get_uuid());
}

main();