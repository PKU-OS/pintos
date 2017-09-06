# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(exec-bound-2) begin
exec-bound-2: exit(-1)
EOF
pass;
