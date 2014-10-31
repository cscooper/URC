import math, sys, random
from sumolib import net
from VectorMath import *
import xml.sax
import pyproj

#Simple function to generate an ordered pair. Used to ensure classifications are only stored in the forward direction
def orderPair(one, two):
	if one < two:
		return str(one)+" "+str(two)
	else:
		return str(two)+" "+str(one)

#Class Intersection: Represents a node in CORNER. Contains the coordinates of the node as well as all the links connected to this node
class Intersection:
	def __init__(self, x, y):
		self.x = x
		self.y = y
		self.links=[]
	def getVector(self):
		return Vector2D(self.x, self.y)
	def __str__(self):
		return("("+str(self.x)+","+str(self.y)+")")

#Class Link: Represents a link in CORNER. Contains the nodes which form this link as well as the number of lanes and speed. Also holds a copy of all the nodes for the getNode function
class Link:
	def __init__(self, node1, node2, lanes, speed, nodeList, name):
		self.uid = -1 #uninitialised
		self.node1 = node1
		self.node2 = node2
		self.lanes = lanes
		self.speed = speed
		self.nodeList = nodeList
		self.name = name
		self.sublinkNames = []
	
	#Function setUID(uid): sets the uid of this link
	def setUid(self, uid):
		self.uid = uid
	
	#Function isReverse(link): returns True if the link is the reverse of another link
	def isReverse(self, link):
		if (link.node1 == self.node2 and link.node2 == self.node1):
			return True
		else:
			return False
	
	#Function merge(link): merge two links together. This is used to merge an edge with a reverse edge
	def merge(self, link):
		self.speed = (self.speed+link.speed)/2
		self.lanes += link.lanes
		self.sublinkNames.append(link.name)
	
	#Function getNodeID(node): returns the ID of node2 if node = True and node1 if node = False
	def getNodeID(self, node):
		if node:
			return self.node2
		else:
			return self.node1
	
	#Function getNode(node): returns node2 if node = True and node1 if node = False
	def getNode(self, node):
		if node:
			return self.nodeList[self.node2]
		else:
			return self.nodeList[self.node1]
	
	#Function getOtherNode(node): returns the other node in the link compared to the input node
	def getOtherNode(self, node):
		if self.node1 == node:
			return self.node2
		elif self.node2 == node:
			return self.node1
		else:
			print "Error. Node doesn't exist in this link"
			exit(-1)
	
	#Function getCommonNode(link): returns the node that two links share in common. If none, returns -1
	def getCommonNode(self, link):
		if self.node1 == link.node1 or self.node1 == link.node2:
			return self.node1
		elif self.node2 == link.node2 or self.node2 == link.node1:
			return self.node2
		else:
			return -1
	#Function angleBetween(link): returns the angle between self and another link in the range [0, pi]
	def angleBetween(self, link): ###verify this with craig later
		vS = ((self.getNode(True).x-self.getNode(False).x),(self.getNode(True).y-self.getNode(False).y))
		vL = ((link.getNode(True).x-link.getNode(False).x),(link.getNode(True).y-link.getNode(False).y))
		value = (vS[0]*vL[0] + vS[1]*vL[1]) / (math.sqrt(vS[0]*vS[0]+vS[1]*vS[1])*math.sqrt(vL[0]*vL[0]+vL[1]*vL[1]))
		#this is to handle compiler rounding errors
		if value > 1:
			value = 1.0
		elif value < -1:
			value = -1.0
		return math.acos(value)
		#angle1 = math.atan2(self.getNode(True).y - self.getNode(False).y, self.getNode(True).x - self.getNode(False).x)
		#angle2 = math.atan2(link.getNode(True).y - link.getNode(False).y, link.getNode(True).x - link.getNode(False).x)
		#return angle1 - angle2

	#Function returnSegment(): returns the link as a 2D line-segment class
	def returnSegment(self):
		return LineSegment2D(self.nodeList[self.node1].x, self.nodeList[self.node1].y, self.nodeList[self.node2].x, self.nodeList[self.node2].y)
	
	def distanceFromNode(self, node, point):
		if node == self.node1:
			return (self.nodeList[self.node1].getVector()-point).Magnitude()
		else:
			return (self.nodeList[self.node2].getVector()-point).Magnitude()
	
	def linkLength(self):
		return (self.nodeList[self.node1].getVector()-self.nodeList[self.node2].getVector()).Magnitude()
	
	### Overloaded str function returns the link's UID
	def __str__(self):
		return (str(self.uid))

