#!/usr/bin/python

from optparse import OptionParser
import sys, os, subprocess
from matplotlib import pyplot, animation
import random
import time

def plot_classification(path, classification):	
	if classification == 2:
		c='r'
	elif classification == 1:
		c='y'
	elif classification == 0:
		c='g'
	else:
		print "ERROR: Unknown classification "+str(classification)
	#plot startlink and endlink
	print "Startlink: "+str(s)+" Endlink: "+str(e)
	for segment in links:
		pyplot.plot([segment[0][0],segment[1][0]], [segment[0][1],segment[1][1]],'xb-')
	pyplot.plot([links[s][0][0],links[s][1][0]],[links[s][0][1],links[s][1][1]], 'o'+c+'-')
	pyplot.plot([links[e][0][0],links[e][1][0]],[links[e][0][1],links[e][1][1]], 'o'+c+'-')
	if (classification == 0):
		pyplot.plot([links[s][0][0],links[e][0][0]],[links[s][0][1],links[e][0][1]], 'o'+c+'-')
	elif (classification == 1):
		#start to int
		pyplot.plot([links[s][0][0],intersections[str(path[0])][0]],[links[s][0][1],intersections[str(path[0])][1]], 'o'+c+'-')
		#end to int
		pyplot.plot([links[e][0][0],intersections[str(path[0])][0]],[links[e][0][1],intersections[str(path[0])][1]], 'o'+c+'-')
	elif (classification == 2):
		#start to first
		pyplot.plot([links[s][0][0],intersections[str(path[0])][0]],[links[s][0][1],intersections[str(path[0])][1]], 'o'+c+'-')
		#first to second
		pyplot.plot([intersections[str(path[0])][0],intersections[str(path[1])][0]],[intersections[str(path[0])][1],intersections[str(path[1])][1]], 'o'+c+'-')
		#second to end
		pyplot.plot([links[e][0][0],intersections[str(path[1])][0]],[links[e][0][1],intersections[str(path[1])][1]], 'o'+c+'-')
	
	if options.axes:
		pyplot.axis(axes)
	pyplot.gca().set_autoscale_on(False)
	pyplot.show()

optParser = OptionParser()
optParser.add_option("-i", "--intersections-file", dest="intfile", help="define a CORNER intersections file")
optParser.add_option("-l", "--links-file", dest="lnkfile", help="define a CORNER links file")
optParser.add_option("-c", "--classification-file", dest="classfile", help="define a CORNER classifications file")
optParser.add_option("-p", "--file-prefix", dest="prefix", help="define a CORNER file prefix. net/lnk/cls files will be read in as prefix.corner.net/int/cls")
optParser.add_option("-a", "--axes", dest="axes", help="define axis range to show in format minX,maxX,minY,maxY")
(options, args) = optParser.parse_args()

if not options.intfile or not options.lnkfile:
	if options.prefix:
		options.intfile = options.prefix+".corner.int"
		options.lnkfile = options.prefix+".corner.lnk"
		if os.path.exists(options.prefix+".corner.cls"):
			options.classfile = options.prefix+".corner.cls"
	else:
		optParser.print_help()
		sys.exit()
plot_class=1
if not options.classfile:
	plot_class=0

axes=[]
if options.axes:
	axes=options.axes.split(',')
	if len(axes) != 4:
		print "Axes must be in format minX,maxX,minY,maxY"
		exit(-1)
	for i in range(0,4):
		axes[i] = float(axes[i])

intfile=open(options.intfile)
counter=0
intersections={}
for line in intfile:
	if counter == 0:
		counter = 1
		continue
	intersections[line.split()[0]]=(line.split()[1],line.split()[2])

intfile.close()
lnkfile=open(options.lnkfile)
counter=0
links=[]
for line in lnkfile:
	if counter == 0:
		counter = 1
		continue
	links.append((intersections[line.split()[1]], intersections[line.split()[2]]))
	
	
lnkfile.close()

for segment in links:
	pyplot.plot([segment[0][0],segment[1][0]], [segment[0][1],segment[1][1]],'xb-')

if not plot_class:
	pyplot.show()
	exit(0)
	
### Read in classifications
classfile=open(options.classfile)
classifications={}
paths={}
counter = 0
for line in classfile:
	if counter == 0:
		counter = 1
		continue
	#read in information
	info = line.split()
	startLink=int(info[0])
	endLink=int(info[1])
	cf=int(info[2])
	
	#remove everything except the node positions
	if cf == 1:
		for i in range(0,6):
			info.remove(info[0])
	elif cf == 2:
		for i in range(0,7):
			info.remove(info[0])
	
	#allocate classification and node positions to respective variables
	if startLink not in classifications.keys():
		classifications[startLink]={}
		paths[startLink]={}
	classifications[startLink][endLink]=cf
	paths[startLink][endLink]=[]
	for node in info:
		paths[startLink][endLink].append(int(node))

		
# Prompt on the display of classifications
print "Enter how you'd like to display the classifications"
print "\t 1) Display each classification in a loop"
print "\t 2) Enter in source and destination links to show the classification between them"
print "\t 3) Quit Program"
print "Enter 1 or 2 below"
while 1:
	a = raw_input()
	if a == '3':
		exit(0)
	elif a == '1':
		for s in classifications:
			for e in classifications[s]:
				classification = classifications[s][e]
				path = paths[s][e]
				plot_classification(path, classification)
	elif a == '2':
		print "Enter a source link and destination link in the format <srcLinkID destLinkID> or type \"quit\" to exit application"
		while 1:
			a = raw_input()
			if a == 'quit':
				exit(0)
			else:
				if len(a.split()) != 2:
					print "Error. Please enter the links in the format <srcLinkID, destLinkID>"
				[s,e] = a.split()
				s = int(s)
				e = int(e)
				classification = classifications[s][e]
				path = paths[s][e]
				plot_classification(path, classification)
				
				
				

