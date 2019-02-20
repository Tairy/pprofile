<?php

function test1() {
    return "ssss\n";
}

function test() {
   echo test1();
}
function eee() {
    pprofile_enable(PPROFILE_FLAGS_MEMORY_ALLOC | PPROFILE_FLAGS_CPU);

    for( $i = 0; $i < 10; ++$i ) {
        test();
    }

    echo json_encode(pprofile_disable());
}

eee();