# class Classification: Represents the classification between two CORNER links. Consists of:
	# The path between the two links
	# The reduced path between two links (turning links)
	# The node count between two links
	# Bool shortest, representing if this is the shortest possible classification for this link-pair
	# The classification itself and the number of lanes present in the mainLanes, sideLanes, parallel lanes etc for NLOS1/2 classifications
class Classification:
	def __init__(self, *args):
		#Classifications being written out to file
		if len(args) == 3:
			self.path = args[0]						# Total path
			self.nodeCount = len(args[0])+1					# Total number of nodes
			
			if not args[1]:
				self.classification = 0					
			else:
				self.classification = len(args[1])-1
				self.reducedPath = args[1][:]
			
			self.shortest = args[2]
			
			if self.classification == 1:
				turningPtIdx = args[0].index(args[1][1])
				self.turningNode = args[0][turningPtIdx-1].getCommonNode(args[1][1])
				self.mainLanes = self._getAvgLanes(args[0][:turningPtIdx])
				self.sideLanes = self._getAvgLanes(args[0][turningPtIdx:])
			elif self.classification == 2:
				turningPtIdx1 = args[0].index(args[1][1])
				turningPtIdx2 = args[0].index(args[1][2])
				self.turningNode1 = args[0][turningPtIdx1-1].getCommonNode(args[1][1])
				self.turningNode2 = args[0][turningPtIdx2-1].getCommonNode(args[1][2])
				self.mainLanes = self._getAvgLanes(args[0][:turningPtIdx1])
				self.sideLanes = self._getAvgLanes(args[0][turningPtIdx1:turningPtIdx2])
				self.paraLanes = self._getAvgLanes(args[0][turningPtIdx2:])
		#Classifications being read in from file
		elif len(args) == 1:
			if len(args[0]) == 1:
				self.classification = 0
				self.nodeCount = int(args[0][0])
			elif len(args[0]) == 4:
				self.classification = 1
				self.nodeCount = int(args[0][0])
				self.mainLanes = int(args[0][1])
				self.sideLanes = int(args[0][2])
				self.turningNode = args[0][3]
			elif len(args[0]) == 6:
				self.classification = 2
				self.nodeCount = int(args[0][0])
				self.mainLanes = int(args[0][1])
				self.sideLanes = int(args[0][2])
				self.paraLanes = int(args[0][3])
				self.turningNode1 = args[0][4]
				self.turningNode2 = args[0][5]
		else:
			raise TypeError('Error with number of input arguments to Classification class')
			
			
	def setLOS(self, nodeCount):
		self.classification=0
		self.nodeCount=nodeCount
		
	def setNLOS1(self, nodeCount, mainLanes, sideLanes, turningNode):
		self.classification=1
		self.nodeCount=nodeCount
		self.mainLanes=mainLanes
		self.sideLanes=sideLanes
		self.turningNode=turningNode
		
	def setNLOS2(self, nodeCount, mainLanes, sideLanes, paraLanes, turningNode1, turningNode2):
		self.classification=2
		self.nodeCount=nodeCount
		self.mainLanes=mainLanes
		self.sideLanes=sideLanes
		self.paraLanes=paraLanes
		self.turningNode1=turningNode1
		self.turningNode2=turningNode2
	
	def _getAvgLanes(self, path):
		lanes=0
		for link in path:
			lanes += link.lanes
		return lanes/len(path)
		
	def __str__(self):
		if self.classification == 0:
			return str(self.classification)+" "+str(self.nodeCount)
		elif self.classification == 1:
			return str(self.classification)+" "+str(self.nodeCount)+" "+str(self.mainLanes)+" "+str(self.sideLanes)+" "+str(self.turningNode)
		elif self.classification == 2:
			return str(self.classification)+" "+str(self.nodeCount)+" "+str(self.mainLanes)+" "+str(self.sideLanes)+" "+str(self.paraLanes)+" "+str(self.turningNode1)+" "+str(self.turningNode2)
	

# class Generator:
	# This class generates classifications on a graph in preparation for CORNER simulations
	# Each link-pair is classified as either LOS, NLOS1 or NLOS2 
	# If link-pairs are out of range, they are not added to the classification
