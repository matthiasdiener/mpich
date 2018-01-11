% SUMMARY FILE FOR NODE 0 (world node 0) , row 0, col 0

% parameters:
%		datatype	min msg	max msg	mult factor	add step
%		--------	-------	-------	-----------	--------
% value used:	10		1	8192	2.000000	1.000000
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
params = [10 1 8192 2.000000e+00 1.000000e+00 1 0 0 0 0 1 2];

% estimated time for clock = 0.000000e+00
% MPI clock resolution = 8.300000e-05

routine = 'Bcast';
coll = 'MPI';
waycomm = 'MPI';
waygsync = 'GSYNCE';

times = [
% bytes     max          min       avg
  4 3.706000e-03 9.200000e-04 2.313000e-03
  12 9.700000e-04 9.190000e-04 9.445000e-04
  28 1.116000e-03 9.240000e-04 1.020000e-03
  60 1.202000e-03 9.410000e-04 1.071500e-03
  124 1.263000e-03 9.720000e-04 1.117500e-03
  252 1.337000e-03 9.670000e-04 1.152000e-03
  508 2.193000e-03 1.330000e-03 1.761500e-03
  1020 3.870000e-03 1.030000e-03 2.450000e-03
  2044 8.036000e-03 1.253000e-03 4.644500e-03
  4092 1.190400e-02 1.164900e-02 1.177650e-02
  8188 2.496100e-02 2.110900e-02 2.303500e-02
  16380 4.766200e-02 4.273900e-02 4.520050e-02
  32764 9.410200e-02 8.372800e-02 8.891500e-02
];
comm_stats
case=case+1;

% normal termination
