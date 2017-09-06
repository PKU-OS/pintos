# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(exec-bound-3) begin
exec-bound-3: exit(-1)
EOF
pass;