class Generator:
	def __init__(self, intersections, links, los_angle):
		#dictionary of classifications represented as classifications[sLink][eLink]. Initialised as an empty double-dict
		self.classifications = {}
		
		#private variables. Intersections and Links list as well as the maximum angle at which to assume LOS
		self._intersections = intersections
		self._links = links
		self._losAngle = los_angle
		
		#temporary variable cP. Only used to pass an empty array into the tracer
		cP = []
			
		#solve all possible classifications for each link
		for sLink in links:
			#set the startLink for later comparison within the tracer
			self.startLink = sLink
			
			#create a single entry in the reduced path, since the startLink is always on it
			cRP = []
			cRP.append(sLink)
			
			#start tracer on both nodes of the startLink
			self._Tracer(sLink, cP, cRP, sLink.getNodeID(False))
			self._Tracer(sLink, cP, cRP, sLink.getNodeID(True))

	#add a classification after checking if it or its reverse exist. If one of them exist, then add this if that one is shorter
	def _addClassification(self, sLink, eLink, path, reducedPath, shortest):		
		linkPair = orderPair(sLink.uid, eLink.uid)
		
		#DEBUG: Remove
		#if (sLink.uid == 399 or sLink.uid == 516) and (eLink.uid == 399 or eLink.uid == 516):
			#for i in path:
				#sys.stdout.write(str(i.uid)+' ')
			#print ' '
			#for i in reducedPath:
				#sys.stdout.write(str(i.uid)+' ')
			#print '' 
			#print ''
		
		#Check if the link pair already exists. If it does, check if we are shorter and replace the classification.
		try:
			if self.classifications[linkPair].shortest:
				return
			else:
				if shortest or len(path)+1 < self.classifications[linkPair].nodeCount:
					self.classifications[linkPair] = Classification(path, reducedPath, shortest)
		#Classification doesn't exist, add a new one
		except KeyError:
			self.classifications[linkPair] = Classification(path, reducedPath, shortest)
		return
				
	def _isNLOS(self, angle):
		return abs(angle) > self._losAngle and abs(math.pi-angle) > self._losAngle
	
	def _Tracer(self, cLink, cP, cRP, entryNode):
		#duplicate these arrays for this trace
		cPath = cP[:]
		cReducedPath = cRP[:]
			
		#add this link to the current path
		cPath.append(cLink)
		
		##DEBUG: Remove
		#if cLink.uid == 394:
			#if self.startLink.uid == 516:
				#print "Angle between "+str(cPath[-2].uid)+" and "+str(cLink.uid)+": "+str(cPath[-2].angleBetween(cLink))
				#print "Angle between "+str(cReducedPath[-1].uid)+" and "+str(cLink.uid)+": "+str(cReducedPath[-1].angleBetween(cLink))
		
		#check and add link to current reduced path if new path is in LOS to previous path and previous reduced path
		if len(cPath) > 1 and (self._isNLOS(cPath[-2].angleBetween(cLink)) or self._isNLOS(cReducedPath[-1].angleBetween(cLink))):
			cReducedPath.append(cLink)
		
		#end condition check if we've exceeded NLOS2 on this trace
		if len(cReducedPath) > 3:
			return
		
		#set the current classification. if it is LOS or NLOS1, len(cReducedPath) is true, since it is the shortest. NLOS2, we can't be sure
		self._addClassification(self.startLink, cLink, cPath, cReducedPath, len(cReducedPath) < 3)
				
		#find list of links connected to the exit node
		exitNode = cLink.getOtherNode(entryNode)
		for link in self._intersections[exitNode].links:
			if self._links[link] in cPath:
				continue
			self._Tracer(self._links[link], cPath, cReducedPath, exitNode)

		return
