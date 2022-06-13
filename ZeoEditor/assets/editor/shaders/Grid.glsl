// Grid Shader

#type vertex
#version 450 core

layout (std140, binding = 1) uniform Camera
{
    mat4 View;
    mat4 Projection;
    vec3 Position;
}u_Camera;

layout (std140, binding = 2) uniform Grid
{
	mat4 Transform;
	vec4 ThinLinesColor;
	vec4 ThickLinesColor;
	vec4 OriginAxisXColor;
	vec4 OriginAxisZColor;
	float Extent;
	float CellSize;
}u_Grid;

struct VertexOutput
{
    mat4 Transform;
    vec4 ThinLinesColor;
	vec4 ThickLinesColor;
	vec4 OriginAxisXColor;
	vec4 OriginAxisZColor;
    float Extent;
    float CellSize;

	vec2 TilePos;
    vec2 GridPos;
    vec2 CameraGridPos;
    vec3 ViewDir;
};

layout (location = 0) out VertexOutput Output;

const vec2 vertices[6] = {
    vec2(1.0f, -1.0f), vec2(-1.0f, -1.0f), vec2(-1.0, 1.0f),
    vec2(1.0, -1.0f), vec2(-1.0, 1.0f), vec2(1.0f, 1.0f)
};

void main()
{
    Output.Transform = u_Grid.Transform;
    Output.ThinLinesColor = u_Grid.ThinLinesColor;
    Output.ThickLinesColor = u_Grid.ThickLinesColor;
    Output.OriginAxisXColor = u_Grid.OriginAxisXColor;
    Output.OriginAxisZColor = u_Grid.OriginAxisZColor;
    Output.Extent = u_Grid.Extent;
    Output.CellSize = u_Grid.CellSize;

    int vertexId = gl_InstanceIndex * 6 + gl_VertexIndex;

    const float gridTileSize = 100.0f;
    uint numTilesPerAxis = uint(ceil((u_Grid.Extent * 2.0f) / gridTileSize));
    uint tileIndex = uint(floor(vertexId / 6));
    vec2 tilePos = (vec2(tileIndex % numTilesPerAxis, tileIndex / numTilesPerAxis) - (numTilesPerAxis / 2)) * gridTileSize;
    Output.TilePos = tilePos;

    vec2 gridUv = vec2(vertices[vertexId % 6].x * gridTileSize * 0.5f, vertices[vertexId % 6].y * gridTileSize * 0.5f);
    Output.GridPos = gridUv;

    vec3 cameraGridPos = (inverse(u_Grid.Transform) * vec4(u_Camera.Position, 1.0f)).xyz;
    vec4 pos = vec4(tilePos.x + gridUv.x, 0.0f, tilePos.y + gridUv.y, 1.0f);
    pos.y -= cameraGridPos.y;
    pos = u_Grid.Transform * pos;
    Output.CameraGridPos = cameraGridPos.xz + tilePos.xy;        
        
    Output.ViewDir = -pos.xyz;

    vec3 vp = (mat3(u_Camera.View) * pos.xyz).xyz;
    vec4 cp = u_Camera.Projection * vec4(vp, 1.0f);
    gl_Position = cp;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out vec4 o_EntityID;

struct VertexOutput
{
    mat4 Transform;
    vec4 ThinLinesColor;
	vec4 ThickLinesColor;
	vec4 OriginAxisXColor;
	vec4 OriginAxisZColor;
    float Extent;
    float CellSize;

	vec2 TilePos;
    vec2 GridPos;
    vec2 CameraGridPos;
    vec3 ViewDir;
};

layout (location = 0) in VertexOutput Input;

float log10(float x)
{
    return log(x) / log(10.0f);
}

float Saturate(float x)
{
    return clamp(x, 0.0f, 1.0f);
}

vec2 Saturate(vec2 x)
{
    return clamp(x, vec2(0.0f), vec2(1.0f));
}

void main()
{
    // UV is grid space coordinate of pixel
    vec2 uv = Input.GridPos;
    // Find screen space derivates in grid space. 
    vec2 dudv = vec2(length(vec2(dFdx(uv.x), dFdy(uv.x))), length(vec2(dFdx(uv.y), dFdy(uv.y))));        

    // Define minimum number of pixels between cell lines before LOD switch should occur
    float minPixelsBetweenCells = 1.0f;

    // Calc lod-level
    float lodLevel = max(0, log10((length(dudv) * minPixelsBetweenCells) / Input.CellSize) + 1);
    float lodFade = fract(lodLevel);

    // Calc cell sizes for lod0, lod1 and lod2
    float cellSizeLod0 = Input.CellSize * pow(10, floor(lodLevel));
    float cellSizeLod1 = cellSizeLod0 * 10.f;
    float cellSizeLod2 = cellSizeLod1 * 10.f;

    // If grid is setup to be infinte, shift uv based on camera position in local tile space
    uv += Input.CameraGridPos;

    // Allow each anti-aliased line to cover up to 4 pixels
    int lineThickness = 3;
    dudv *= lineThickness;
    // Offset to pixel center.
    uv = abs(uv) + dudv / 2;

    // Calculate distance to cell line center for each lod and pick max of X,Y to get a coverage alpha value
    // Note: another alternative is to average the x,y values and use that as the covergae alpha (dot(crossAlphaLod0, 0.5))
    vec2 crossAlphaLod0 = 1.f - abs(Saturate(mod(uv, cellSizeLod0) / dudv) * 2 - 1.f);
    float alphaLod0 = max(crossAlphaLod0.x, crossAlphaLod0.y);
    vec2 crossAlphaLod1 = 1.f - abs(Saturate(mod(uv, cellSizeLod1) / dudv) * 2 - 1.f);
    float alphaLod1 = max(crossAlphaLod1.x, crossAlphaLod1.y);
    vec2 crossAlphaLod2 = 1.f - abs(Saturate(mod(uv, cellSizeLod2) / dudv) * 2 - 1.f);
    float alphaLod2 = max(crossAlphaLod2.x, crossAlphaLod2.y);

    // Set XZ axis colors for axis-matching thick lines
    const vec2 displacedGridPos = Input.GridPos + Input.CameraGridPos;
    const bool bIsAxisZ = crossAlphaLod2.x > 0 && (-cellSizeLod1 < displacedGridPos.x && displacedGridPos.x < cellSizeLod1);
    const bool bIsAxisX = crossAlphaLod2.y > 0 && (-cellSizeLod1 < displacedGridPos.y && displacedGridPos.y < cellSizeLod1);
    vec4 thickColor = bIsAxisX ? Input.OriginAxisXColor : Input.ThickLinesColor;
    thickColor = bIsAxisZ ? Input.OriginAxisZColor : thickColor;

    // Blend between falloff colors
    vec4 c = alphaLod2 > 0 ? thickColor : alphaLod1 > 0 ? mix(thickColor, Input.ThinLinesColor, lodFade) : Input.ThinLinesColor;

    // Calculate opacity falloff based on distance to grid extents and gracing angle
    vec3 viewDir = normalize(Input.ViewDir);
    float opGracing = 1.f - pow(1.f - abs(dot(viewDir, Input.Transform[1].xyz)), 10);        
    vec2 v = Input.TilePos + Input.GridPos;
    float opDistance = (1.f - Saturate(length(v) / Input.Extent));
    float op = opGracing * opDistance;

    // Blend between LOD level alphas and scale with opacity falloff
    c.a *= (alphaLod2 > 0 ? alphaLod2 : alphaLod1 > 0 ? alphaLod1 : (alphaLod0 * (1-lodFade))) * op;
    if (c.a == 0.0f) discard;

    o_Color = c;

    // Setting alpha to 0 to ensure that current ID buffer value is not overridden by the grid
    o_EntityID = vec4(-1.0f, 0.0f, 0.0f, 0.0f);
}
