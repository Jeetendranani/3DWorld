uniform float alpha = 1.0;
varying vec2 tc;

void main()
{
	tc          = gl_MultiTexCoord0;
	gl_Position = ftransform();
	vec3 normal = normalize(gl_NormalMatrix * gl_Normal); // eye space
	gl_FogFragCoord = length((gl_ModelViewMatrix * gl_Vertex).xyz); // set standard fog coord
	vec4 color = vec4(0,0,0, alpha);
	if (enable_light0) color.rgb += add_light_comp0(normal).rgb;
	if (enable_light1) color.rgb += add_light_comp1(normal).rgb;
	gl_FrontColor = color;
}
