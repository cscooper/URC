#!/usr/bin/python -O
#######################################################
### VectorMath Python Library
### Consists of:
###		Vector2D
###		LineSegment2D
###
### Written by Abhinay Mukunthan and Craig Cooper
### Contact: lexxonnet@gmail.com, andor734@gmail.com
###
### License: GPLv3
#######################################################

import math
class Vector2D():
	def __init__(self, x, y):
		try:
			self.x = float(x)
			self.y = float(y)
		except:
			raise TypeError('Vector2D only accepts 2 int/float arguments')

	### Overload operators
	def __neg__(self):
		return Vector2D(-self.x, -self.y)
	
	def __add__(self, rhs):
		if type(rhs) == int or type(rhs) == float:
			return Vector2D(self.x+rhs, self.y+rhs)
		elif rhs.__class__.__name__ == 'Vector2D':
			return Vector2D(self.x+rhs.x,self.y+rhs.y)
		else:
			raise TypeError('Invalid input to + operator of Vector2D class. Input should be a float, int or Vector2D')
	
	def __radd__(self, lhs):
		return self.__add__(lhs)
		
	def __sub__(self, rhs):
		if type(rhs) == int or type(rhs) == float:
			return Vector2D(self.x-rhs, self.y-rhs)
		elif rhs.__class__.__name__ == 'Vector2D':
			return Vector2D(self.x-rhs.x,self.y-rhs.y)
		else:
			raise TypeError('Invalid input to - operator of the Vector2D class. Input should be a float, int or Vector2D')
		
	def __rsub__(self, lhs):
		return -self.__sub__(lhs)
	
	def __mul__(self, rhs):
		if type(rhs) == int or type(rhs) == float:
			return Vector2D(self.x*rhs, self.y*rhs)
		else:
			raise TypeError('Invalid input to the * operator of the Vector2D class. Input should be a float or int')
	
	def __rmul__(self, lhs):
		return self.__mul__(lhs)
	
	def __div__(self, rhs):
		if type(rhs) == int or type(rhs) == float:
			return Vector2D(self.x/rhs, self.y/rhs)
		else:
			raise TypeError('Invalid input to the / operator of the Vector2D class. Input should be a float or int')
	
	def __rdiv__(self, lhs):
		if type(rhs) == int or type(rhs) == float:
			return Vector2D(lhs/self.x, lhs/self.y)
		else:
			raise TypeError('Invalid input to the / operator of the Vector2D class. Input should be a float or int')
		
	def __eq__(self, rhs):
		try:
			if self.x == rhs.x and self.y == rhs.y:
				return True
		except:
			return False
	
	def __ne__(self, rhs):
		return not self.__eq__(rhs)
		
	def Dot(self, rhs):
		try:
			return (self.x*rhs.x + self.y*rhs.y)
		except AttributeError:
			raise TypeError('Invalid input to the Dot operator of Vector2D class. Input must be another Vector2D object')
	
	def Cross(self, rhs):
		try:
			return (self.x*rhs.y - self.y*rhs.x)
		except AttributeError:
			raise TypeError('Invalid input to the Dot operator of Vector2D class. Input must be another Vector2D object')
		
	def MagnitudeSq(self):
		return self.x**2 + self.y**2
		
	def Magnitude(self):
		return math.sqrt(self.MagnitudeSq())
		
	def Unitise(self):
		return self/self.Magnitude()
	
	##figure out the error here
	def AngleBetween(self, rhs):
		try:
			return (math.acos(self.Dot(rhs)/(rhs.Magnitude()*self.Magnitude())))
		except AttributeError:
			raise TypeError('Invalid input to the AngleBetween function of Vector2D class. Input must be another Vector2D object')
	
	def __str__(self):
		return '('+str(self.x)+','+str(self.y)+')'
		
	def __hash__(self):
		return hash((self.x,self.y))
		
	def Tuple(self):
		return (self.x, self.y)

