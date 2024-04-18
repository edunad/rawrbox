#pragma once

#ifdef RAWRBOX_RENDER
	#include <rawrbox/physics/utils.hpp>
	#include <rawrbox/render/static.hpp>
	#include <rawrbox/render/stencil.hpp>

	#include <Jolt/Renderer/DebugRenderer.h>

namespace rawrbox {
	struct RBatch : public JPH::RefTargetVirtual, public JPH::RefTarget<RBatch> {
		std::vector<JPH::DebugRenderer::Vertex> vertices;
		std::vector<uint32_t> indices;

		RBatch(const JPH::DebugRenderer::Vertex* vertex, int vertexCount, const JPH::uint32* index, int indicesCount) : vertices(vertex, vertex + vertexCount) {
			if (index != nullptr) {
				indices.assign(index, index + indicesCount);
			} else {
				indices.reserve(vertexCount);
				for (uint32_t i = 0; i < static_cast<uint32_t>(vertexCount); ++i) {
					indices.push_back(i);
				}
			}
		}

		void AddRef() override { JPH::RefTarget<RBatch>::AddRef(); }
		void Release() override { JPH::RefTarget<RBatch>::Release(); }
	};

	class DebugRenderer : public JPH::DebugRenderer {
	protected:
		float _maxRenderDistance = 100.F;

	public:
		DebugRenderer(float maxDistance = 100.F) : _maxRenderDistance(maxDistance) { JPH::DebugRenderer::Initialize(); }

		Batch CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount) override {
			// NOLINTBEGIN(*)
			return new RBatch(inTriangles->mV, inTriangleCount * 3, nullptr, inTriangleCount * 3); // Screams in memory leak
													       // NOLINTEND(*)
		}

		Batch CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const JPH::uint32* inIndices, int inIndexCount) override {
			// NOLINTBEGIN(*)
			return new RBatch(inVertices, inVertexCount, inIndices, inIndexCount); // Screams in memory leak
											       // NOLINTEND(*)
		}

		void DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode /*inCullMode*/ = ECullMode::CullBackFace, ECastShadow /*inCastShadow*/ = ECastShadow::On, EDrawMode inDrawMode = EDrawMode::Solid) override {
			auto* renderer = rawrbox::RENDERER;
			auto* camera = rawrbox::MAIN_CAMERA;
			if (renderer == nullptr || camera == nullptr) return;

			auto camPos = rawrbox::PhysUtils::vecToPos(camera->getPos());
			const auto& screenSize = renderer->getSize();
			auto* stencil = renderer->stencil();
			if (stencil == nullptr) return;

			if (inWorldSpaceBounds.GetSqDistanceTo(camPos) > this->_maxRenderDistance) {
				return;
			}

			const RBatch& b = *dynamic_cast<const RBatch*>(inGeometry->GetLOD(camPos, inWorldSpaceBounds, inLODScaleSq).mTriangleBatch.GetPtr());

			auto mdlColor = inModelColor.ToVec4();
			rawrbox::Color color = {mdlColor.GetX(), mdlColor.GetY(), mdlColor.GetZ(), mdlColor.GetW()};

			// Transform 3D vertices to 2D screen coordinates
			stencil->pushClipping({0, 0, static_cast<int>(screenSize.x), static_cast<int>(screenSize.y)});
			for (size_t fi = 0; fi < b.indices.size(); fi += 3) {
				JPH::Float3 v0 = {};
				JPH::Float3 v1 = {};
				JPH::Float3 v2 = {};

				(inModelMatrix * JPH::Vec3(b.vertices[b.indices[fi + 0]].mPosition)).StoreFloat3(&v0);
				(inModelMatrix * JPH::Vec3(b.vertices[b.indices[fi + 1]].mPosition)).StoreFloat3(&v1);
				(inModelMatrix * JPH::Vec3(b.vertices[b.indices[fi + 2]].mPosition)).StoreFloat3(&v2);

				rawrbox::Vector3f screenPos0 = camera->worldToScreen({v0.x, v0.y, v0.z}) /*.clamp({0, 0, 0}, {static_cast<float>(screenSize.x), static_cast<float>(screenSize.y), 1.F})*/;
				rawrbox::Vector3f screenPos1 = camera->worldToScreen({v1.x, v1.y, v1.z}) /*.clamp({0, 0, 0}, {static_cast<float>(screenSize.x), static_cast<float>(screenSize.y), 1.F})*/;
				rawrbox::Vector3f screenPos2 = camera->worldToScreen({v2.x, v2.y, v2.z}) /*.clamp({0, 0, 0}, {static_cast<float>(screenSize.x), static_cast<float>(screenSize.y), 1.F})*/;

				// Check if all vertices are outside the screen
				if (screenPos0.z < 0 || screenPos0.z > 1.F || screenPos1.z < 0 || screenPos1.z > 1.F || screenPos2.z < 0 || screenPos2.z > 1.F) continue;

				if (inDrawMode == EDrawMode::Wireframe) {
					stencil->drawLine(screenPos0.xy(), screenPos1.xy());
					stencil->drawLine(screenPos1.xy(), screenPos2.xy());
					stencil->drawLine(screenPos2.xy(), screenPos0.xy());
				} else {
					stencil->drawTriangle(screenPos0.xy(), {}, color,
					    screenPos1.xy(), {}, color,
					    screenPos2.xy(), {}, color);
				}
			}
			stencil->popClipping();
		}

		void DrawText3D(JPH::RVec3Arg /*inPosition*/, const JPH::string_view& /*inString*/, JPH::ColorArg /*inColor = JPH::Color::sWhite*/, float /*inHeight = 0.5F*/) override {
		}

		void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override {
			auto* stencil = rawrbox::RENDERER->stencil();

			rawrbox::Vector2f from(inFrom.GetX(), inFrom.GetY());
			rawrbox::Vector2f to(inTo.GetX(), inTo.GetY());
			rawrbox::Color color(inColor.r, inColor.g, inColor.b, inColor.a);
			stencil->drawLine(from, to, color);
		}

		void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow /*inCastShadow*/ = ECastShadow::Off) override {
			auto* stencil = rawrbox::RENDERER->stencil();

			rawrbox::Vector2f v1(inV1.GetX(), inV1.GetY());
			rawrbox::Vector2f v2(inV2.GetX(), inV2.GetY());
			rawrbox::Vector2f v3(inV3.GetX(), inV3.GetY());
			rawrbox::Color color(inColor.r, inColor.g, inColor.b, inColor.a);

			stencil->drawTriangle(v1, {}, color, v2, {}, color, v3, {}, color);
		}
	};
} // namespace rawrbox
#endif
