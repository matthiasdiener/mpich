% SUMMARY FILE FOR NODE 0 (world node 0) , row 0, col 0

% parameters:
%		datatype	min msg	max msg	mult factor	add step
%		--------	-------	-------	-----------	--------
% value used:	1		1	8192	2.000000	1.000000
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
params = [1 1 8192 2.000000e+00 1.000000e+00 1 0 0 0 0 1 2];

% estimated time for clock = 0.000000e+00
% MPI clock resolution = 8.300000e-05

routine = 'Bcast';
coll = 'MPI';
waycomm = 'MPI';
waygsync = 'GSYNCE';

times = [
% bytes     max          min       avg
  1 9.480000e-04 9.220000e-04 9.350000e-04
  3 9.040000e-04 8.810000e-04 8.925000e-04
  7 1.164000e-03 9.060000e-04 1.035000e-03
  15 9.040000e-04 6.600000e-04 7.820000e-04
  31 1.804000e-03 9.300000e-04 1.367000e-03
  63 9.360000e-04 6.890000e-04 8.125000e-04
  127 9.620000e-04 7.420000e-04 8.520000e-04
  255 3.298000e-03 9.540000e-04 2.126000e-03
  511 1.323000e-03 7.550000e-04 1.039000e-03
  1023 1.897000e-03 1.127000e-03 1.512000e-03
  2047 7.897000e-03 1.236000e-03 4.566500e-03
  4095 1.562700e-02 1.414900e-02 1.488800e-02
  8191 3.097400e-02 2.355200e-02 2.726300e-02
];
comm_stats
case=case+1;

% normal termination
