% SUMMARY FILE FOR NODE 0 (world node 0) , row 0, col 0

% parameters:
%		datatype	min msg	max msg	mult factor	add step
%		--------	-------	-------	-----------	--------
% value used:	11		1	8192	2.000000	1.000000
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
params = [11 1 8192 2.000000e+00 1.000000e+00 1 0 0 0 0 1 2];

% estimated time for clock = 0.000000e+00
% MPI clock resolution = 8.400000e-05

routine = 'Bcast';
coll = 'MPI';
waycomm = 'MPI';
waygsync = 'GSYNCE';

times = [
% bytes     max          min       avg
  2 4.080000e-03 9.000000e-04 2.490000e-03
  6 8.900000e-04 6.700000e-04 7.800000e-04
  14 1.080000e-03 8.950000e-04 9.875000e-04
  30 1.489000e-03 9.200000e-04 1.204500e-03
  62 1.065000e-03 9.220000e-04 9.935000e-04
  126 9.660000e-04 9.490000e-04 9.575000e-04
  254 1.823000e-03 1.315000e-03 1.569000e-03
  510 1.508000e-03 1.313000e-03 1.410500e-03
  1022 2.782000e-03 1.033000e-03 1.907500e-03
  2046 6.916000e-03 1.232000e-03 4.074000e-03
  4094 1.199200e-02 1.108400e-02 1.153800e-02
  8190 2.914000e-02 2.569100e-02 2.741550e-02
  16382 5.336300e-02 4.912400e-02 5.124350e-02
];
comm_stats
case=case+1;

% normal termination
