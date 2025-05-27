#version 460 core

layout(rgba32f, binding = 0) uniform writeonly image2D outputImage;

layout(binding = 1) uniform sampler2D u_SkyTexture;
layout(binding = 2) uniform sampler2D u_GroundTexture;

layout(location = 0) uniform vec4 u_Near;
layout(location = 1) uniform vec4 u_Far;

layout(local_size_x = 16, local_size_y = 16) in;
void main()
{
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);

    if (pixelCoord.x >= imageSize(outputImage).x || pixelCoord.y >= imageSize(outputImage).y)
		return;

    ivec2 texSize = imageSize(outputImage);
    vec2 fTexSize = vec2(texSize);
    vec2 normalizedCoord = vec2(pixelCoord) / vec2(texSize);

    float fSampleDepth = abs(normalizedCoord.y * 2.0 - 1.0);
    // knowing how far into the frustum we want to be, we calculate two points
    // that represent scan line begin and end point
    // think of it as the horizontal line going across the frustum at a given depth
    float fStartX;
    float fStartY;
    float fEndX;
    float fEndY;    
    if (pixelCoord.y > 0)
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

    float fSampleWidth = float(pixelCoord.x) / fTexSize.x;
    float fSampleX = (fEndX - fStartX) * fSampleWidth + fStartX;
    float fSampleY = (fEndY - fStartY) * fSampleWidth + fStartY;

    fSampleX -= int(fSampleX);
    fSampleY -= int(fSampleY);

    if (normalizedCoord.y > 0.5)
    {
        vec4 skySample = texture(u_SkyTexture, vec2(fSampleX, fSampleY));
        skySample.rgb = mix(vec3(1.0), skySample.rgb, fSampleDepth * fSampleDepth);

        imageStore(outputImage, pixelCoord, skySample);
    }
    else
    {
        vec4 groundSample = texture(u_GroundTexture, vec2(fSampleX, fSampleY));
        groundSample.rgb = mix(vec3(1.0), groundSample.rgb, fSampleDepth * fSampleDepth);

        imageStore(outputImage, pixelCoord, groundSample);
    }
    
}