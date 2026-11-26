--[[
	RESOURCES ENUMS
]]
_G.RESOURCES = {
	GLTF = {
		NONE = 0,

		IMPORT_LIGHT = bit32.lshift(1, 1),
		IMPORT_TEXTURES = bit32.lshift(1, 2), -- Aka 1 << 2
		IMPORT_ANIMATIONS = bit32.lshift(1, 3),
		IMPORT_BLEND_SHAPES = bit32.lshift(1, 4),

		CALCULATE_BBOX = bit32.lshift(1, 5),
		VALIDATE = bit32.lshift(1, 6),

		DEBUG = {
			PRINT_BONE_STRUCTURE = bit32.lshift(1, 10),
			PRINT_MATERIALS = bit32.lshift(1, 11),
			PRINT_ANIMATIONS = bit32.lshift(1, 12),
			PRINT_BLENDSHAPES = bit32.lshift(1, 13),
			PRINT_OPTIMIZATION_STATS = bit32.lshift(1, 14)
		},

		OPTIMIZER = {
			MESH_OPTIMIZE = bit32.lshift(1, 20),
			MESH_SIMPLIFY = bit32.lshift(1, 21),
			SKELETON_ANIMATIONS = bit32.lshift(1, 22)
		}
	},
	TEXTURE = {
		NONE = 0,
		DYNAMIC = bit32.lshift(1, 30),
		NO_MIPMAPS = bit32.lshift(1, 31)
	},
	WEBM = {
		NONE = 0,
		PRELOAD = bit32.lshift(1, 1),
		PAUSE_WHEN_HIDDEN = bit32.lshift(1, 2)
	}
}