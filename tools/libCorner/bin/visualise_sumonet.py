#!/usr/bin/python

from optparse import OptionParser
import sys, os, subprocess

optParser = OptionParser()
optParser.add_option("-n", "--net-file", dest="netfile", help="define a SUMO net file (mandatory)")
optParser.add_option("-m", "--mobility-file", dest="mobfile", help="define a Qualnet mobility file (default is netfile_prefix.qnet.nodes)")
optParser.add_option("-f", "--output-format", dest="out_fmt", help="define whether to output for Octave or Matlab (default: Octave)")
optParser.add_option("-o", "--output-file", dest="outfile", help="define the output file (default: visualise.m)")

(options, args) = optParser.parse_args()

if not options.netfile:
	optParser.print_help()
	sys.exit()

node_movement = 1
if not options.mobfile:
	print "No mobility file specified. Will not be printing node movements"
	node_movement = 0
if not options.out_fmt:
	options.out_fmt="octave"
else:
	if options.out_fmt == "octave" or options.out_fmt == "Octave":
		options.out_fmt="octave"
	elif options.out_fmt == "matlab" or options.out_fmt == "Matlab":
		options.out_fmt="matlab"
	else:
		print "Output format invalid"
		optParser.print_help()
		sys.exit()
		
if not options.outfile:
	options.outfile="visualise.m"

#read in qnet nodes file
if (node_movement):
	qnetfile = open(options.mobfile)
	nodepos={}
	last_timestamp=0
	for line in qnetfile:
		nodeid = line.split()[0]
		timestamp = int(line.split()[1])
		coords = line.split()[2].split('(')[1].split(')')[0].split(',')
		coords = coords[0]+"+"+coords[1]+"i"
		if nodeid not in nodepos:
			nodepos[nodeid] = {}
		nodepos[nodeid][timestamp] = coords
		last_timestamp = timestamp
	qnetfile.close()

#read in all lane description
snetfile = open(options.netfile)
lanes={}
for line in snetfile:
	if "lane id=" in line:
		laneid = line.split("\"")[1]
		shape=[]
		temp = line.split("\"")[-2].split()
		for i in range(0, len(temp)):
			shape.append(temp[i].split(","))
			shape[i][0] = float(shape[i][0])
			shape[i][1] = float(shape[i][1])
		lanes[laneid] = shape

#visualise the map in matlab/octave
mfile = open(options.outfile, 'w')
mfile.write('hold on\n')
for lane in lanes:
	#lets plot the shapes
	mfile.write('plot([')
	for i in range(0, len(lanes[lane])):
		mfile.write(str(lanes[lane][i][0]))
		mfile.write('+')
		mfile.write(str(lanes[lane][i][1]))
		mfile.write('i')
		if i != len(lanes[lane])-1:
			mfile.write(',')
	mfile.write('], \'-\')\n')

if (node_movement):
	#now visualise each vehicle at each timestamp
	for timestamp in range(0, last_timestamp):
		for nodeid in nodepos:
			coords = nodepos[nodeid][timestamp]
			if options.out_fmt == 'octave':
				mfile.write("node"+nodeid+" = plot ("+coords+",'x1')\n")
			else:
				mfile.write("node"+nodeid+" = plot ("+coords+",'x', 'Color', 'red')\n")
		
		if options.out_fmt == 'octave':
			mfile.write("print ('plot"+str(timestamp)+".jpg', '-color')\n")
		else:
			mfile.write("M("+str(timestamp+1)+") = getframe;\n")
		for nodeid in nodepos:
			mfile.write("delete(node"+nodeid+")\n")

if options.out_fmt == 'matlab':		
	mfile.write("movie(M,30)\n")

mfile.close()

#if options.out_fmt == 'octave':
#	subprocess.call(["octave", options.outfile])
#	subprocess.call(["mencoder", "mf://*.jpg", "fps=25:type=jpg", "-ovc", "lavc", "-lavcopts", "vcodec=mpeg4:mbd=2:trell", "-oac", "copy", "-o", "output.avi"])
