# rendered the hairball at 1000px resolution and 10 samples per pixel

import matplotlib.pyplot as plt

buildTimes = [13019, 14311, 16041, 21368, 39509, 104399, 13731]
renderTimes = [29950, 29794, 25314, 22644, 21984, 21637, 27681]
buckets = [2 ** i for i in range(1, 7)]

fig, ax = plt.subplots()

ax.scatter(buildTimes, renderTimes)
for i, txt in enumerate(buckets):
    ax.annotate("  " + str(txt), (buildTimes[i], renderTimes[i]))

plt.xlabel("build time")
plt.ylabel("render time")

ax.annotate("  dynamic", (buildTimes[-1], renderTimes[-1]))

plt.show()
