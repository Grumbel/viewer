#version 140

// uniform mat4  model_matrix;
// uniform mat4  view_matrix;
// uniform mat4  projection_matrix;

attribute vec4  bone_weights;
attribute ivec4 bone_indices;

uniform mat4 bones[30];
uniform mat4 pose_bones[30];

// shadowmap
uniform mat4 ShadowMapMatrix;
varying vec4 ProjShadow;
varying vec4 position;

// phong
varying vec3 normal;
varying vec3 eyeVec;
varying vec3 lightDir[2];

// normal mapping
varying vec3 lightDir_;
varying vec3 viewDir;

// grid
varying vec4 vertex_position;

// cubemap
varying vec3 world_normal;
varying vec4 world_position;
uniform mat4 eye_matrix;

void main(void)
{
  { // cube map
    world_position = eye_matrix * gl_ModelViewMatrix * gl_Vertex;
    world_normal   = mat3(eye_matrix) * gl_NormalMatrix * gl_Normal;
  }

  { // grid
    vertex_position = gl_Vertex;    
  }

  { // shadowmap
    ProjShadow  = ShadowMapMatrix * gl_Vertex;
  }

  { // phong
    normal = gl_NormalMatrix * gl_Normal;

    vec3 vVertex = vec3(gl_ModelViewMatrix * gl_Vertex);

    for(int i = 0; i < 2; ++i)
    {
      lightDir[i] = vec3(gl_LightSource[i].position.xyz - vVertex);
    }

    eyeVec = -vVertex;
  }

  { // normal mapping 
    vec3 vVertex = vec3(gl_ModelViewMatrix * gl_Vertex);

    vec3 n = normalize(gl_NormalMatrix * gl_Normal);
    vec3 t = normalize(gl_NormalMatrix * gl_MultiTexCoord1.xyz);
    vec3 b = cross(n, t) * gl_MultiTexCoord1.w;
    
    mat3 tbnMatrix = mat3(t.x, b.x, n.x,
                          t.y, b.y, n.y,
                          t.z, b.z, n.z);

    lightDir_ = (gl_LightSource[0].position.xyz - vVertex);
    lightDir_ = tbnMatrix * lightDir_;

    viewDir = -vVertex;
    viewDir = tbnMatrix * viewDir;
  }

  { // regular stuff
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

    ivec4 bi = bone_indices;
    vec4 bw = bone_weights;

    vec4 p;
    for(int i = 0; i < 4; ++i)
    {
      p += pose_bones[bi.x] * bones[bi.x] * gl_Vertex * bw.x;
      
      bw = bw.yzwx;
      bi = bi.yzwx;
    }

    gl_Position = position = gl_ModelViewProjectionMatrix * p;
  }
}

/* EOF */
