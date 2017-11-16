import matplotlib.pyplot as plt

f = open("benchmark.txt", "r")
s = f.readline()
arr = [[], [], [], []]
while s != "":
	nums = s.split(',');
	for i in range(4):
		arr[i].append(float(nums[i]))

	s = f.readline()

print("Should plot now")
plt.plot(arr[0], arr[1])
plt.hold(True)
plt.plot(arr[0], arr[2])
plt.hold(True)
plt.plot(arr[0], arr[3])
plt.show()

