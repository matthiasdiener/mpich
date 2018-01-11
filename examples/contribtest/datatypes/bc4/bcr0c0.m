% SUMMARY FILE FOR NODE 0 (world node 0) , row 0, col 0

% parameters:
%		datatype	min msg	max msg	mult factor	add step
%		--------	-------	-------	-----------	--------
% value used:	4		1	8192	2.000000	1.000000
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
params = [4 1 8192 2.000000e+00 1.000000e+00 1 0 0 0 0 1 2];

% estimated time for clock = 0.000000e+00
% MPI clock resolution = 8.200000e-05

routine = 'Bcast';
coll = 'MPI';
waycomm = 'MPI';
waygsync = 'GSYNCE';

times = [
% bytes     max          min       avg
  4 4.341000e-03 9.070000e-04 2.624000e-03
  12 9.020000e-04 6.680000e-04 7.850000e-04
  28 1.010000e-03 9.200000e-04 9.650000e-04
  60 9.220000e-04 6.690000e-04 7.955000e-04
  124 9.820000e-04 9.440000e-04 9.630000e-04
  252 4.198000e-03 1.047000e-03 2.622500e-03
  508 2.054000e-03 1.316000e-03 1.685000e-03
  1020 3.759000e-03 1.017000e-03 2.388000e-03
  2044 6.927000e-03 1.221000e-03 4.074000e-03
  4092 1.612900e-02 1.273500e-02 1.443200e-02
  8188 3.686600e-02 6.830000e-04 1.877450e-02
  16380 6.009100e-02 5.214400e-02 5.611750e-02
  32764 1.038060e-01 1.007720e-01 1.022890e-01
];
comm_stats
case=case+1;

% normal termination
