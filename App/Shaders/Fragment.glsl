#version 460 core

layout (location = 0) out vec4 o_Color;

in vec2 v_TexCoord;

layout(binding = 1) uniform sampler2D u_SkyTexture;
layout(binding = 2) uniform sampler2D u_GroundTexture;

layout(location = 0) uniform vec4 u_Near;
layout(location = 1) uniform vec4 u_Far;

void main()
{
	ivec2 texSize = ivec2(1280, 720);
    vec2 fTexSize = vec2(texSize);
    vec2 normalizedCoord = v_TexCoord;

    float fSampleDepth = abs(normalizedCoord.y * 2.0 - 1.0);
    // knowing how far into the frustum we want to be, we calculate two points
    // that represent scan line begin and end point
    // think of it as the horizontal line going across the frustum at a given depth
    float fStartX;
    float fStartY;
    float fEndX;
    float fEndY;    
    if (normalizedCoord.y > 0)
    {
        fStartX = (u_Far.x - u_Near.x) / (fSampleDepth) + u_Near.x;
        fStartY = (u_Far.y - u_Near.y) / (fSampleDepth) + u_Near.y;
        fEndX = (u_Far.z - u_Near.z) / (fSampleDepth) + u_Near.z;
        fEndY = (u_Far.w - u_Near.w) / (fSampleDepth) + u_Near.w; 
    }
    else
    {
		fStartX = u_Far.x;
		fStartY = u_Far.y;
		fEndX = u_Far.z;
		fEndY = u_Far.w;
    }

    float fSampleWidth = normalizedCoord.x;
    float fSampleX = (fEndX - fStartX) * fSampleWidth + fStartX;
    float fSampleY = (fEndY - fStartY) * fSampleWidth + fStartY;

    fSampleX -= int(fSampleX);
    fSampleY -= int(fSampleY);

    vec4 skySample = texture(u_SkyTexture, vec2(fSampleX, fSampleY));
    vec4 groundSample = texture(u_GroundTexture, vec2(fSampleX, fSampleY));

    skySample.rgb = mix(vec3(1.0), skySample.rgb, fSampleDepth * fSampleDepth);
    groundSample.rgb = mix(vec3(1.0), groundSample.rgb, fSampleDepth * fSampleDepth);

    skySample.r += 0.2;

    o_Color = normalizedCoord.y > 0.5 ? skySample : groundSample;
}