class Vector3D():
	def __init__(self, x, y, z):
		try:
			self.x = float(x)
			self.y = float(y)
			self.z = float(z)
		except:
			raise TypeError('Vector3D only accepts 3 int/float arguments')
	
	### Overload operators
	def __neg__(self):
		return Vector3D(-self.x, -self.y, -self.z)
	
	def __add__(self, rhs):
		if type(rhs) == int or type(rhs) == float:
			return Vector3D(self.x+rhs, self.y+rhs, self.z+rhs)
		elif rhs.__class__.__name__ == 'Vector3D':
			return Vector3D(self.x+rhs.x,self.y+rhs.y,self.z+rhs.z)
		else:
			raise TypeError('Invalid input to + operator of Vector3D class. Input should be a float, int or Vector3D')
	
	def __radd__(self, lhs):
		return self.__add__(lhs)
		
	def __sub__(self, rhs):
		if type(rhs) == int or type(rhs) == float:
			return Vector3D(self.x-rhs, self.y-rhs, self.z-rhs)
		elif rhs.__class__.__name__ == 'Vector3D':
			return Vector3D(self.x-rhs.x,self.y-rhs.y,self.z-rhs.z)
		else:
			raise TypeError('Invalid input to - operator of the Vector3D class. Input should be a float, int or Vector3D')
		
	def __rsub__(self, lhs):
		return -self.__sub__(lhs)
	
	def __mul__(self, rhs):
		if type(rhs) == int or type(rhs) == float:
			return Vector3D(self.x*rhs, self.y*rhs, self.z*rhs)
		else:
			raise TypeError('Invalid input to the * operator of the Vector3D class. Input should be a float or int')
	
	def __rmul__(self, lhs):
		return self.__mul__(lhs)
	
	def __div__(self, rhs):
		if type(rhs) == int or type(rhs) == float:
			return Vector3D(self.x/rhs, self.y/rhs, self.z/rhs)
		else:
			raise TypeError('Invalid input to the / operator of the Vector3D class. Input should be a float or int')
	
	def __rdiv__(self, lhs):
		if type(rhs) == int or type(rhs) == float:
			return Vector3D(lhs/self.x, lhs/self.y, lhs/self.z)
		else:
			raise TypeError('Invalid input to the / operator of the Vector3D class. Input should be a float or int')
		
	def __eq__(self, rhs):
		try:
			if self.x == rhs.x and self.y == rhs.y and self.z == rhs.z:
				return True
		except:
			return False
	
	def __ne__(self, rhs):
		return not self.__eq__(rhs)
		
	def Dot(self, rhs):
		try:
			return (self.x*rhs.x + self.y*rhs.y + self.z*rhs.z)
		except AttributeError:
			raise TypeError('Invalid input to the Dot operator of Vector3D class. Input must be another Vector3D object')
	
	def Cross(self, rhs):
		try:
			return Vector3D(self.y*rhs.z - self.z*rhs.y,self.x*rhs.z - self.z*rhs.x,self.x*rhs.y - self.y*rhs.x)
		except AttributeError:
			raise TypeError('Invalid input to the Dot operator of Vector3D class. Input must be another Vector3D object')
		
	def MagnitudeSq(self):
		return self.x**2 + self.y**2 + self.z**2
		
	def Magnitude(self):
		return math.sqrt(self.MagnitudeSq())
		
	def Unitise(self):
		return self/self.Magnitude()
	
	##figure out the error here
	def AngleBetween(self, rhs):
		try:
			return (math.acos(self.Dot(rhs)/(rhs.Magnitude()*self.Magnitude())))
		except AttributeError:
			raise TypeError('Invalid input to the AngleBetween function of Vector3D class. Input must be another Vector3D object')
	
	def __str__(self):
		return '('+str(self.x)+','+str(self.y)+','+str(self.z)+')'
		
