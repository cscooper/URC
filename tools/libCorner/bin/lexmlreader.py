import xml.sax.handler

class UrbanReader(xml.sax.handler.ContentHandler):
	def __init__(self):
		self.inBuilding=0
		self.inFace=0
		self.inPosition=0
		self.buildings=[]
		self.facelist=[]
		self.patchlist=[]
	def startElement(self, name, attributes):
		if name == "Building":
			self.inBuilding=1
			self.facelist=[]
		elif name == "face":
			self.inFace=1
			self.patchlist=[]
		elif name == "position":
			self.inPosition=1
			self.position_buf=""
	
	def characters(self, data):
		if self.inPosition and self.inFace and self.inBuilding:
			self.position_buf += data
	
	def endElement(self, name):
		if name == "position":
			self.inPosition = 0
			if self.inFace and self.inBuilding:
				thispatch={}
				thispatch['x']=float(self.position_buf.split()[0])
				thispatch['y']=float(self.position_buf.split()[1])
				thispatch['z']=float(self.position_buf.split()[2])
				self.patchlist.append(thispatch)
		elif name == "face":
			self.inFace = 0
			if self.inBuilding:
				self.facelist.append(self.patchlist)
		elif name == "Building":
			self.inBuilding = 0
			self.buildings.append(self.facelist)

def readQualnetTerrain(terrainfile):
	parser = xml.sax.make_parser()
	terrain = UrbanReader()
	parser.setContentHandler(terrain)
	parser.parse(terrainfile)
	return terrain
	