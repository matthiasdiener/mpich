% SUMMARY FILE FOR NODE 0 (world node 0) , row 0, col 0

% parameters:
%		datatype	min msg	max msg	mult factor	add step
%		--------	-------	-------	-----------	--------
% value used:	0		1	8192	2.000000	1.000000
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
params = [0 1 8192 2.000000e+00 1.000000e+00 1 0 0 0 0 1 2];

% estimated time for clock = 0.000000e+00
% MPI clock resolution = 8.100000e-05

routine = 'Bcast';
coll = 'MPI';
waycomm = 'MPI';
waygsync = 'GSYNCE';

times = [
% bytes     max          min       avg
  1 1.090000e-03 9.040000e-04 9.970000e-04
  3 1.010000e-03 6.720000e-04 8.410000e-04
  7 9.230000e-04 9.150000e-04 9.190000e-04
  15 1.119000e-03 9.230000e-04 1.021000e-03
  31 1.251000e-03 9.310000e-04 1.091000e-03
  63 4.766000e-03 9.400000e-04 2.853000e-03
  127 1.042000e-03 9.620000e-04 1.002000e-03
  255 1.043000e-03 9.650000e-04 1.004000e-03
  511 1.743000e-03 1.331000e-03 1.537000e-03
  1023 2.973000e-03 1.132000e-03 2.052500e-03
  2047 6.164000e-03 1.242000e-03 3.703000e-03
  4095 1.064100e-02 1.054000e-02 1.059050e-02
  8191 2.653600e-02 2.243900e-02 2.448750e-02
];
comm_stats
case=case+1;

% normal termination
