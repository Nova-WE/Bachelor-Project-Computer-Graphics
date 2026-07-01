import matplotlib.pyplot as plt

buildTimes = [3710, 3919, 4559, 6181, 12006, 33640, 114538]
renderTimes = [8468, 8246, 7206, 7473, 7246, 7195, 7312]
buckets = [2 ** i for i in range(1, 8)]

plt.plot(buckets, buildTimes)
plt.plot(buckets, renderTimes)
plt.show()

plt.plot(buildTimes, renderTimes, 'ro')
plt.show()
