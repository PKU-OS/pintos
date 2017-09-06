# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(sc-boundary-3) begin
sc-boundary-3: exit(-1)
EOF
pass;
