<?php

function test1() {
    return "ssss\n";
}

function test() {
   echo test1();
}
function eee() {
    pprofile_start();

    for( $i = 0; $i < 10; ++$i ) {
        test();
    }

    //pprofile_end();
    echo "===";
    echo "\n";
    echo json_encode(pprofile_end());
}

eee();