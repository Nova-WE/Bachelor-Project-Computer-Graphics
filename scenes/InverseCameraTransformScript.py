
import bpy
from mathutils import Matrix

# Get the active scene camera
cam = bpy.context.scene.camera

# Compute inverse camera transform
view_matrix = cam.matrix_world.inverted()

# Transform all mesh objects into camera space
for obj in bpy.data.objects:
    if obj.type == 'MESH':

        # Apply:
        # inverse(camera transform) * object transform
        mesh_transform = view_matrix @ obj.matrix_world

        # Permanently transform mesh vertices
        obj.data.transform(mesh_transform)

        # Reset object transform to identity
        obj.matrix_world.identity()
        
    if obj.type == "LIGHT":
        obj.matrix_world = view_matrix @ obj.matrix_world
                
# resez camera position to origin        
cam.matrix_world = Matrix.Identity(4)