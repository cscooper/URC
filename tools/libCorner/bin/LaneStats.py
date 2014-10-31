import sumolib
import sys

net = sumolib.net.readNet( sys.argv[1] )
edges = net.getEdges()

totalEdges = len(edges)
laneCounts = {}
totalLanes = 0

for edge in edges:
	nLanes = edge.getLaneNumber()
	if nLanes not in laneCounts:
		laneCounts[nLanes] = 0
	laneCounts[nLanes] += 1
	totalLanes += nLanes

print "Map: " + sys.argv[1]
for count in laneCounts:
	print "\t" + str(count) + " Lane Links: ", laneCounts[count], " (", 100 * float(laneCounts[count]) / totalLanes, "%)"

print "\tTotal number of edges: ", totalEdges
print "\tTotal number of lanes: ", totalLanes
print "\tAverage lanes per link: ", float(totalLanes) / totalEdges

