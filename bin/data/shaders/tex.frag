/* fragment */

#version 120
#extension GL_ARB_texture_rectangle : enable

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;

uniform sampler2DRect colorMap;

void main(void)
{
	vec2 texCoord = gl_TexCoord[0].st * resolution;
	//vec2 texCoord = gl_FragCoord.xy;
	gl_FragColor = texture2DRect(colorMap, texCoord);
}
