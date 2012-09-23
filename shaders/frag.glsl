#version 120

varying vec3 position;
varying vec3 normal;
varying vec3 color;

vec3 hsv(float h,float s,float v) { return mix(vec3(1.),clamp((abs(fract(h+vec3(3.,2.,1.)/3.)*6.-3.)-1.),0.,1.),s)*v; }

vec3 colgrad(float a) { return vec3(65./255., 35./255.*a, 25./255.); }

void main()
{	
	gl_FragColor = vec4(vec3((length(position)-0.9)*10.), 1.0);
}
