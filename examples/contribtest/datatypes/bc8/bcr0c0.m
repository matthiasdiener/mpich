% SUMMARY FILE FOR NODE 0 (world node 0) , row 0, col 0

% parameters:
%		datatype	min msg	max msg	mult factor	add step
%		--------	-------	-------	-----------	--------
% value used:	8		1	8192	2.000000	1.000000
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
params = [8 1 8192 2.000000e+00 1.000000e+00 1 0 0 0 0 1 2];

% estimated time for clock = 0.000000e+00
% MPI clock resolution = 8.200000e-05

routine = 'Bcast';
coll = 'MPI';
waycomm = 'MPI';
waygsync = 'GSYNCE';

times = [
% bytes     max          min       avg
  4 4.101000e-03 9.210000e-04 2.511000e-03
  12 9.510000e-04 9.120000e-04 9.315000e-04
  28 9.280000e-04 6.560000e-04 7.920000e-04
  60 9.930000e-04 9.530000e-04 9.730000e-04
  124 1.165000e-03 1.073000e-03 1.119000e-03
  252 1.237000e-03 1.063000e-03 1.150000e-03
  508 1.851000e-03 1.334000e-03 1.592500e-03
  1020 2.192000e-03 1.027000e-03 1.609500e-03
  2044 6.241000e-03 1.247000e-03 3.744000e-03
  4092 1.979400e-02 1.232600e-02 1.606000e-02
  8188 2.659300e-02 2.194900e-02 2.427100e-02
  16380 5.253600e-02 4.662200e-02 4.957900e-02
  32764 1.075400e-01 1.004740e-01 1.040070e-01
];
comm_stats
case=case+1;

% normal termination
