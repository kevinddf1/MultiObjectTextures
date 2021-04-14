#version 150 core

in vec3 Color;
in vec3 vertNormal;
in vec3 pos;
in vec3 lightDir;
in vec2 texcoord;

out vec4 outColor;

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;


uniform int texID;
uniform int ColorID;

const float ambient = .3;
void main() {
  vec3 color;

  //mornal texture
  if (texID == -1)
    color = Color;
  else if (texID == 0)
    color = texture(tex0, texcoord).rgb;
  else if (texID == 1)
    color = texture(tex1, texcoord).rgb;  
  else if (texID == 2)
    color = texture(tex2, texcoord).rgb;
  else if (texID == 3)
    color = texture(tex3, texcoord).rgb;

  // for key
  else if (texID == 11)
    color = vec3(1,0,0);
  else if (texID == 12)
    color = vec3(0,1,0);
  else if (texID == 13)
    color = vec3(0,0,1);
  else if (texID == 14)
    color = vec3(1,1,0);
  else if (texID == 15)
    color = vec3(0,1,1);

  //for door
  else if (texID == 21)
    color = 0.5*vec3(1,0,0)+ 0.5*texture(tex0, texcoord).rgb;
  else if (texID == 22)
    color =  0.5*vec3(0,1,0)+0.5*texture(tex0, texcoord).rgb;
  else if (texID == 23)
    color =  0.5*vec3(0,0,1)+0.5*texture(tex0, texcoord).rgb;
  else if (texID == 24)
    color =  0.5*vec3(1,1,0)+0.5*texture(tex0, texcoord).rgb;
  else if (texID == 25)
    color =  0.5*vec3(0,1,1)+0.5*texture(tex0, texcoord).rgb;

  else{
    outColor = vec4(1,0,0,1);
    return; //This was an error, stop lighting!
  }
  vec3 normal = normalize(vertNormal);
  vec3 diffuseC = color*max(dot(-lightDir,normal),0.0);
  vec3 ambC = color*ambient;
  vec3 viewDir = normalize(-pos); //We know the eye is at (0,0)! (Do you know why?)
  vec3 reflectDir = reflect(viewDir,normal);
  float spec = max(dot(reflectDir,lightDir),0.0);
  if (dot(-lightDir,normal) <= 0.0) spec = 0; //No highlight if we are not facing the light
  vec3 specC = .8*vec3(1.0,1.0,1.0)*pow(spec,4);
  vec3 oColor = ambC+diffuseC+specC;
  outColor = vec4(oColor,1);
}