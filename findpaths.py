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

	def getPaths(self,pathConditions,symbolTables,currTable,currCondition):
		for instruction in self.instructions:
			temp = getInstructionType(instruction)
			if (temp[0] == ASSIGNMENT_INSTRUCTION):
				updateCurrSymbolTable(self,temp[1],temp[2],currTable)
			elif (temp[0] == BRANCH_INSTRUCTION):
				if(self.Then != null):
					getPaths(self.Then, pathConditions,symbolTables, currTable, currCondition + "and" + temp[1])
				if(self.Else != null):
					getPaths(self.Else,pathConditions,symbolTables,currTable,currCondition + "and" + temp[1])

		return (pathConditions+currCondition,symbolTables+currTable)



		
def isAssignmentInstruction(instruction):
	#return array containing either False, or True,LHS,RHS

def findPaths(function,inputs):
	symbols = {}
	for input in inputs:
		symbols[input] = input
	basicblocks = getBasicBlocks(function)

