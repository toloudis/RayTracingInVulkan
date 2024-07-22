#version 460
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_ray_tracing : require
#include "Material.glsl"

layout(binding = 4) readonly buffer VertexArray { float Vertices[]; };
layout(binding = 5) readonly buffer IndexArray { uint Indices[]; };
layout(binding = 6) readonly buffer MaterialArray { Material[] Materials; };
layout(binding = 7) readonly buffer OffsetArray { uvec4[] Offsets; };
layout(binding = 8) uniform sampler2D[] TextureSamplers;
layout(binding = 9) uniform sampler3D[] VolumeSamplers;
layout(binding = 10) readonly buffer SphereArray { vec4[] Spheres; };

#include "Scatter.glsl"
#include "Vertex.glsl"

hitAttributeEXT vec4 Sphere;
rayPayloadInEXT RayPayload Ray;

vec2 GetSphereTexCoord(const vec3 point)
{
	const float phi = atan(point.x, point.z);
	const float theta = asin(point.y);
	const float pi = 3.1415926535897932384626433832795;

	return vec2
	(
		(phi + pi) / (2* pi),
		1 - (theta + pi /2) / pi
	);
}

void main()
{
// gl_InstanceCustomIndexEXT is the model index here.
	// Get the material.
	const uvec4 offsets = Offsets[gl_InstanceCustomIndexEXT];
	const uint indexOffset = offsets.x;
	const uint vertexOffset = offsets.y;
	const Vertex v0 = UnpackVertex(vertexOffset + Indices[indexOffset]);
	const Material material = Materials[v0.MaterialIndex];

	// Get the coordinates of the hit point

	const uint sphereOffset = offsets.z;
	const vec4 sphere = Spheres[sphereOffset + gl_PrimitiveID];
	// get center into world space.  assume radius unchanged.
	const vec3 center = (gl_ObjectToWorldEXT * vec4(sphere.xyz, 1.0)).xyz;

	// The above code is slightly faster than reading a hitAttributeEXT value!!
	// as this code is expanded, we may need to use hitAttributes anway later...
	//const vec4 sphere = Sphere; // was set in intersection shader
	//const vec3 center = sphere.xyz;

	const float radius = sphere.w;

	const vec3 point = gl_WorldRayOriginEXT + gl_HitTEXT * gl_WorldRayDirectionEXT;
	const vec3 normal = (point - center) / radius;
	const vec2 texCoord = GetSphereTexCoord(normal);

	Ray = Scatter(material, gl_WorldRayDirectionEXT, normal, texCoord, gl_HitTEXT, Ray.RandomSeed);
}
