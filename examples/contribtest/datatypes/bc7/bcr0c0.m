% SUMMARY FILE FOR NODE 0 (world node 0) , row 0, col 0

% parameters:
%		datatype	min msg	max msg	mult factor	add step
%		--------	-------	-------	-----------	--------
% value used:	7		1	8192	2.000000	1.000000
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
params = [7 1 8192 2.000000e+00 1.000000e+00 1 0 0 0 0 1 2];

% estimated time for clock = 0.000000e+00
% MPI clock resolution = 8.300000e-05

routine = 'Bcast';
coll = 'MPI';
waycomm = 'MPI';
waygsync = 'GSYNCE';

times = [
% bytes     max          min       avg
  2 4.065000e-03 9.210000e-04 2.493000e-03
  6 9.070000e-04 8.670000e-04 8.870000e-04
  14 1.067000e-03 9.190000e-04 9.930000e-04
  30 9.490000e-04 9.210000e-04 9.350000e-04
  62 2.077000e-03 9.350000e-04 1.506000e-03
  126 1.291000e-03 1.036000e-03 1.163500e-03
  254 1.577000e-03 9.560000e-04 1.266500e-03
  510 1.337000e-03 7.970000e-04 1.067000e-03
  1022 3.233000e-03 1.031000e-03 2.132000e-03
  2046 7.464000e-03 1.242000e-03 4.353000e-03
  4094 1.397500e-02 1.217900e-02 1.307700e-02
  8190 2.493000e-02 1.961000e-02 2.227000e-02
  16382 4.692630e-01 2.555760e-01 3.624195e-01
];
comm_stats
case=case+1;

% normal termination
