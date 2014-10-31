#!/usr/bin/python -O
import math
from optparse import OptionParser
from sumolib import net
import sys,os
import Corner
import subprocess

def copyFile( src, dest ):
	"""Copy a file from src to dest."""
	inFile = open( src, 'r' )
	outFile = open( dest, 'w' )
	for line in inFile:
		outFile.write( line )
	outFile.close()
	inFile.close()

def StripOSM( filename ):
	"""Strip from the OSM anything that isn't a building or roadway."""
	osmFile = open( filename, 'r' )
	strippedFile = open( "tmpfile.osm", "w" )

	tmpBuf = [];
	bufferData = False
	inWay = False
	keepWay = False

	for line in osmFile:
		curr = line.lstrip()
		if curr[0] != '<':
			continue	# we ignore lines that aren't xml tags.

		loc = curr.find(" ")
		if loc == -1:
			loc = curr.find(">")

		tagName = curr[1:loc]

		if tagName == "way":
			if inWay:
				raise Exception("OSM can't have ways nested in other ways.")
			inWay = True
			bufferData = True
		elif tagName == "/way":
			if not inWay:
				raise Exception("Reached an end of way tag when not in way.")
			inWay = False
		elif tagName == "tag":
			if inWay and not keepWay:
				attribs = curr[loc+1:len(curr)-3].split(" ")
				keyAttrib = attribs[0].split("=")[1].rstrip("\"").lstrip("\"")
				valAttrib = attribs[1].split("=")[1].rstrip("\"").lstrip("\"")
				if keyAttrib == "building" and valAttrib == "yes":
					keepWay = True
				elif keyAttrib == "highway" and ( valAttrib != "footway" and valAttrib != "pedestrian" ):
					keepWay = True

		if bufferData:
			tmpBuf.append(line)
			if not inWay:
				if keepWay:
					for l in tmpBuf:
						strippedFile.write( l )
				tmpBuf = []
				bufferData = False
				keepWay = False
		else:
			strippedFile.write( line )


	osmFile.close()
	strippedFile.close()


def writeIntFile(filename, intersections):
	intfile=open(filename, 'w')
	intfile.write(str(len(intersections))+"\n")
	id=0
	for i in intersections:
		intfile.write(str(id)+" "+str(i.x)+" "+str(i.y)+"\n")
		id+=1
	intfile.close()
	
def writeLinksFile(filename, links):
	lnkfile = open(filename, 'w')
	lnkfile.write(str(len(links))+"\n")
	for l in links:
		lnkfile.write(str(l.uid)+" "+str(l.node1)+" "+str(l.node2)+" "+str(l.lanes)+" 0 0 "+str(l.speed)+"\n")
	lnkfile.close()	

def writeClassifications(filename, classifications):
	classfile = open(filename, 'w')
	classfile.write(str(len(classifications))+"\n")
	for LinkPair in classifications:
		classfile.write(str(LinkPair)+	" "+str(classifications[LinkPair])+"\n")
	classfile.close()

def writeBuildings(filename, buildings):
	bldFile = open(filename,'w')
	bldFile.write(str(len(buildings))+"\n")
	for b in buildings:
		bldFile.write( str(b.Id) + " " + str(b.permittivity.Magnitude()) + " " + str(b.maxHeight) + " " + str(b.heightStd) + " " + str(len(b.nodes)) )
		for n in b.nodes:
			bldFile.write( " " + str(n.position.x) + " " + str(n.position.y) )
		bldFile.write( "\n" )
	bldFile.close()

def writeLinkLookup(filename, linkLookup):
	lnmFile = open(filename,'w')
	lnmFile.write(str(len(linkLookup))+"\n")
	for l in linkLookup:
		lnmFile.write( l[0] + " " + str(l[1]) + "\n" )
	lnmFile.close()

def writeInternalLinkLookup(filename, linkLookup):
	ilnmFile = open(filename,'w')
	ilnmFile.write(str(len(linkLookup))+"\n")
	for l in linkLookup.keys():
		ilnmFile.write( l + " " + str(linkLookup[l]) + "\n" )
	ilnmFile.close()

