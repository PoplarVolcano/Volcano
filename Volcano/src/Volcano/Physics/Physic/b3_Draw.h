#pragma once

#include "b3_Math.h"

namespace Volcano {

	// Color for debug drawing. Each value has the range [0,1].
	struct b3_Color
	{
		b3_Color() {}
		b3_Color(float rIn, float gIn, float bIn, float aIn = 1.0f) { r = rIn; g = gIn; b = bIn; a = aIn; }
		void Set(float rIn, float gIn, float bIn, float aIn = 1.0f) { r = rIn; g = gIn; b = bIn; a = aIn; }
		float r, g, b, a;
	};

	// 在b3_World中实现并注册此类，以提供游戏中物理实体的调试绘图(debug drawing)。
	class b3_Draw
	{
	public:
		b3_Draw();
		virtual ~b3_Draw() {}

		enum
		{
			e_shapeBit = 0x0001,	    // draw shapes
			e_jointBit = 0x0002,	    // draw joint connections
			e_aabbBit  = 0x0004,	    // draw axis aligned bounding boxes
			e_pairBit  = 0x0008,	    // draw broad-phase pairs
			e_centerOfMassBit = 0x0010	// draw center of mass frame
		};

		void SetFlags(uint32_t flags);    // Set the drawing flags.
		uint32_t GetFlags() const;        // Get the drawing flags.
		void AppendFlags(uint32_t flags); // Append flags to the current flags.
		void ClearFlags(uint32_t flags);  // Clear flags from the current flags.
		virtual void DrawPolygon(const glm::vec3* vertices, int vertexCount, const b3_Color& color) = 0;                       // Draw a closed polygon provided in CCW order.
		virtual void DrawSolidPolygon(const glm::vec3* vertices, int vertexCount, const b3_Color& color) = 0;                  // Draw a solid closed polygon provided in CCW order.
		virtual void DrawCircle(const glm::vec3& center, float radius, const b3_Color& color) = 0;		                       // Draw a circle.
		virtual void DrawSolidCircle(const glm::vec3& center, float radius, const glm::vec3& axis, const b3_Color& color) = 0; // Draw a solid circle.
		virtual void DrawSegment(const glm::vec3& p1, const glm::vec3& p2, const b3_Color& color) = 0;                         // Draw a line segment.
		virtual void DrawTransform(const b3_Transform& transform) = 0;                                                         // Draw a transform. Choose your own length scale.
		virtual void DrawPoint(const glm::vec3& p, float size, const b3_Color& color) = 0;                                     // Draw a point.

	protected:
		uint32_t m_drawFlags;
	};

}