# Class to load a SUMO network and process it into a set of one-directional edges		
class sumoLoader:
	
	#Class InternalEdgeExtractor: Prescans the sumo.net file and obtains a set of internal edges and their associated nodes.
	#Arguments: fileName: the name of the file to analyse.
	class InternalEdgeExtractor:

		class InternalEdgeHandler( xml.sax.ContentHandler ):

			def __init__(self,parent):
				xml.sax.ContentHandler.__init__(self)
				self.parent = parent;
				self._currentEdge = None
				self.parent.nodeSet = {}


			def startElement( self, name, attrs ):
				if name == 'junction':
					if attrs['id'][0]!=':':
						intLanes = attrs['intLanes'].split(" ")
						if intLanes[0] != "":
							if attrs['id'] not in self.parent.nodeSet:
								self.parent.nodeSet[attrs['id']] = []
							self.parent.nodeSet[attrs['id']] += intLanes

			def endElement( self, name ):
				pass

		def __init__(self,fileName):
			self.nodeSet = {}

			internalEdgeParser = xml.sax.make_parser()
			internalEdgeParser.setContentHandler( sumoLoader.InternalEdgeExtractor.InternalEdgeHandler(self) )
			internalEdgeParser.parse( open(fileName, 'r') )
	
	def __init__(self, netfile):
		self._sumonet = net.readNet(netfile)
		self._intersections=[]
		self._links=[]
		self._linkMap=[]
		
		self._preprocessNet( netfile )
		self._processNet()
		self._postprocessNet()

	# prescans the sumo net file and creates a dict pairing internal edges with their parent nodes.
	def _preprocessNet( self, netfile ):
		self._internalLinkMap = sumoLoader.InternalEdgeExtractor( netfile ).nodeSet

	# Scans through the internal link map and replace node name strings with node indices.
	def _postprocessNet( self ):
		tmp = {}
		for node in self._internalLinkMap.iterkeys():
			for intLink in self._internalLinkMap[node]:
				if intLink in tmp.keys():
					print "WARNING: Internal link '" + intLink + "' is associated with multiple nodes."
					continue
				tmp[intLink] = node
		self._internalLinkMap = tmp

	def getCornerData(self):
		return (self._intersections, self._links, self._linkMap, self._internalLinkMap)
	
	def _getSubShapeName(self, edge, node1, node2, shape):
		names=[]
		names.append(node1)
		names.append(node2)
		names.sort()
		return str(edge)+"_"+str(names[0])+"_"+str(names[1])+"_"+str(shape)
	
	def _processNet(self):
		nodeMap={}
		allLinks=[]
		#read all the nodes from the file
		nodeid=0
		for node in self._sumonet._nodes:
			self._intersections.append(Intersection(node._coord[0], node._coord[1]))
			nodeMap[node._id] = nodeid
			# also update the internal link map, replacing the node name string with its ID
			if node._id in self._internalLinkMap.keys():
				self._internalLinkMap[nodeid] = self._internalLinkMap[str(node._id)]
				del self._internalLinkMap[node._id]
			nodeid += 1
		
		edgeList={}
		#collate the edges into a list of ids
		for edge in self._sumonet._edges:
			edgeList[edge._id] = edge
		
		#read all the edges from the file
		for e in edgeList:
			edge = edgeList[e]
			shape=edge.getShape()
			if len(shape) == 2: #we have an edge connecting two intersections, nice and straight-forward
				allLinks.append(Link(nodeMap[edge._from._id],nodeMap[edge._to._id],edge.getLaneNumber(),edge.getSpeed(),self._intersections,edge.getID()))
			else:
				edge_prefix=''
				edge_type=''
				#find out if the edge has a negative equivalent or *is* a negative edge
				if str('-'+e) in edgeList:
					#I am the positive edge
					edge_prefix = e
					edge_type='f'
				elif str(e[1:]) in edgeList and str(e[:1]) == '-':
					#I am the negative edge
					edge_prefix = e[1:]
					edge_type='r'
				else:
					#This is a unique edge
					edge_prefix = e
					edge_type='f'
				
				subshape=0
				iteration=0
				if edge_type == 'r':
					subshape=len(shape)-3
				#iterate through the shape eg... [from, 0, 1, 2, 3, to]
				for coord in shape:
					#if coord is the first point of the shape, then link is [from, 0]
					if coord == shape[0]:
						int_b=self._getSubShapeName(edge_prefix, edge._from._id, edge._to._id, subshape)
						if int_b not in nodeMap:
							self._intersections.append(Intersection(shape[1][0],shape[1][1]))
							nodeMap[int_b]=nodeid
							nodeid += 1
						allLinks.append(Link(nodeMap[edge._from._id],nodeMap[int_b],edge.getLaneNumber(),edge.getSpeed(),self._intersections,edge.getID()))
						
					#if coord is the last point of the shape, then link is [3, to]. Also, break...
					elif coord == shape[-2]:
						if edge_type=='f':
							shp_a = subshape-1
						else:
							shp_a = subshape+1
						int_a=self._getSubShapeName(edge_prefix, edge._from._id, edge._to._id, shp_a)
						if int_a not in nodeMap:
							self._intersections.append(Intersection(shape[-2][0],shape[-2][1]))
							nodeMap[int_a] = nodeid
							nodeid += 1
						allLinks.append(Link(nodeMap[int_a],nodeMap[edge._to._id],edge.getLaneNumber(),edge.getSpeed(),self._intersections,edge.getID()))
						break
					#intermediate point. Calculate both intersection a and b, for first iter, it is [0, 1]
					#since int_a is from the previously calculated value, it is unnecessary to calculate it again
					else:
						if edge_type=='f':
							shp_a = subshape-1
						else:
							shp_a = subshape+1
						int_a=self._getSubShapeName(edge_prefix, edge._from._id, edge._to._id, shp_a)
						int_b=self._getSubShapeName(edge_prefix, edge._from._id, edge._to._id, subshape)
						if int_b not in nodeMap:
							self._intersections.append(Intersection(shape[iteration+1][0],shape[iteration+1][1]))
							nodeMap[int_b] = nodeid
							nodeid += 1
						allLinks.append(Link(nodeMap[int_a],nodeMap[int_b],edge.getLaneNumber(),edge.getSpeed(),self._intersections,edge.getID()))
					if edge_type == 'f':
						subshape += 1
					else:
						subshape -= 1
					iteration += 1
		
		#reduce the links to one link between nodes
		for link in allLinks:
			match=0
			for rLink in allLinks:
				#if link is the reverse of rLink, then add the pair to reducedLinks as one link
				if (link.isReverse(rLink)):
					self._links.append(link)
					self._links[-1].merge(rLink)
					allLinks.remove(rLink)
					match=1
			if not match:
				#no reverse links were found. Just add this link directly to the reducedLinks set
				self._links.append(link)

		#now connect the nodes to each other using the reduced links set. Also allocate each of these links a UID
		linkId=0
		for link in self._links:
			self._intersections[link.node1].links.append(linkId)
			self._intersections[link.node2].links.append(linkId)
			link.setUid(linkId)
			self._linkMap.append( [ link.name, linkId ] )
			for name in link.sublinkNames:
				self._linkMap.append( [ name, linkId ] )
			linkId += 1
		return

