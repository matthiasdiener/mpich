% SUMMARY FILE FOR NODE 0 (world node 0) , row 0, col 0

% parameters:
%		datatype	min msg	max msg	mult factor	add step
%		--------	-------	-------	-----------	--------
% value used:	5		1	8192	2.000000	1.000000
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
params = [5 1 8192 2.000000e+00 1.000000e+00 1 0 0 0 0 1 2];

% estimated time for clock = 0.000000e+00
% MPI clock resolution = 8.300000e-05

routine = 'Bcast';
coll = 'MPI';
waycomm = 'MPI';
waygsync = 'GSYNCE';

times = [
% bytes     max          min       avg
  4 4.867000e-03 9.040000e-04 2.885500e-03
  12 1.656000e-03 9.140000e-04 1.285000e-03
  28 1.017000e-03 9.270000e-04 9.720000e-04
  60 1.134000e-03 9.390000e-04 1.036500e-03
  124 9.610000e-04 7.180000e-04 8.395000e-04
  252 1.047000e-03 9.590000e-04 1.003000e-03
  508 5.890000e-03 1.523000e-03 3.706500e-03
  1020 2.934000e-03 1.017000e-03 1.975500e-03
  2044 2.548000e-03 1.240000e-03 1.894000e-03
  4092 1.349000e-02 1.313500e-02 1.331250e-02
  8188 2.877000e-02 2.495200e-02 2.686100e-02
  16380 5.831200e-02 4.534900e-02 5.183050e-02
  32764 1.008790e-01 9.343200e-02 9.715550e-02
];
comm_stats
case=case+1;

% normal termination
