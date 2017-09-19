// Found this on GLSL sandbox. I really liked it, changed a few things and made it tileable.
// :)

// -----------------------------------------------------------------------
// Water turbulence effect by joltz0r 2013-07-04, improved 2013-07-07
// Altered
// -----------------------------------------------------------------------

#define TAU 6.28318530718
#define MAX_ITER 5

uniform float time;
uniform float alp;
uniform vec2 mouse;
uniform vec2 resolution;

void main( void ) 
{
	float ltime = time * .5;
	vec2 sp = gl_FragCoord.xy / (resolution.xy * 0.5);
	
	vec2 p = sp*TAU - 20.0;
	vec2 i = p;
	float c = 1.0;
	float inten = .05;

	for (int n = 0; n < MAX_ITER; n++) 
	{
		float t = ltime * (1.0 - (3.5 / float(n+1)));
		i = p + vec2(cos(t - i.x) + sin(t + i.y), sin(t - i.y) + cos(t + i.x));
		c += 1.0/length(vec2(p.x / (sin(i.x+t)/inten),p.y / (cos(i.y+t)/inten)));
	}
	c /= float(MAX_ITER);
	c = 1.55-sqrt(c);
	vec3 colour = vec3(pow(abs(c), 6.0));
	gl_FragColor = vec4(clamp(colour + vec3(0.0, 0.35, 0.5), 0.0, 1.0), alp);
}

