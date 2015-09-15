import re

ASSIGNMENT_INSTRUCTION = 1
BRANCH_INSTRUCTION = 2
prog = re.compile("[a-zA-Z_][a-zA-Z0-9_]{0,31}")
class BasicBlock:
	def __init__(self):
		self.root = null
		self.leaves = null # should contain the path condition and the symbol table at the leaf
	
	def getPaths(self):
		self.leaves = root.getPaths()


class BasicBlockNode:
	def __init__(self):
		self.instructions = []
		self.Next = null
		self.Then = null
		self.Else = null

	def getPaths(self,currTable,currCondition):
		for instruction in self.instructions:
			temp = getInstructionType(instruction)
			if (temp[0] == ASSIGNMENT_INSTRUCTION):
				updateCurrSymbolTable(temp,currTable)
			elif (temp[0] == BRANCH_INSTRUCTION):
				pathSymbs = []
				if(self.Then != null):
					pathSymbs = pathSymbs + getPaths(self.Then, currTable, currCondition + "and" + temp[1])
				if(self.Else != null):
					pathSymbs = pathSymbs + getPaths(self.Else,currTable,currCondition + "and not(" + temp[1]+")")
				if(self.Next != null):
					pathSymbs2  = getPaths(self.Next,currTable,[])
					retPathSymbs = extendPaths(pathSymbs,pathSymbs2)
					return (retPathSymbs)
				else:
					return pathSymbs

		return (currCondition,currTable)

	def extendPathSymbs(pathSymbs,pathSymbs2):
		for (pathSymb) in pathSymbs:
			for (pathSymb2) in pathSymbs2:
				extendPathSymb(pathSymb,pathSymb2)

	def extendPathSymb(pathSymb,pathSymb2):
		symbolTable2 = pathSymb2[1]
		symbolTable = pathSymb[1]
		retPathSymbs = []
		for (key,val) in symbolTable2:
			if(val[1] == 0 and symbolTable.get(key,('blah',0))[1] == 1):
				varss = getVarsInExp(val[0])
				for var in varss:
					if(symbolTable2.get(var,0) == 0 and symbolTable.get(var,0) == 1):
						updateCurrSymbolTable(var,symbolTable[var],False,[],symbolTable2)
						retPathSymbs = retPathSymbs + (pathSymb[0],symbolTable2)



	def updateCurrSymbolTable(lhs,rhs,isConstant,vars,currTable):
		if(isConstant):
			currTable[lhs] = (rhs,1)
		else:
			known = True
			for var in vars:
				if(not currTable.get(var,False)):
					known = False
					break
			currTable[lhs] = (expandVars(rhs,symbolTable),known)

	def expandVars(expression,symbolTable):
		#replace variables with their values in terms of consts and function inputs
		varss = getVarsinExp(expression)
		for var in varss:
			if (not isAConstant(var) and not symbolTable.get(var,false)):
				return [-1]
			else:
				expression = expression.replace(" " + var + " ",symbolTable[var])
	def getVarsinExp(expression):
		# returns all the variables in the expression
		return prog.findall(expression)

	def getInstructionType(instruction):
		#returns array containing type of instruction, plus some other data

	def isAssignmentInstruction(instruction):
		#return array containing either False, or True,LHS,RHS
		if( " = " in instruction):
			lhs,expression = instruction.split(" = ")
			lhs = lhs.strip()
			varss = getVarsinExp(rhs)
			temp = expandVars(expression,symbolTable)
			if(temp[0] == -1):
				return temp
			else:
				return (ASSIGNMENT_INSTRUCTION,lhs)+temp
		else:
			return [-1]

	def isAConstant(word):
		toReturn = True
		for char in word:
			if(not(char >= '0' and char <= '9')):
				toReturn = False
				break
		return toReturn

	def isBranchInstruction(instruction,symbolTable):
		if ("if" in instruction):
			condition = instruction.replace("if","")
			varss = getVarsinExp(instruction)
			temp = expandVars(expression,symbolTable)
			if(temp[0] == -1):
				return temp
			else:
				return (BRANCH_INSTRUCTION)+temp
		else:
			return [-1]

def findPaths(function,inputs):
	symbols = {}
	for input in inputs:
		symbols[input] = (input,1)
	basicblocks = getBasicBlocks(function)

