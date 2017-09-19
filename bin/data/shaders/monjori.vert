varying vec2 the_uv;

void main()
{          
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	//the_uv = gl_MultiTexCoord0.st;
}

