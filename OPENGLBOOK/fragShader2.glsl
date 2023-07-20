#version 430

in vec3 varyingNormal, varyingLightDir, varyingVertPos, varyingHalfVec;
in vec4 shadow_coord;
out vec4 fragColor;

struct PositionalLight
{
	vec4 ambient, diffuse, specular;
	vec3 position;
};

struct Material
{
	vec4 ambient, diffuse, specular;
	float shininess;
};

uniform vec4 globalAmbient;
uniform PositionalLight light;
uniform Material material;
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix;
uniform mat4 shadowMVP;

layout (binding=0) uniform sampler2DShadow shTex;

float lookup(float ox, float oy)
{
	float t = textureProj(shTex, shadow_coord + vec4(ox * 0.001 * shadow_coord.w, oy * 0.001 * shadow_coord.w, -0.01, 0.0));
	return t;
}

void main(void)
{
	float shadowFactor = 0.0;
	vec3 L = normalize(varyingLightDir);
	vec3 N = normalize(varyingNormal);
	vec3 V = normalize(-varyingVertPos);
	vec3 H = normalize(varyingHalfVec);

	float swidth = 2.5;
	vec2 offset = mod(floor(gl_FragCoord.xy), 2.0) * swidth;
	shadowFactor += lookup(-1.5 * swidth + offset.x,  1.5 * swidth - offset.y);
	shadowFactor += lookup(-1.5 * swidth + offset.x, -0.5 * swidth - offset.y);
	shadowFactor += lookup( 0.5 * swidth + offset.x,  1.5 * swidth - offset.y);
	shadowFactor += lookup( 0.5 * swidth + offset.x, -0.5 * swidth - offset.y);
	shadowFactor = shadowFactor / 4.0;

	vec4 shadowColor = globalAmbient * material.ambient + light.ambient * material.ambient;

	vec4 lightedColor = light.diffuse * material.diffuse * max(dot(L, N), 0.0) + light.specular * material.specular
		* pow(max(dot(H, N), 0.0), material.shininess * 3.0);

	fragColor = vec4((shadowColor.xyz + shadowFactor * (lightedColor.xyz)), 1.0);
}