#Class Classifier: Classifies the CORNER map and returns the pathloss between two points if requested
#fadingArgs: Fading?(T/F), Samples, Seed, kFactor
class Classifier:
	def __init__(self, tau, laneWidth, lam, lpr, rxSensitivity, clsfile, lnkfile, intfile, *fadingArgs):
		'''states:	0: LOS
				1: NLOS1
				2: NLOS2
				3: Out of Range
		'''
		self.state=3
		self.nodeset=[]
		self.lam = lam
		self.l42 = (self.lam/4*math.pi)**2
		self.freespaceRange = math.sqrt(tau/10**(rxSensitivity/10)) * (self.lam/4*math.pi)
		self.tau = tau
		self.laneWidth=laneWidth
		self.lpr = lpr
		self.nodeList={}
		self.linkList={}
		self.clsList={}
		if fadingArgs[0] == 1:
			self.fading = True
			self.fadingSamples = fadingArgs[1]
			self.seed = fadingArgs[2]
			self.kFactor = fadingArgs[3]
			#initialise random distribution
			random.seed(self.seed)
		else:
			self.fading = False
		self.netBounds = [Vector2D(1000000,100000),Vector2D(0,0)]
		self.__readFiles(lnkfile, intfile, clsfile)
		
	def __readFiles(self, lnkfile, intfile, clsfile):
		self.netbounds1 = Vector2D(float('inf'),float('inf'))
		self.netbounds2 = Vector2D(0,0)
		f=open(intfile)
		numInt = int(f.readline())
		for line in f:
			tmp = line.split()
			self.nodeList[tmp[0]] = Intersection(float(tmp[1]), float(tmp[2]))
			self.netBounds[0].x = min(self.netBounds[0].x,float(tmp[1]))
			self.netBounds[0].y = min(self.netBounds[0].y,float(tmp[2]))
			self.netBounds[1].x = max(self.netBounds[1].x,float(tmp[1]))
			self.netBounds[1].y = max(self.netBounds[1].y,float(tmp[2]))

			if self.nodeList[tmp[0]].x < self.netbounds1.x:
				self.netbounds1.x = self.nodeList[tmp[0]].x
			if self.nodeList[tmp[0]].y < self.netbounds1.y:
				self.netbounds1.y = self.nodeList[tmp[0]].y
			if self.nodeList[tmp[0]].x > self.netbounds2.x:
				self.netbounds2.x = self.nodeList[tmp[0]].x
			if self.nodeList[tmp[0]].y > self.netbounds2.y:
				self.netbounds2.y = self.nodeList[tmp[0]].y

		self.netbounds2 = self.netbounds2 - self.netbounds1
		self.netbounds1 = Vector2D(0,0)

		f.close()
		
		f=open(lnkfile)
		numLnk = int(f.readline())
		for line in f:
			tmp = line.split()
			self.linkList[tmp[0]] = Link(tmp[1], tmp[2], tmp[3], tmp[6], self.nodeList,"")
			#add link to the nodes list
			self.nodeList[tmp[1]].links.append(tmp[0])
			self.nodeList[tmp[2]].links.append(tmp[0])
			#set the link ID
			self.linkList[tmp[0]].setUid(tmp[0])
		f.close()
		
		f=open(clsfile)
		numCls = int(f.readline())
		for line in f:
			tmp = line.split()
			clsId = orderPair(tmp[0], tmp[1])
			self.clsList[clsId] = Classification(tmp[3:])
		f.close()
	
	def CalculatePathloss(self, source, destination):
		signal = self.__ComputeSignal(source, destination)
		if self.fading:
			fading = self.__ComputeFading()
			#print "Signal: "+str(signal)+" * Fading "+str(fading)
			return signal * fading * self.tau
		else:
			return signal * self.tau
	
	def __ComputeSignal(self, source, destination):
		self.ComputeState(source, destination)
		if self.state == 0: #LOS
			return (self.l42/(source-destination).MagnitudeSq())
		elif self.state == 1: #NLOS1
			rm2 = (source - self.nodeList[str(self.turningNode)].getVector()).MagnitudeSq()
			rm = math.sqrt(rm2)
			rs2 = (self.nodeList[str(self.turningNode)].getVector() - destination).MagnitudeSq()
			rs = math.sqrt(rs2)
			Wm = float(self.mainStreetLaneCount) * self.laneWidth
			Ws = float(self.sideStreetLaneCount) * self.laneWidth
			
			Nmin = int(2*math.sqrt((rm*rs)/(Ws*Wm)))
			#signal after reflection
			PL = (self.l42 * self.lpr**(2*Nmin))/((rm+rs)**2)
			
			#multiply in signal after diffraction
			if (rm < rs):
				return (PL + ((self.l42*self.lam)/(4*rm*rs2)))
			else:
				return (PL + ((self.l42*self.lam)/(4*rs*rm2)))
			
		elif self.state == 2: #NLOS2
			rm2 = (source - self.nodeList[self.turningNode1].getVector()).MagnitudeSq()
			rm = math.sqrt(rm2)
			rs = (self.nodeList[str(self.turningNode1)].getVector() - self.nodeList[self.turningNode2].getVector()).Magnitude()
			rp2 = (self.nodeList[str(self.turningNode1)].getVector() - destination).MagnitudeSq()
			rp = math.sqrt(rp2)
			rsp = rs+rp
			
			Wm = float(self.mainStreetLaneCount) * self.laneWidth
			Ws = float(self.sideStreetLaneCount) * self.laneWidth
			Wp = float(self.paraStreetLaneCount) * self.laneWidth
			
			temp = math.sqrt((rs*Wm*Wp)/(Ws*(rm*Wp+rp*Wm)))
			Nmin = int(((rm*temp) / Wm) + (rs / (Ws*temp)) + ((rp*temp) / Wp))
			rPow2Nmin = self.lpr**(2*Nmin)
			N = int(rp*rs/(Wp*Ws))
			
			#Signal after reflection
			PL = (self.l42*rPow2Nmin)/(rsp+rm)**2
			
			#Signal after double diffraction
			if (rm < rs):
				PL += (self.l42*self.lam**2) / (16*rm*rs*rp2)
			else:
				PL += (self.l42*self.lam**2) / (16*rm2*rp*rs)
			
			#Signal after reflection and diffraction
			if (rs < rp):
				PL += (self.l42*rPow2Nmin*self.lam*rs) / (4 * ((rs+rm)**2) * rp2)
			else:
				PL += (self.l42*rPow2Nmin*self.lam) / (4 * ((rs+rm)**2) * rp)
				
			#Signal after diffraction and reflection
			if (rm < rsp):
				return ((PL + (self.lpr**(2*N)) * self.l42 * self.lam)/(4*rm*rsp**2))
			else:
				return ((PL + (self.lpr**(2*N)) * self.l42 * self.lam)/(4*rsp*rm2))
			
		else:
			return 0
	
	'''__ComputeFading(self, samples, *args)
		Can be called as CalculateFading(20, 'rayleigh') --> 20 runs, rayleigh
		or CalculateFading(50, 'rician', 6) --> 50 runs, rician, K-factor of 6
	'''
	def __ComputeFading(self):
		fading=0
		for i in range(0, self.fadingSamples+1):
			x = random.gauss(0,1)
			y = random.gauss(0,1)
			if self.state == 1:
				#rician
				fading += (((x + math.sqrt(2*self.kFactor))**2) + (y**2)) / (2*(self.kFactor)+1)
			else:
				#rayleigh
				fading += 0.5*((x**2) +(y**2))
		fading = fading/self.fadingSamples
		if fading > 1:
			return 1
		return fading
			
	
	def ComputeState(self, source, destination):
		self.state = 3
		self.FullNodeCount = sys.maxint
		if (destination-source).Magnitude() > self.freespaceRange:
			return
		#find the source link(s)
		possibleSLinks = self.__FindLinksNearPoint(source)
		possibleDLinks = self.__FindLinksNearPoint(destination)
		
		best_class=4
		best_linkset=''
		for s in possibleSLinks:
			for d in possibleDLinks:
				try:
					classification = self.clsList[orderPair(s, d)].classification
					if classification < best_class:
						self.sourceLink = s
						self.destLink = d
						best_class = classification
						best_linkset = orderPair(s,d)
						self.state = best_class
						if self.state == 1:
							self.mainStreetLaneCount = self.linkList[s].lanes
							self.sideStreetLaneCount = self.linkList[d].lanes
							self.turningNode = self.clsList[orderPair(s,d)].turningNode
						if self.state == 2:
							self.mainStreetLaneCount = self.linkList[s].lanes
							self.paraStreetLaneCount = self.linkList[d].lanes
							self.sideStreetLaneCount = self.clsList[orderPair(s,d)].sideLanes
							self.turningNode1 = self.clsList[orderPair(s,d)].turningNode1
							self.turningNode2 = self.clsList[orderPair(s,d)].turningNode2
							#print "Classification "+str(classification)+" from link "+str(s)+" to "+str(d)+" via "+str(self.turningNode1)+" and "+str(self.turningNode2)
				except:
					continue
		return self.state		
		
	def __FindLinksNearPoint(self, point):
		#first check if the point is within an intersection's radius
		#print "Looking for links near "+str(point)
		possibleLinks = []
		for node in self.nodeList:
			maxWidth=0
			#create a circle with the width of the lanes surrounding it
			for link in self.nodeList[node].links:
				laneWidth = float(self.linkList[link].lanes)*self.laneWidth
				if laneWidth > maxWidth:
					maxWidth = laneWidth
			
			#if its within the radius of the circle from the centre of the intersection, then it could be on either link
			if (point - Vector2D(self.nodeList[node].x, self.nodeList[node].y)).Magnitude() < maxWidth:
				#we are near this intersection. Must consider all links connected to this intersection
				possibleLinks += self.nodeList[node].links
		
		#If we were near any node, we need to return now and consider all these links
		if len(possibleLinks) > 0:
			return possibleLinks
		
		#point is not near any node, lets continue and find the link that its closest to
		minDist = sys.maxint
		minLink = -1
		for link in self.linkList:
			linkSegment = self.linkList[link].returnSegment()
			linkDist = linkSegment.DistanceFromPoint(point)
			if linkDist < minDist:
				minLink = link
				minDist = linkDist
		return [minLink]
		



