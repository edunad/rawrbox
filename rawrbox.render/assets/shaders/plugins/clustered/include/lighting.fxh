
#ifdef LIGHT_UNIFORMS
#ifdef READ_LIGHTS
#ifdef READ_CLUSTER_DATA_GRID

    #ifndef INCLUDED_LIGHTING
        #define INCLUDED_LIGHTING

        #include <math.fxh>

        struct LightResult {
            float3 Diffuse;
            float3 Specular;
        };

		float3 Diffuse_Lambert(float3 diffuseColor) {
			return diffuseColor * INV_PI;
		}

        // Angle >= Umbra -> 0
		// Angle < Penumbra -> 1
		// Gradient between Umbra and Penumbra
		float DirectionalSpotAttenuation(float3 L, float3 direction, float cosUmbra, float cosPenumbra) {
			float cosAngle = dot(-normalize(L), direction);
			float falloff = saturate((cosAngle - cosUmbra) / (cosPenumbra - cosUmbra));
			return falloff * falloff;
		}

        float DirectionalAttenuation(float3 L, float3 direction) {
			float cosAngle = dot(-normalize(L), direction);
			return cosAngle;
		}

        // Distance between rays is proportional to distance squared
		// Extra windowing function to make light radius finite
		// https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
		float RadialAttenuation(float3 L, float range) {
			float distSq = dot(L, L);
			float distanceAttenuation = 1 / (distSq + 1);
			float windowing = Square(saturate(1 - Square(distSq * Square(rcp(range)))));
			return distanceAttenuation * windowing;
		}

        float GetAttenuation(Light light, float3 worldPosition, out float3 L) {
			float attenuation = 1.0f;

			L = light.position.xyz - worldPosition;
			attenuation *= RadialAttenuation(L, light.radius);

			if(light.type == LIGHT_SPOT) {
				attenuation *= DirectionalSpotAttenuation(L, light.direction.xyz, cos(light.umbra), cos(light.penumbra));
			} else if(light.type == LIGHT_DIRECTIONAL) {
				attenuation = DirectionalAttenuation(L, light.direction.xyz);
            }

			float distSq = dot(L, L);
			L *= rsqrt(distSq);

			return attenuation;
		}

        // GGX / Trowbridge-Reitz
		// Note the division by PI here
		// [Walter et al. 2007, "Microfacet models for refraction through rough surfaces"]
		float D_GGX(float a2, float NdotH) {
			float d = (NdotH * a2 - NdotH) * NdotH + 1;
			return a2 / (PI * d * d);
		}

        // Appoximation of joint Smith term for GGX
		// Returned value is G2 / (4 * NdotL * NdotV). So predivided by specular BRDF denominator
		// [Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"]
		float Vis_SmithJointApprox(float a2, float NdotV, float NdotL) {
			float Vis_SmithV = NdotL * (NdotV * (1 - a2) + a2);
			float Vis_SmithL = NdotV * (NdotL * (1 - a2) + a2);
			return 0.5 * rcp(Vis_SmithV + Vis_SmithL);
		}

        // [Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"]
		float3 F_Schlick(float3 f0, float VdotH) {
			float Fc = Pow5(1.0f - VdotH);
			return Fc + (1.0f - Fc) * f0;
		}

		float3 F_Schlick(float3 f0, float3 f90, float VdotH) {
			float Fc = Pow5(1.0f - VdotH);
			return f90 * Fc + (1.0f - Fc) * f0;
		}

        uint GetSliceFromDepth(float depth) {
			return floor(log(depth) * g_LightGridParams.x - g_LightGridParams.y);
		}

        // 0.08 is a max F0 we define for dielectrics which matches with Crystalware and gems (0.05 - 0.08)
        // This means we cannot represent Diamond-like surfaces as they have an F0 of 0.1 - 0.2
        float DielectricSpecularToF0(float specular) {
            return 0.08f * specular;
        }

        // Narkowicz2014 - Analytical DFG Term for IBL
        // Source: https://knarkowicz.wordpress.com/2014/12/27/analytical-dfg-term-for-ibl/
        float3 EnvDFGPolynomial(float3 specularColor, float gloss, float ndotv)
        {
            float x = gloss;
            float y = ndotv;

            float b1 = -0.1688;
            float b2 = 1.895;
            float b3 = 0.9903;
            float b4 = -4.853;
            float b5 = 8.404;
            float b6 = -5.069;
            float bias = saturate( min( b1 * x + b2 * x * x, b3 + b4 * y + b5 * y * y + b6 * y * y * y ) );

            float d0 = 0.6045;
            float d1 = 1.699;
            float d2 = -0.5228;
            float d3 = -3.603;
            float d4 = 1.404;
            float d5 = 0.1939;
            float d6 = 2.661;
            float delta = saturate( d0 + d1 * x + d2 * y + d3 * x * x + d4 * x * y + d5 * y * y + d6 * x * x * x );
            float scale = delta - bias;

            bias *= saturate( 50.0 * specularColor.y );
            return specularColor * scale + bias;
        }

        // Note from Filament: vec3 f0 = 0.16 * reflectance * reflectance * (1.0 - metallic) + baseColor * metallic;
        // F0 is the base specular reflectance of a surface
        // For dielectrics, this is monochromatic commonly between 0.02 (water) and 0.08 (gems) and derived from a separate specular value
        // For conductors, this is based on the base color we provided
        float3 ComputeF0(float specular, float3 baseColor, float metalness) {
            return lerp(DielectricSpecularToF0(specular).xxx, baseColor, metalness);
        }

        float3 ComputeDiffuseColor(float3 baseColor, float metalness) {
            return baseColor * (1 - metalness);
        }

        LightResult DefaultLitBxDF(float3 specularColor, float specularRoughness, float3 diffuseColor, half3 N, half3 V, half3 L, float falloff) {
            LightResult lighting = (LightResult)0;

            if(falloff <= 0.0f) {
                return lighting;
            } else {
                float NdotL = saturate(dot(N, L));

                if(NdotL == 0.0f) {
                    return lighting;
                } else {
                    if(specularColor.r != 0.0 && specularColor.g != 0.0 && specularColor.b != 0.0) { // No specular texture?
                        float3 H = normalize(V + L);
                        float NdotV = saturate(abs(dot(N, V)) + 1e-5); // Bias to avoid artifacting
                        float NdotH = saturate(dot(N, H));
                        float VdotH = saturate(dot(V, H));

                        // Generalized microfacet Specular BRDF
                        float a = Square(specularRoughness);
                        float a2 = clamp(Square(a), 0.0001f, 1.0f);
                        float D = D_GGX(a2, NdotH);
                        float Vis = Vis_SmithJointApprox(a2, NdotV, NdotL);
                        float3 F = F_Schlick(specularColor, VdotH);

                        lighting.Specular = (falloff * NdotL) * (D * Vis) * F;

                        // Kulla17 - Energy conervation due to multiple scattering
                        /*float gloss = Pow4(1 - specularRoughness);
                        float3 DFG = EnvDFGPolynomial(specular, gloss, NdotV);
                        float3 energyCompensation = 1.0f + specular * (1.0f / DFG.y - 1.0f);
                        lighting.Specular *= energyCompensation;*/
                    }

                    // Diffuse BRDF
                    lighting.Diffuse = (falloff * NdotL) * Diffuse_Lambert(diffuseColor);
                    return lighting;
                }
            }
        }

        LightResult ApplyLight(float3 specular, float R, float3 diffuse, float3 N, float3 V, float3 worldPos, float4 pos, float dither) {
            LightResult lighting = (LightResult)0;

            if(g_LightSettings.x == 1.0) {
                lighting.Diffuse = diffuse;  // FULL BRIGHT
                return lighting;
            } else {
                uint3 clusterIndex3D = uint3(floor(pos.xy / float2(CLUSTER_TEXTEL_SIZE, CLUSTER_TEXTEL_SIZE)), GetSliceFromDepth(pos.w));
                uint tileIndex = Flatten3D(clusterIndex3D, float2(CLUSTERS_X, CLUSTERS_Y));
                uint lightGridOffset = tileIndex * CLUSTERED_LIGHTING_NUM_BUCKETS;

                for(uint bucketIndex = 0; bucketIndex < CLUSTERED_LIGHTING_NUM_BUCKETS; ++bucketIndex) {
                    uint bucket = g_ClusterDataGrid[lightGridOffset + bucketIndex];

                    while(bucket) {
                        uint bitIndex = firstbitlow(bucket);
                        bucket ^= 1u << bitIndex;

                        uint lightIndex = bitIndex + bucketIndex * CLUSTERS_Z;
                        Light light = g_Lights[lightIndex];

                        // Apply light ------------
                        float3 L;
                        float attenuation = GetAttenuation(light, worldPos, L);

                        if(attenuation > 0.0F) {
                            LightResult result = DefaultLitBxDF(specular, R, diffuse, N, V, L, attenuation);

                            lighting.Diffuse += result.Diffuse * light.color * light.intensity;
                            lighting.Specular += result.Specular * light.color * light.intensity;
                        }
                        // ------------------------
                    }
                }

                // AMBIENT LIGHT ---
                lighting.Diffuse *= g_AmbientColor.rgb;
                // -----------------

                return lighting;
            }
        }

    #endif

#endif
#endif
#endif
