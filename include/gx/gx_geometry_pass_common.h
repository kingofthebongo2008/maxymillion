#ifndef __GX_GEOMETRY_PASS_COMMON_H__
#define __GX_GEOMETRY_PASS_COMMON_H__

#include <cstdint>

#include <math/math_matrix.h>

#include <gx/gx_pinhole_camera.h>

namespace gx
{
	 enum cbuffer_frequency : uint8_t
	 {
         slot_per_frame = 0,
		 slot_per_pass = 1,
	     slot_per_draw_call = 2
	 };

	 struct reprojection_params
	 {
		 float a;
		 float b;
		 float c;
		 float d;

         operator math::float4() const
         {
             return math::set(a, b, c, d);
         }
	 };

	 struct cbuffer_constants_per_pass
	 {
		 math::float4x4 m_v;	//view matrix;
		 math::float4x4 m_p;	//projection matrix;

		 reprojection_params m_reprojection_params;
	 };

     inline reprojection_params create_reprojection_params(float zn, float zf)
     {
         return { 0.0f, 1.0f, 1.0f / zn - 1.0f / zf, 1.0f / zn };
     }

     inline reprojection_params create_perspective_reprojection_params(const pinhole_camera* camera)
     {
         return create_reprojection_params( camera->get_near(), camera->get_far() );
     }

     inline reprojection_params create_perspective_reprojection_params(const pinhole_camera& camera)
     {
         return create_reprojection_params(camera.get_near(), camera.get_far());
     }
}

#endif