def parseOptions():
	optParser = OptionParser()
	optParser.add_option("-o", "--osm-file", dest="osmfile", help="define the Open Street Map file")
	optParser.add_option("-s", "--strip-osm-file", dest="stripOSM", action="store_true", default=False, help="strip unneccesary information from the OSM file.")
	optParser.add_option("-n", "--net-file", dest="netfile", help="define the SUMO net output file")
	optParser.add_option("-i", "--output-intersection-file", dest="intfile", help="define the intersection output file")
	optParser.add_option("-l", "--output-links-file", dest="lnkfile", help="define the output links file")
	optParser.add_option("-c", "--output-class-file", dest="classfile", help="define the output classifications file")
	optParser.add_option("-b", "--output-buildings-file", dest="buildfile", help="define the output buildings file")
	optParser.add_option("-m", "--output-link-lookup", dest="lnknamefile", help="define the output Link-Name map file")
	optParser.add_option("-I", "--output-internal-link-lookup", dest="intlnknamefile", help="define the output Internal Link - Node ID map file")
	optParser.add_option("-a", "--los-angle", dest="angle", type="float", default=0.34906585, help="define an angle within which two links are considered to be in LOS")
	optParser.add_option("-t", "--no-turnarounds", dest="turnArounds", action="store_true", default=False, help="enable no turn arounds in netconvert")
	(options, args) = optParser.parse_args()

	file_prefix=None
	if not options.osmfile:
		if not options.netfile:
			optParser.print_help()
			sys.exit()
		file_prefix=options.netfile.split('.net.xml')[0]
	else:
		file_prefix=options.osmfile.split('.osm')[0]

	if not options.netfile:
		options.netfile=file_prefix+".net.xml"
	if not options.intfile:
		options.intfile=file_prefix+".corner.int"
	if not options.lnkfile:
		options.lnkfile=file_prefix+".corner.lnk"
	if not options.classfile:
		options.classfile=file_prefix+".corner.cls"
	if not options.buildfile:
		options.buildfile=file_prefix+".corner.bld"
	if not options.lnknamefile:
		options.lnknamefile=file_prefix+".corner.lnm"
	if not options.intlnknamefile:
		options.intlnknamefile=file_prefix+".corner.ilnm"
	
	return options

############################# Begin Main Function ######################################
if __name__ == "__main__":
	options = parseOptions()

	if options.osmfile:
		if options.stripOSM:
			print "Stripping OSM file..."
			try:
				StripOSM( options.osmfile )
			except Exception as e:
				print e
				sys.exit()
			copyFile( options.osmfile, "osm.tmp" )
			copyFile( "tmpfile.osm", options.osmfile )
			os.remove( "tmpfile.osm" )

		print "Converting OSM to SUMO"
		try:
			args = ["netconvert","--lefthand","--no-internal-links","--remove-edges.isolated","--osm-files",options.osmfile,"--output-file",options.netfile]
			if not options.turnArounds:
				args.append("--no-turnarounds")
			subprocess.check_call( args )
		except OSError as e:
			print "OS Error({0}): {1}".format(e.errno, e.strerror)
		except CalledProcessError as e:
			print "Process Error: " + e.strerror

		print "Extracting buildings from OSM"
		buildings = Corner.BuildingExtractor( options.osmfile.split('.osm')[0] ).buildings

		if options.stripOSM:
			os.remove( options.osmfile )
			copyFile( "osm.tmp", options.osmfile )
			os.remove( "osm.tmp" )
	else:
		buildings = []

	print "Reading SUMO Network"
	#net = net.readNet(options.netfile)
	(intersections,links,linkMap,intLinkMap) = Corner.sumoLoader(options.netfile).getCornerData()

	print "Generating Classifications"
	classifications = Corner.Generator(intersections, links, options.angle).classifications

	print "Writing Output Intersections"
	writeIntFile(options.intfile, intersections)
	print "Writing Output Links"
	writeLinksFile(options.lnkfile, links)
	print "Writing Output Classifications"
	writeClassifications(options.classfile, classifications)
	print "Writing Output Buildings"
	writeBuildings(options.buildfile, buildings)
	print "Writing Output Link-Name lookup"
	writeLinkLookup(options.lnknamefile, linkMap)
	print "Writing Output Internal Link - Node ID lookup"
	writeInternalLinkLookup(options.intlnknamefile, intLinkMap)



