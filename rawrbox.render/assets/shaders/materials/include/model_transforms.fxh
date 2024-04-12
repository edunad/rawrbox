#ifndef INCLUDED_MODEL_TRANSFORMS
#ifdef INCLUDED_CAMERA
	#define INCLUDED_MODEL_TRANSFORMS

	struct TransformedData {
		half4 pos;
		half4 final;
	};

	// Snap vertex to achieve PSX look
	half4 PSXTransform(half4 vertex, half2 resolution) {
		half4 snappedPos = vertex;
		snappedPos.xyz = vertex.xyz / vertex.w;                         // convert to normalized device coordinates (NDC)+
		snappedPos.xy = floor(resolution * snappedPos.xy + 0.5) / resolution;
		snappedPos.xyz *= vertex.w;                                     // convert back to projection-space

		return snappedPos;
	}

	// Billboard transformation
	half4 billboardTransform(half4 vertex, uint billboard) {
		half3 right = CAMERA_RIGHT;
		half3 up = CAMERA_UP;

		if ((billboard & 2) != 0) { // X
			right = half3(Camera.view[0][0], Camera.view[1][0], Camera.view[2][0]);
		}

		if ((billboard & 4) != 0) { // Y
			up = half3(Camera.view[0][1], Camera.view[1][1], Camera.view[2][1]);
		}

		return half4((right * vertex.x) + (up * vertex.y), 1.);
	}

		#ifdef SKINNED
		#ifdef TRANSFORM_BONES
			half4 boneTransform(uint4 indices, half4 weight, half4 position) {
				half4x4 BoneTransform = half4x4(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
				bool skinned = false;

				for (uint idx = 0; idx < MAX_BONES_PER_VERTEX; idx++) {
					if (weight[idx] > 0.0) {
						BoneTransform += SkinnedConstants.bones[indices[idx]] * weight[idx];
						skinned = true;
					}
				}

				return skinned ? mul(BoneTransform, position) : position;
			}
		#endif
	#endif

	// Apply model transforms
	TransformedData applyPosTransforms(float4x4 proj, half4 a_position, half2 a_texcoord0) {
		TransformedData data;
		data.pos = a_position;

		// displacement mode
		#ifdef TRANSFORM_DISPLACEMENT
		if (DisplacementPower != 0. && DisplacementTexture != 0.) {
			data.pos.y += (min16float)g_Textures[DisplacementTexture].SampleLevel(g_Sampler, half3(a_texcoord0, 0), 0).x * DisplacementPower;
		}
		#endif
		// ----

		// Billboard mode
		#ifdef TRANSFORM_BILLBOARD
		if (Billboard != 0.) {
			data.pos = billboardTransform(data.pos, asint(Billboard));
		}
		#endif
		// ----

		// vertex_snap mode
		#ifdef TRANSFORM_PSX
		if (VertexSnap != 0.) {
			data.final = PSXTransform(mul(data.pos, proj), ScreenSize / VertexSnap);
		} else {
			data.final = (half4)mul(data.pos, proj);
		}
		#else
		data.final = mul(data.pos, proj);
		#endif
		// ----

		return data;
	}

	TransformedData applyPosTransforms(half4 a_position, half2 a_texcoord0) {
		return applyPosTransforms(Camera.worldViewProj, a_position, a_texcoord0);
	}

	TransformedData applyPosTransforms(half3 a_position, half2 a_texcoord0) {
		return applyPosTransforms(Camera.worldViewProj, half4(a_position, 1.0), a_texcoord0);
	}

	TransformedData applyPosTransforms(float4x4 proj, half3 a_position, half2 a_texcoord0) {
		return applyPosTransforms(proj, half4(a_position, 1.0), a_texcoord0);
	}
	// ----------------------
#endif
#endif
