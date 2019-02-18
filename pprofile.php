<?php
function main() {
    pprofile_enable();

    for( $i = 0; $i < 10; ++$i ) {
        echo "xx\n";
    }

    echo json_encode(pprofile_disable());
    // pprofile_disable();
}

main();