#!/usr/bin/python

from optparse import OptionParser
import lexmlreader
import sys, os, xml.sax.handler, pprint
import random

optParser = OptionParser()
optParser.add_option("-x", "--xml-file", dest="netfile", help="define a Qualnet urban terrain XML file (mandatory)")
optParser.add_option("-f", "--output-format", dest="out_fmt", help="define whether to output for Octave or Matlab (default: Octave)")
optParser.add_option("-o", "--output-file", dest="outfile", help="define the output file (default: visualise.m)")

(options, args) = optParser.parse_args()

if not options.netfile:
	optParser.print_help()
	sys.exit()

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

terrain = lexmlreader.readQualnetTerrain(options.netfile)

f=open(options.outfile, 'w')
f.write("hold on\n")
colors=["cyan", "green", "yellow"]
for building in terrain.buildings:
	color = random.choice(colors)
	for face in building:
		face_x="["
		face_y="["
		face_z="["
		for i in range(0,len(face)):
			patch=face[i]
			face_x+=str(patch['x'])+','
			face_y+=str(patch['y'])+','
			face_z+=str(patch['z'])+','
		face_x=face_x[:-1]
		face_y=face_y[:-1]
		face_z=face_z[:-1]
		face_x+="]"
		face_y+="]"
		face_z+="]"
		f.write("patch("+face_x+","+face_y+","+face_z+",'"+color+"')\n")

			
		
			
	





