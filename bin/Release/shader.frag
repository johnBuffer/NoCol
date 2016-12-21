uniform sampler2D texture;
uniform float WIDTH;
uniform float HEIGHT;

void main(void)
{
    vec4 color = texture2D(texture, vec2(gl_FragCoord.x/WIDTH, gl_FragCoord.y/HEIGHT));
	
	if (length(color.xyz) < 0.1)
		color = vec4(0, 0, 0, 1);
	else
		color = vec4(color.xyz*0.95, 1);
	
	gl_FragColor = color;
}