class Complex:
	real = 0
	imag = 0
	def __init__(self,r,i):
		self.real = r
		self.imag = i

	def __str__(self):
		return str(self.real)+"+j"+str(self.imag)

	def Magnitude(self):
		return math.sqrt( self.real*self.real + self.imag*self.imag )

materials = {}
materials['concrete'] = Complex(8.575,1.536)
materials['brick'] = Complex(3.74,0.16)
materials['gypsum'] = Complex(2.5,0.3)
materials['polycarbonate'] = Complex(2.86,0.05)

#Class BuildingExtractor: Extracts building data from an osm map
#Arguments: fileName: the name of the file to analyse, without extension.
class BuildingExtractor:

	buildings = []
	nodeSet = {}

	netOffset = 0
	projTransform = 0

	class Way():
		Id=0
		nodes=[]
		permittivity = 0;
		maxHeight = 0;
		heightStd = 0;

		def __init__(self,attr):
			self.Id = attr.getValue('id')
			self.nodes = []
			self.permittivity = materials['brick']
			self.maxHeight = 0.020396989;
			self.heightStd = 0.0025;

		def AddNode(self,nodeId,nodeSet):
			n = nodeSet[ nodeId ]
			self.nodes.append( n )

	class Node():
		Id=0
		position=0

		def __init__(self,attr,projTransform,netOffset):
			self.Id = attr.getValue('id')
			p = projTransform(float(attr.getValue('lon')), float(attr.getValue('lat')))
			self.position = (Vector2D(float(p[0]), float(p[1])) + netOffset)

	class BuildingHandler( xml.sax.ContentHandler ):
		isNode = False
		isWay = False
		isND = False
		isTag = False

		currentWay = 0
		currWayIsBuilding = False
		parent = None

		def __init__(self,parent):
			xml.sax.ContentHandler.__init__(self)
			self.parent = parent;
		
		def startElement( self, name, attrs ):
			if name == 'node':
				if self.isWay == False and self.isND == False and self.isTag == False and self.isNode == False:
					self.parent.nodeSet[attrs.getValue('id')] = BuildingExtractor.Node( attrs,self.parent.projTransform,self.parent.netOffset )
					self.isNode = True
			elif name == 'way':
				if self.isWay == False and self.isND == False and self.isTag == False and self.isNode == False:
					self.currentWay = BuildingExtractor.Way( attrs )
					self.currWayIsBuilding = False
					self.isWay = True
			elif name == 'nd':
				if self.isWay == True and self.isND == False and self.isTag == False:
					self.currentWay.AddNode( attrs.getValue('ref'), self.parent.nodeSet )
					self.isND = True
			elif name == 'tag':
				if self.isWay == True and self.isND == False and self.isTag == False:
					if attrs.getValue('k') == 'building' and attrs.getValue('v') == 'yes':
						self.currWayIsBuilding = True
					elif attrs.getValue('k') == 'material':
						self.currentWay.permittivity = materials[attrs.getValue('v')];
					elif attrs.getValue('k') == 'maxHeight':
						self.currentWay.maxHeight = attrs.getValue('v');
					elif attrs.getValue('k') == 'heightStd':
						self.currentWay.heightStd = attrs.getValue('v');
					self.isTag = True

		def endElement( self, name ):
			if name == 'node':
				self.isNode = False
			elif name == 'way':
				self.isWay = False
				if self.currWayIsBuilding == True:
					self.parent.buildings.append( self.currentWay )
				else:
					self.currWayIsBuilding = False
			elif name == 'nd':
				self.isND = False
			elif name == 'tag':
				self.isTag = False


	class LocationHandler( xml.sax.ContentHandler ):
		isLocation = False
		parent = None

		def __init__(self,parent):
			xml.sax.ContentHandler.__init__(self)
			self.parent = parent;


		def startElement( self, name, attrs ):
			if name == 'location':
				self.isLocation = True
				strOffset = attrs['netOffset'].split(',')
				self.parent.netOffset = Vector2D( float(strOffset[0]), float(strOffset[1]) )
				projStr = attrs['projParameter']
				self.parent.projTransform = pyproj.Proj( str(projStr) )

		def endElement( self, name ):
			if name == 'location':
				self.isLocation = False

	def __init__(self,fileName):
		self.buildings = []
		self.nodeSet = {}
		self.netOffset = 0
		self.projTransform = 0

		locationParser = xml.sax.make_parser()
		locationParser.setContentHandler( BuildingExtractor.LocationHandler(self) )
		locationParser.parse( open(fileName+".net.xml", 'r') )

		buildingParser = xml.sax.make_parser()
		buildingParser.setContentHandler( BuildingExtractor.BuildingHandler(self) )
		buildingParser.parse( open(fileName+".osm",'r') )

		self.nodeSet = None