class LineSegment2D:
	#either LineSegment2D(Vector2D, Vector2D) or LineSegment2D(x1,y1,x2,y2)
	def __init__(self,*arg):
		if len(arg) == 2:
			if arg[0].__class__.__name__=='Vector2D' and arg[1].__class__.__name__=='Vector2D':
				self.start = arg[0]
				self.end = arg[1]
			else:
				raise TypeError('LineSegment2D accepts only 2 Vector2D arguments or 4 int/float arguments')
		elif len(arg) == 4:
			self.start = Vector2D(arg[0],arg[1])
			self.end = Vector2D(arg[2],arg[3])
		else:
			raise TypeError('LineSegment2D accepts only 2 Vector2D arguments or 4 int/float arguments')
	
	def __CounterClockWise(self,a,b,c):
		return((c.y-a.y)*(b.x-a.x) > (b.y-a.y)*(c.x-a.x))
		
	def IntersectSegment(self, l2, pVec=False):
		try:
			if (self.__CounterClockWise(self.start, l2.start, l2.end) != self.__CounterClockWise(self.end, l2.start, l2.end)) and (self.__CounterClockWise(self.start, self.end, l2.start) != self.__CounterClockWise(self.start, self.end, l2.end)):
				if pVec:
					u = self.start - self.end
					v = l2.end - l2.start
					w = self.start - l2.start
					return l2.start + v * ((u * w)/(u * v))
				return True
			else:
				return False
		except AttributeError:
			raise TypeError("LineSegment2D's method IntersectSegment only takes in another Linesegment as an argument")
	
	def __eq__(self, rhs):
		return self.start == rhs.start and self.end == rhs.end
	
	def __ne__(self, rhs):
		return not self.__eq(rhs)
		
	def GetVector(self):
		return self.end-self.start
	
	def GetDirection(self):
		vec = self.GetVector()
		return vec/vec.Magnitude()
		
	def GetDistance(self):
		return self.GetVector().Magnitude()
	
	def GetNormal(self):
		return Vector2D(self.end.y-self.start.y, self.start.x-self.end.x).Unitise()
		
	def GetGradient(self):
		return ((self.end.y-self.start.y)/(self.end.x-self.start.x))
	
	def DistanceFromPoint(self, p):
		if p.__class__.__name__ != 'Vector2D':
			raise TypeError("LineSegment2D's method DistanceFromPoint only takes in a Vector2D argument")
		t = (p-self.start).Dot(self.end-self.start) / ((self.end.x-self.start.x)**2 + (self.end.y-self.start.y)**2)
		if t < 0.0:
			return (p-self.start).Magnitude()
		elif t > 1.0:
			return (p-self.end).Magnitude()
		else:
			return (p-(self.start+(self.end-self.start)*t)).Magnitude()
	
	def DistanceAlongLine(self, p):
		if p.__class__.__name__ != 'Vector2D':
			raise TypeError("LineSegment2D's method DistanceAlongLine only takes in a Vector2D argument")
		return self.GetVector().Unitise().Dot(self.start-p)
	
	def PointInCommon(self, l):
		if l.__class__.__name__ != 'LineSegment2D':
			raise TypeError("LineSegment2D's method PointInCommon only takes in another LineSegment2D argument")
		return self.start == l.start or self.end == l.end or l.end == self.start or l.end == self.end
				
	def Midpoint(self):
		return (self.end+self.start)/2
	
	def __str__(self):
		return str(self.start)+"---"+str(self.end)
		
'''	Rectangle:
	inputs: 	bottom_x, bottom_y, width, height
	or		Vector2D location, Vector2D Dimensions
'''
class Rectangle:
	def __init__(self, *args):
		if len(args) == 4:
			try:
				self.location = Vector2D(args[0], args[1])
				self.size = Vector2D(args[2], args[3])
			except:
				raise TypeError('Class Rectangle takes in 4 int/float arguments')
		elif len(args) == 2:
			if self.args[0].__class__.__name__ == 'Vector2D' and self.args[1].__class__.__name__ == 'Vector2D':
				self.location = args[0]
				self.size = args[1]
			else:
				raise TypeError('Class Rectangle takes in 2 Vector2D arguments')
		
	def Scale(self, factor, vecPoint):
		try:
			newSize = self.size*factor
			newLoc = (vecPoint * -factor) + self.location + self.size / 2
			return Rectangle(newLoc, newSize)
		except:
			raise TypeError("Rectangle's scale method takes in a float and vector as arguments")
		
	def Centre(self, absolute):
		if absolute:
			return size/2 + location
		else:
			return size/2
	
	def PointWithin(self, point):
		try:
			return (point.x > self.location.x and point.y > self.location.y and point.x < (self.location.x + self.size.x) and poiny.y < (self.location.y + self.size.y))
		except AttributeError:
			raise TypeError("Rectangle's method PointWithin takes in a single Vector2D argument")
		
	
	def IntersectsRect(self, rect):
		return self.PointWithin(rect.location) or self.PointWithin(Vector2D(rect.location.x + rect.size.x, rect.location.y)) or self.PointWithin(Vector2D(rect.location.x, rect.location.y + rect.size.y)) or PointWithin(Vector2D(rect.location.x + rect.size.y, rect.location.y.y + rect.size.y))
		
	def LineSegmentWithin(self, line):
		try:
			if (PointWithin(line.start) or PointWithin(line.end)):
				return true
		except AttributeError:
			raise TypeError("LineSegmentWithin takes in a single LineSegment2D argument")
	
		addWidth = Vector2D(self.size.x, 0)
		addHeight = Vector2D(0, self.size.y)
		leftLine = LineSegment2D(self.location, self.location+addHeight)
		bottomLine = LineSegment2D(self.location, self.location+addWidth)
		rightLine = LineSegment2D(self.location+addWidth, self.location+size)
		topLine = LineSegment2D(self.location+addHeight, self.location+size)
		
		if line.IntersectSegment(leftLine) or line.IntersectSegment(rightLine) or line.IntersectSegment(bottomLine) or line.IntersectSegment(topLine):
			return true
		
		return false
	
	
	
		
	