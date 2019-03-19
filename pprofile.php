<?php

function test1() {
    return "ssss\n";
}

function test() {
   echo test1();
}
function main() {
    pprofile_start();

    for( $i = 0; $i < 10; ++$i ) {
        test();
    }

    echo "===";
    echo "\n";
    echo json_encode(pprofile_end());
    //echo "\n";
    //echo json_encode(pprofile_get_uuid());
}

main();