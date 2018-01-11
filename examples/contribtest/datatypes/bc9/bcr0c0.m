% SUMMARY FILE FOR NODE 0 (world node 0) , row 0, col 0

% parameters:
%		datatype	min msg	max msg	mult factor	add step
%		--------	-------	-------	-----------	--------
% value used:	9		1	8192	2.000000	1.000000
%
%		# trials	# clock trials	direction	offset	vary
%		--------	---------------	--------	------	----
% value used:	1		0		0		0	0
%
%		# rows		# cols
%		------		------
% value used:	1		2
%
mach = 'UNKNOWN';
params = [9 1 8192 2.000000e+00 1.000000e+00 1 0 0 0 0 1 2];

% estimated time for clock = 0.000000e+00
% MPI clock resolution = 8.100000e-05

routine = 'Bcast';
coll = 'MPI';
waycomm = 'MPI';
waygsync = 'GSYNCE';

times = [
% bytes     max          min       avg
  1 5.704000e-03 1.013000e-03 3.358500e-03
  3 1.232000e-03 8.910000e-04 1.061500e-03
  7 8.980000e-04 6.590000e-04 7.785000e-04
  15 9.050000e-04 6.650000e-04 7.850000e-04
  31 1.128000e-03 9.160000e-04 1.022000e-03
  63 1.133000e-03 9.200000e-04 1.026500e-03
  127 1.415000e-03 9.500000e-04 1.182500e-03
  255 1.086000e-03 9.530000e-04 1.019500e-03
  511 1.801000e-03 1.325000e-03 1.563000e-03
  1023 2.962000e-03 1.011000e-03 1.986500e-03
  2047 5.900000e-03 1.339000e-03 3.619500e-03
  4095 1.231900e-02 1.227100e-02 1.229500e-02
  8191 2.676000e-02 2.321500e-02 2.498750e-02
];
comm_stats
case=case+1;

% normal termination
