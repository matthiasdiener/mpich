% SUMMARY FILE FOR NODE 0 (world node 0) , row 0, col 0

% parameters:
%		datatype	min msg	max msg	mult factor	add step
%		--------	-------	-------	-----------	--------
% value used:	3		1	8192	2.000000	1.000000
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
params = [3 1 8192 2.000000e+00 1.000000e+00 1 0 0 0 0 1 2];

% estimated time for clock = 0.000000e+00
% MPI clock resolution = 8.300000e-05

routine = 'Bcast';
coll = 'MPI';
waycomm = 'MPI';
waygsync = 'GSYNCE';

times = [
% bytes     max          min       avg
  4 4.578000e-03 9.020000e-04 2.740000e-03
  12 8.990000e-04 8.850000e-04 8.920000e-04
  28 9.650000e-04 9.110000e-04 9.380000e-04
  60 9.130000e-04 7.100000e-04 8.115000e-04
  124 1.004000e-03 9.120000e-04 9.580000e-04
  252 9.460000e-04 7.920000e-04 8.690000e-04
  508 1.311000e-03 7.670000e-04 1.039000e-03
  1020 3.840000e-03 1.025000e-03 2.432500e-03
  2044 8.289000e-03 1.251000e-03 4.770000e-03
  4092 1.072100e-02 1.061000e-02 1.066550e-02
  8188 2.530800e-02 2.153000e-02 2.341900e-02
  16380 4.943400e-02 4.332900e-02 4.638150e-02
  32764 9.679600e-02 9.180500e-02 9.430050e-02
];
comm_stats
case=case+1;

% normal termination
