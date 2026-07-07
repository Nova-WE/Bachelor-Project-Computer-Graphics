import matplotlib.pyplot as plt

buildTimes = [13681, 15170, 16944, 22113, 39744, 103734]
renderTimes = [25823, 25888, 22506, 20500, 20100, 19501]
buckets = [2 ** i for i in range(1, 7)]

fig, ax = plt.subplots()

ax.scatter(buildTimes, renderTimes)
for i, txt in enumerate(buckets):
    ax.annotate(txt, (buildTimes[i], renderTimes[i]))

plt.show()
