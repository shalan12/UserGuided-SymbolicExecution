with open('foo.cpp') as f:
    content = f.readlines()

i = -1;
while 1:
	l = len(content)
	i = i + 1;
	if (i >= l):
		break
	content[i] = content[i].strip()
	content[i] = content[i].replace(" ", "")
	# if content[i] == "{" or content[i] == "}":
	# 	content.remove(content[i])
	# 	i = i - 1;
	# print(content[i])

for c in content:
	print(c)

