import sys
import os



# Vulkan SDK Directory.
vulaknSDK = "C:/VulkanSDK/1.2.162.1/"
glslc = vulaknSDK + "Bin/glslc.exe"

# Shader Relative Directory.
shaderDir = "/Resources/Shaders/"

# Output Directory.
spvDir = "/Resources/Shaders/SPV/"


# Add Shader & their type to this array...
# (TYPE, PATH, MACRO, POSTFIX)
shaders = [ ]

def AddShader(stype, spath, smacro = "", postfix = ""):
	shaders.append( (stype, spath, smacro, postfix) )




# --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- ---
# Add Shaders...
AddShader("VERTEX", "ScreenVert.glsl")
AddShader("FRAGMENT", "PostProcess.glsl")
AddShader("FRAGMENT", "LightingPass.glsl")
AddShader("FRAGMENT", "FinalBlit.glsl")

# Mesh Shaders...
AddShader("VERTEX",   "MeshVert.glsl")
AddShader("FRAGMENT", "MeshFrag.glsl")

AddShader("VERTEX",   "MeshVert.glsl", "-D=PIPELINE_STAGE_DIR_SHADOW", "_DirShadow")
AddShader("FRAGMENT", "MeshFrag.glsl", "-D=PIPELINE_STAGE_DIR_SHADOW", "_DirShadow")

AddShader("VERTEX",   "MeshVert.glsl", "-D=PIPELINE_STAGE_OMNI_SHADOW", "_OmniShadow")
AddShader("FRAGMENT", "MeshFrag.glsl", "-D=PIPELINE_STAGE_OMNI_SHADOW", "_OmniShadow")







# --- -- - -- --- --- -- - -- --- --- -- - -- --- --- -- - -- ---
# Run...
print("RealTimeGI - Prebuild...")

shaderOutDirAbs = os.getcwd() + shaderDir + "SPV/"

if not os.path.exists(shaderOutDirAbs):
	os.mkdir(shaderOutDirAbs)

for shader in shaders:
	shaderSrcPath = os.getcwd() + shaderDir + shader[1]

	shaderSPVPath = shaderOutDirAbs + shader[1]
	shaderSPVPath = os.path.splitext(shaderSPVPath)[0] + shader[3] + ".spv"

	ft = os.path.getmtime(shaderSrcPath)
	gt = ft

	if os.path.exists(shaderSPVPath):
		gt = os.path.getmtime(shaderSPVPath)


	if ft >= gt:
		print("Compile Shader ->", shaderDir + os.path.splitext(shader[1])[0] + shader[3] + ".spv")
		cmd = "{0} -fshader-stage={4} {1} -o {2} {3}".format(
			glslc, shaderSrcPath, shaderSPVPath,
			shader[2], shader[0].lower()
		)

		sys.stdout.flush()
		r = os.system(cmd)

		if r != 0:
			exit(2)

