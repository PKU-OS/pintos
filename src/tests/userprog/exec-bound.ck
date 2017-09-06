# -*- perl -*-
use strict;
use warnings;
use tests::tests;
check_expected ([<<'EOF']);
(exec-bound) begin
(args) begin
(args) argc = 3
(args) argv[0] = 'child-args'
(args) argv[1] = 'arg1'
(args) argv[2] = 'arg2'
(args) argv[3] = null
(args) end
child-args: exit(0)
(exec-bound) end
exec-bound: exit(0)
EOF
pass;
