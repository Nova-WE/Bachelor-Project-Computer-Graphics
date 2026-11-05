import numpy as np
import matplotlib.pyplot as plt



def uniform_hemisphere_sampling(num_samples=5000):
    eta1 = np.random.uniform(0, 1, num_samples)
    eta2 = np.random.uniform(0, 1, num_samples)

    # Generate random samples in spherical coordinates
    theta = np.acos(1 - eta1)
    phi = 2 * np.pi * eta2

    # Convert spherical coordinates to Cartesian coordinates
    x = np.sin(theta) * np.cos(phi)
    y = np.sin(theta) * np.sin(phi)
    z = np.cos(theta)

    return x, y, z


def cosine_hemisphere_sampling(num_samples=5000):
    eta1 = np.random.uniform(0, 1, num_samples)
    eta2 = np.random.uniform(0, 1, num_samples)

    # Generate random samples in spherical coordinates
    phi = 2 * np.pi * eta2

    # Convert spherical coordinates to Cartesian coordinates
    x = np.cos(phi) * np.sqrt(eta1)
    y = np.sin(phi) * np.sqrt(eta1)
    z = np.sqrt(1 - eta1)

    return x, y, z


def plot_hemisphere_samples(title, x, y, z):
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    ax.scatter(x, y, z, s=1)
    ax.set_xlabel('X axis')
    ax.set_ylabel('Y axis')
    ax.set_zlabel('Z axis')
    ax.set_title(title)
    plt.show()


if __name__ == "__main__":
    plot_hemisphere_samples("Uniform Hemisphere Sampling", *uniform_hemisphere_sampling(2000))
    plot_hemisphere_samples("Cosine Hemisphere Sampling", *cosine_hemisphere_sampling(2000))
