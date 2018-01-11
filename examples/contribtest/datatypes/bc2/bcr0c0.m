% SUMMARY FILE FOR NODE 0 (world node 0) , row 0, col 0

% parameters:
%		datatype	min msg	max msg	mult factor	add step
%		--------	-------	-------	-----------	--------
% value used:	2		1	8192	2.000000	1.000000
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
params = [2 1 8192 2.000000e+00 1.000000e+00 1 0 0 0 0 1 2];

% estimated time for clock = 0.000000e+00
% MPI clock resolution = 8.200000e-05

routine = 'Bcast';
coll = 'MPI';
waycomm = 'MPI';
waygsync = 'GSYNCE';

times = [
% bytes     max          min       avg
  8 4.317000e-03 9.260000e-04 2.621500e-03
  24 1.132000e-03 9.210000e-04 1.026500e-03
  56 1.141000e-03 9.350000e-04 1.038000e-03
  120 1.400000e-03 9.590000e-04 1.179500e-03
  248 1.116000e-03 9.710000e-04 1.043500e-03
  504 2.172000e-03 1.331000e-03 1.751500e-03
  1016 3.926000e-03 1.029000e-03 2.477500e-03
  2040 6.659000e-03 1.252000e-03 3.955500e-03
  4088 1.128800e-02 1.005700e-02 1.067250e-02
  8184 3.479100e-02 2.825900e-02 3.152500e-02
  16376 5.347100e-02 5.058500e-02 5.202800e-02
  32760 1.416590e-01 1.310730e-01 1.363660e-01
  65528 1.818630e-01 1.740070e-01 1.779350e-01
];
comm_stats
case=case+1;

% normal termination
