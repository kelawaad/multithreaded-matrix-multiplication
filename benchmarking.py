import matplotlib.pyplot as plt

def printNumbers(arr):
	for i in range(len(arr)):
		print(arr[i])


order = ['Non-threaded', 'Thread/Element', 'Thread/Row']

f = open("benchmark.txt", "r")
s = f.readline()
arr = [[], [], [], []]
while s != "":
	nums = s.split(',')
	for i in range(4):
		arr[i].append(float(nums[i]))

	s = f.readline()

printNumbers(arr)
print("Should plot now")
plt.plot(arr[0], arr[1])
plt.hold(True)
plt.plot(arr[0], arr[2])
plt.hold(True)
plt.plot(arr[0], arr[3])

plt.xlabel('Number of elements per row')
plt.ylabel('time(sec)')
plt.legend(order, loc=0)
plt.show()

