class Instruction:
	def __init__(self, instr):
		self.intstruction = instr
		self.type = 'Default'

		

class BasicBlock:
	def __init__(self):
		self.instructions = list()
		self.Next = None
		self.Then = None
		self.Else = None

	def appendIntruction(self, instr):
		self.instructions.append(instr)

	def setNext(self, n):
		self.Next = n

	def setThen(self, t):
		self.Then = t

	def setElse(self, e):
		self.Else = e



class Function:
	def __init__(self, codefile):
		self.allowedDataTypes = ['int','double']
		self.params = dict()
		self.tree = list()
		self.content = []
		self.header = ''
		self.currLineNo = 0
		with open(codefile) as f:
			self.content = f.readlines()
		self.header = self.content[0]
		self.content.remove(self.content[0])
		for i in range(len(self.content)):
			x = self.content[i]
			self.content[i] = self.content[i].strip()
			for t in self.allowedDataTypes:
				if (x[0:len(t)] == t):
					# self.content[i] = self.content[i].replace(" ", "")
					break



	def getParams(self):
		start = self.header.index('(')
		end  = self.header.index(')')
		temp = self.header[start:(end+1)]
		params_pairs = temp.split(",")
		for x in params_pairs:
			curr_arg = x.split(" ")
			self.params[curr_arg[0]] = curr_arg[1]

	def doBranch(self,parent_block):
		currBlock = BasicBlock()
		self.tree.append(currBlock)
		l = len(self.content)
		line = self.content[self.currLineNo] 

		if line[0:2] == 'if': 
			print "if detected"
			parent_block.setThen(currBlock)
		elif line[0:4] == 'else':
			print "else detected"
			parent_block.setElse(currBlock)
		self.currLineNo = self.currLineNo + 1;

		branches = 0
		nextBlock = BasicBlock()
		newBlock = 0

		while 1:
			if self.content[self.currLineNo] == '}':
				break
			line = self.content[self.currLineNo]
			inst = Instruction(line)
			if line[0:2] == 'if' or line[0:4] == 'else':
				print "going to branch"
				print line
				# self.currLineNo = self.currLineNo - 1
				branches = branches + 1;
				if newBlock == 1:
					newBlock = 0
					currBlock.setNext(nextBlock)
					self.tree.append(nextBlock)
					currBlock = nextBlock
					nextBlock = BasicBlock()
				# for x in currBlock.instructions:
				# 	print x.intstruction
				# st = raw_input("continue? : ");

				self.doBranch(currBlock)
			elif branches > 0:
				nextBlock.appendIntruction(inst)
				newBlock = 1
			else:
				currBlock.appendIntruction(inst)
			
			self.currLineNo = self.currLineNo + 1

		currBlock.setNext(nextBlock)
		self.tree.append(nextBlock)

	def doBranchIndependent(self,currBlock):
		# currBlock = BasicBlock()
		self.tree.append(currBlock)
		l = len(self.content)
		line = self.content[self.currLineNo]


		branches = 0
		nextBlock = BasicBlock()
		newBlock = 0

		while 1:
			if self.content[self.currLineNo] == '}':
				break
			line = self.content[self.currLineNo]
			inst = Instruction(line)
			if line[0:2] == 'if' or line[0:4] == 'else':
				tmp = BasicBlock()
				branches = branches + 1;
				if newBlock == 1:
					newBlock = 0
					currBlock.setNext(nextBlock)
					self.tree.append(nextBlock)
					currBlock = nextBlock
					nextBlock = BasicBlock()
				if line[0:2] == 'if': 
					currBlock.setThen(tmp)
				elif line[0:4] == 'else':
					currBlock.setElse(tmp)
				self.currLineNo = self.currLineNo + 1;
				self.doBranchIndependent(tmp)
			elif branches > 0:
				nextBlock.appendIntruction(inst)
				newBlock = 1
			else:
				currBlock.appendIntruction(inst)
			
			self.currLineNo = self.currLineNo + 1

		currBlock.setNext(nextBlock)
		self.tree.append(nextBlock)

	def computeTreeI(self):
		root = BasicBlock();
		self.doBranchIndependent(root)
		x = root
		print "root is"
		for r in x.instructions:
			print r.intstruction

		print "root ka if is"
		x = root.Then
		for r in x.instructions:
			print r.intstruction

		print "root ka if ka if is"
		x = root.Then.Then
		for r in x.instructions:
			print r.intstruction

		print "root ka if ka else is"
		x = root.Then.Else
		for r in x.instructions:
			print r.intstruction

		print "root ka if ka next is"
		x = root.Then.Next
		for r in x.instructions:
			print r.intstruction

		print "root ka if ka next ka if is"
		x = root.Then.Next.Then
		for r in x.instructions:
			print r.intstruction

		print "root ka if ka next ka else is"
		x = root.Then.Next.Else
		for r in x.instructions:
			print r.intstruction
			

		print "root ka if ka next ka next is"
		x = root.Then.Next.Next
		for r in x.instructions:
			print r.intstruction

		print "root ka else is"
		x = root.Else
		for r in x.instructions:
			print r.intstruction

		print "root ka else ka if is"
		x = root.Else.Then
		for r in x.instructions:
			print r.intstruction

		print "root ka else ka else is"
		x = root.Else.Else
		for r in x.instructions:
			print r.intstruction

		print "root ka else ka  next is"
		x = root.Else.Next
		for r in x.instructions:
			print r.intstruction

		print "root ka next is"
		x = root.Next
		for r in x.instructions:
			print r.intstruction

	def computeTree(self):
		print "big bang!"
		root = BasicBlock()
		self.tree.append(root)
		l = len(self.content)
		while 1:
			line = self.content[self.currLineNo]
			inst = Instruction(line)
			root.appendIntruction(inst)
			if line[0:2] == 'if':
				self.doBranch(root)
			elif line[0:4] == 'else':
				self.doBranch(root)
			self.currLineNo = self.currLineNo + 1;
			if len(self.content) <= self.currLineNo:
				break 
		# self.currLineNo = self.currLineNo - 1;
		for r in root.instructions:
			print r.intstruction

		# st = raw_input("continue? : ");
		# self.doBranch(root)

		print "complete tree: "

		root = self.tree[0]

		x = root
		print "root is"
		for r in x.instructions:
			print r.intstruction
		
		print "root ka if is"
		x = root.Then
		for r in x.instructions:
			print r.intstruction

		print "root ka if ka if is"
		x = root.Then.Then
		for r in x.instructions:
			print r.intstruction

		print "root ka if ka else is"
		x = root.Then.Else
		for r in x.instructions:
			print r.intstruction

		print "root ka if ka next is"
		x = root.Then.Next
		for r in x.instructions:
			print r.intstruction

		print "root ka if ka next ka if is"
		x = root.Then.Next.Then
		for r in x.instructions:
			print r.intstruction

		print "root ka if ka next ka else is"
		x = root.Then.Next.Else
		for r in x.instructions:
			print r.intstruction

		print "root ka if ka next ka next is"
		x = root.Then.Next.Next
		for r in x.instructions:
			print r.intstruction

		print "root ka else is"
		x = root.Else
		for r in x.instructions:
			print r.intstruction

		print "root ka else ka if is"
		x = root.Else.Then
		for r in x.instructions:
			print r.intstruction

		print "root ka else ka else is"
		x = root.Else.Else
		for r in x.instructions:
			print r.intstruction

		print "root ka else ka  next is"
		x = root.Else.Next
		for r in x.instructions:
			print r.intstruction

		print "root ka next is"
		x = root.Next
		for r in x.instructions:
			print r.intstruction

		# print "root ka else is"
		# x = root.Else
		# for r in x.instructions:
		# 	print r.intstruction

		# print "root ka next is"
		# root = root.Next
		# for r in root.instructions:
		# 	print r.intstruction

		# for x in self.tree:
		# 	for y in x.instructions:
		# 		print y.intstruction 




	def something(self):
		tree = list()
		parent = BasicBlock()
		root = parent
		tree.append(currBlock)
		l = len(content)
		i = 0
		while 1:
			line = content[i]
			if line[0:2] == 'if':
				temp_currBlock = BasicBlock()
				currBlock.setThen(temp_currBlock)

		for line in content:
			if line[0:2] == 'if':
				currBlock.setThen
			if line == '{':
				tree.append(currBlock)
				temp_currBlock = BasicBlock()

			elif line == '{':
				continue
			else :
				currBlock.appendIntruction()	


f =  Function('foo2.cpp')
f.getParams()
# f.computeTree()
f.computeTreeI()